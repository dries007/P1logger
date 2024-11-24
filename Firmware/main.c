#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/twi.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "packet.h"
#include "crc.h"

/**
 * Pin assignments
 */
// OUTPUTS
#define LED0        _BV(PORTC0)  // RED
#define LED1        _BV(PORTC1)  // RED
#define LED2        _BV(PORTC2)  // YELLOW
#define LED3        _BV(PORTC3)  // YELLOW
#define LED4        _BV(PORTC4)  // GREEN
#define LED5        _BV(PORTC5)  // GREEN
#define DATA_REQ    _BV(PORTC6)
#define RF_SET      _BV(PORTC7)

// INPUTS
#define SETTING7    _BV(PORTA7)
#define SETTING6    _BV(PORTA6)
#define SETTING5    _BV(PORTA5)
#define SETTING4    _BV(PORTA4)
#define SETTING3    _BV(PORTA3)
#define SETTING2    _BV(PORTA2)
#define SETTING1    _BV(PORTA1)
#define SETTING0    _BV(PORTA0)

/**
 * Error codes
 */
#define ERROR_BASE      0x80000000
#define ERROR_FATAL     0x40000000
#define ERROR_UNKNOWN   (ERROR_BASE | ERROR_FATAL)
#define ERROR_BOOT      (ERROR_BASE | 0x0001)
#define ERROR_CRC       (ERROR_BASE | 0x0002)


/**
 * Simple ring buffer struct
 * 256 long so uint8_t rollover can be used to index the buffer.
 */
struct ringBuffer {
    volatile char buffer[256];
    volatile uint8_t writeIndex;
    volatile uint8_t readIndex;
};

// UART 1 RX ring buffer.
volatile struct ringBuffer rb1 = {0};

// All variables needed to async tx data.
volatile uint8_t tx_buffer[sizeof(Packet)];
volatile bool tx_sending = 0;
volatile uint8_t tx_index = 0;

// readLine uses this global buffer.
#define readLineBufferSize 256
char readLineBuffer[readLineBufferSize];


/**
 * ISR for UART1 (P1 RX) Received Bytes.
 * Fills up ring buffer.
 */
ISR(USART1_RX_vect) {
    rb1.buffer[rb1.writeIndex++] = UDR1;
}

/**
 * ISR for UART0 (RF TX) Data Register Empty.
 * Used for async tx of the entire tx_buffer.
 */
ISR(USART0_UDRE_vect) {
    UCSR0B &= ~(1<<UDRIE0);
    if (tx_index < sizeof(Packet)) {
        UDR0 = tx_buffer[tx_index++];
        UCSR0B |= (1<<UDRIE0);
    } else {
        tx_sending = false;
    }
}

/**
 * Send an error code packet.
 * Max payload len is 50 bytes.
 */
static inline void error(uint32_t code, void* payload, uint8_t len);

/**
 * Send packet.
 * This function calculates & sets the CRC,
 * it copies the packet to a tx buffer, and
 * starts up the (interrupt driven) tx routine.
 *
 * This function blocks if there is already a tx going.
 * Once this function returns, the packet can be reset.
 */
static inline void sendPacket();

/**
 * Read a line from the UART fed read buffer into the readLineBuffer.
 * Includes the newline, for CRC calculations.
 * Max len = readLineBufferSize
 * @return the len of the line
 */
static inline uint8_t readLine();

/**
 * Init UART 1 (P1 RX)
 * 115200 baud, RX only, interrupt driven.
 */
static inline void meterUARTInit(void) {
    // 115200 baud, F_CPU 11.0592 MHz
    // https://trolsoft.ru/en/uart-calc

    // Set baud rate
    UBRR1L = 0x05;
    UBRR1H = 0x00;

    // Enable RX and RX interrupts
    UCSR1B= (1<<RXEN1) | (1<<RXCIE1);
    // Set the "normal" 8N1 UART frame mode.
    UCSR1C= (1<<UCSZ11) | (1<<UCSZ10);
    UCSR1A= 0x00;

    UCSR1A &= ~(1 << U2X1);
}

/**
 * Init UART 0 (RF TX)
 * 1200 baud, TX only, interrupt driven.
 */
static inline void RF_UART_Init(void) {
    // 1200 baud, F_CPU 11.0592 MHz
    // https://trolsoft.ru/en/uart-calc

    // Set baud rate
    UBRR0L = 0x3F;
    UBRR0H = 0x02;

    // Enable TX.
    UCSR0B= (1<<TXEN0);
    // Set the "normal" 8N1 UART frame mode.
    UCSR0C= (1<<UCSZ01) | (1<<UCSZ00);
    UCSR0A= 0x00;

    UCSR0A &= ~(1 << U2X0);
}

/**
 * Do some kit-kat with the LEDs to show we're alive.
 * This function resets the watchdog.
 */
static inline void bootAnimation() {
    PORTC = 0;
    for (int i = 0; i <= 1; ++i) {
        PORTC ^= LED0;
        _delay_ms(50);
        PORTC ^= LED1;
        _delay_ms(50);
        PORTC ^= LED2;
        _delay_ms(50);
        PORTC ^= LED3;
        _delay_ms(50);
        PORTC ^= LED4;
        _delay_ms(50);
        PORTC ^= LED5;
        _delay_ms(50);
        wdt_reset();
    }
    _delay_ms(500);
    PORTC = 0;
    wdt_reset();
}

/**
 * Main program loop
 */
static inline void loop() {
//    uint16_t crc = 0;
    uint8_t len;

    // 0xFF is a lot more recognisable as "bad data" then 0.
    memset(&packet, 0xFF, sizeof(Packet));
    packet.pre[0] = 0x42;
    packet.pre[1] = 0xAA;
    packet.pre[2] = 0xFF;
    packet.post[0] = 0x55;
    packet.post[1] = 0xAA;
    // Reset any previous buffer stuff
    memset(readLineBuffer, 0, readLineBufferSize);

    // Eat all garbage until we read a line that starts with '/' -> the packet header.
    while (readLineBuffer[0] != '/') {
        len = readLine();
    }
//    crc = crc16(crc, readLineBuffer, len);

    PORTC ^= LED2; // Show we are getting some data, alternate so it's not out too quickly.

    // Parse packet
    while (readLineBuffer[0] != '!') {
        len = readLine();
//        crc = crc16(crc, readLineBuffer, len);
        parseLine(len, readLineBuffer);
    }
//    crc = crc16(crc, "!", 1);

    wdt_reset();
    PORTC ^= LED4; // Sending packet
    sendPacket();

//    // Check CRC. If it did not match, send a specially crafted timestamped packet.
//    uint16_t expected_crc = strtol(readLineBuffer+1, NULL, 16);
//    if (expected_crc != crc) {
//        PORTC ^= LED1; // CRC bad LED
//        uint8_t payload[4];
//
//        payload[0] = crc & 0xFF;
//        payload[1] = crc >> 8;
//        payload[2] = expected_crc & 0xFF;
//        payload[3] = expected_crc >> 8;
//
//        error(ERROR_CRC, &payload, sizeof(payload));
//        // return;
//    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
int main (void)
{
    // First thing to do: Enable watchdog. Mainly to auto-reboot if we somehow crash.
    // The exact timeout isn't that important, we can miss a packet or two.
    // Keeping the timeout to >1s means we only need to poke it once every received packet.
    wdt_enable(WDTO_2S);

    // Set all pin data directions
    DDRA = 0x00;
    DDRC = 0x7F;

    // Make sure RF module is not in SET mode.
    // PORTC |= RF_SET;

    // Boot animation
    bootAnimation();

    // Now for the real work.
    meterUARTInit();
    RF_UART_Init();

    // enable all interrupts
    sei();

    error(ERROR_BOOT, NULL, 0);

    // Start requesting data from meter
    PORTC |= DATA_REQ;

    while (1) {
        wdt_reset();
        loop();
    }
}
#pragma clang diagnostic pop


void sendPacket() {
    packet.checksum = 0;
    packet.checksum = crc16(0, (void*) &packet, sizeof(Packet));

    while (tx_sending); // Wait for previous TX to be done, if any.
    tx_sending = true;

    memcpy((void*)tx_buffer, &packet, sizeof(Packet));
    tx_index = 0;
    UDR0 = tx_buffer[tx_index];
    UCSR0B |= (1<<UDRIE0);
}

void error(uint32_t code, void* payload, uint8_t len) {
    wdt_reset(); // Give us ample time to construct packet & send it out.

    PORTC |= LED0; // Indicate an error has happened.

    // Prevent programmer = idiot mistakes.
    if (len > ERROR_PAYLOAD_MAX_LEN) {
        len = ERROR_PAYLOAD_MAX_LEN;
    }

    // Construct error packet
    memset(&packet, 0, sizeof(Packet));
    packet.pre[0] = 0x42;
    packet.pre[1] = 0x55;
    packet.pre[1] = 0xAA;
    packet.post[0] = 0x55;
    packet.post[1] = 0xAA;
    packet.error = code | ERROR_BASE;
    packet.error_payload_len = len;
    memcpy(&packet.error_payload, payload, len);

    // This puts data in the que, but is likely to return immediately.
    sendPacket();

    // Wait for TX to be done
    wdt_reset();
    while (tx_sending);

    // Wait a bit longer, less chance error LEDs are missed.
    //wdt_reset();
    //_delay_ms(100);

    // Fatal error. Hang until watchdog resets entire chip.
    if (code & ERROR_FATAL) {
        while (1);
    }

    PORTC &= ~LED0; // Reset ERROR LED
}

uint8_t readLine() {
    int l = 0;
    while (l < readLineBufferSize) {
        while (rb1.readIndex == rb1.writeIndex);
        char c = rb1.buffer[rb1.readIndex++];
        readLineBuffer[l++] = c;
        if (c == '\n') break;
    }
    readLineBuffer[l] = 0;
    return l;
}
