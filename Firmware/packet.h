#ifndef FIRMWARE_PACKET_H
//
// Created by dries on 3/07/21.
//
#define FIRMWARE_PACKET_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define TIMESTAMP "0-0:1.0.0("
#define TARIFF "0-0:96.14.0("
#define METER_T1_DELIVERED "1-0:1.8.1("
#define METER_T2_DELIVERED "1-0:1.8.2("
#define METER_T1_INJECTED "1-0:2.8.1("
#define METER_T2_INJECTED "1-0:2.8.2("
#define METER_T2_INJECTED "1-0:2.8.2("
#define SUM_POWER_DELIVERED "1-0:1.7.0("
#define SUM_POWER_INJECTED "1-0:2.7.0("
#define POWER_P1_DELIVERED "1-0:21.7.0("
#define POWER_P2_DELIVERED "1-0:41.7.0("
#define POWER_P3_DELIVERED "1-0:61.7.0("
#define POWER_P1_INJECTED "1-0:22.7.0("
#define POWER_P2_INJECTED "1-0:42.7.0("
#define POWER_P3_INJECTED "1-0:62.7.0("
#define VOLTAGE_P1 "1-0:32.7.0("
#define VOLTAGE_P2 "1-0:52.7.0("
#define VOLTAGE_P3 "1-0:72.7.0("
#define CURRENT_P1 "1-0:31.7.0("
#define CURRENT_P2 "1-0:51.7.0("
#define CURRENT_P3 "1-0:71.7.0("
#define GAS_VOLUME "0-1:24.2.3("

#define ERROR_PAYLOAD_MAX_LEN 50

/**
 * This struct is the main packet that is send over UART
 * It must be <= 60 bytes to be able to transmit in mode 4 of the HC12 serial module.
 *
 * Important:
 *   Delivered => supplier -> client
 *   Injected => client -> supplier
 *
 * If a value is -1, it indicates the value was not present in the telegram.
 *
 * Error codes (timestamps):
 *  0xFFFF_FFFF     Blank telegram send. Usually a bad sign.
 *  0x8000_0000     No telegram received within expected timeframe. Optional.
 *  0x8000_0002     CRC mismatch.
 */
typedef struct
#if !defined(__linux__)
    __attribute__ ((packed))
#endif
{
    /**
     * Magic numbers [0x42, 0x55, 0xAA]
     */
    uint8_t pre[3];
union {
struct {
    /**
     * Timestamp in seconds since 2020-01-01 00:00:00
     * Values with msb set indicate errors. Treat payload after timestamp as raw bytes.
     * Source: 0-0:1.0.0
     */
    uint32_t timestamp;

    /**
     * Delivered meter reading for tariff 1 in Wh.
     * Source: 1-0:1.8.1
     */
    uint32_t meter_delivered_t1;

    /**
     * Delivered meter reading for tariff 2 in Wh.
     * Source: 1-0:1.8.2
     */
    uint32_t meter_delivered_t2;

    /**
     * Injected meter reading for tariff 1 in Wh.
     * Source: 1-0:2.8.1
     */
    uint32_t meter_injected_t1;

    /**
     * Injected meter reading for tariff 2 in Wh.
     * Source: 1-0:2.8.2
     */
    uint32_t meter_injected_t2;

    /**
     * Sum "actual" power for all phases in W.
     * Source: 1-0:1.7.0
     */
    uint16_t sum_power_delivered;

    /**
     * Sum "actual" power for all phases in W.
     * Source: 1-0:2.7.0
     */
    uint16_t sum_power_injected;

    /**
     * Instantaneous "actual" power for every phase in W.
     * Source: 1-0:21.7.0,1-0:41.7.0,1-0:61.7.0
     */
    uint16_t power_per_phase_delivered[3];

    /**
     * Instantaneous "actual" power for every phase in W.
     * Source: 1-0:22.7.0,1-0:42.7.0,1-0:62.7.0
     */
    uint16_t power_per_phase_injected[3];

    /**
     * Instantaneous voltage for every phase in .1V.
     * Source: 1-0:32.7.0,1-0:52.7.0,1-0:72.7.0
     */
    uint16_t voltage_per_phase[3];

    /**
     * Instantaneous "actual" power for every phase in .01A.
     * Source: 1-0:31.7.0,1-0:51.7.0,1-0:71.7.0
     */
    uint16_t current_per_phase[3];

    /**
     * Gas volume in 0.001m3.
     * Only present if meter present & connected.
     * Source: 0-1:24.2.3
     */
    uint32_t gas_volume;

    /**
     * Tariff currently in effect.
     * 1 -> Tariff 1 (normal)
     * 2 -> Tariff 2 (night/low)
     * Meaning of tariffs depends on region.
     * Source: 0-0:96.14.0
     */
    uint8_t tariff;

    /**
     * CRC16 checksum
     * Calculated over all of the previous bytes.
     */
    uint16_t checksum;
};
struct {
    uint32_t error;
    uint8_t error_payload_len;
    uint8_t error_payload[ERROR_PAYLOAD_MAX_LEN];
};
};
    /**
     * Magic numbers [0x55, 0xAA]
     */
    uint8_t post[2];
} Packet;

#if !defined(__linux__)
_Static_assert(sizeof(Packet) == 60, "Packet should be 60 bytes.");
#endif

extern Packet packet;

/**
 * Parse a single line of the telegram into the global variable packet.
 * @param n nr of characters that can be safely read from line.
 * @param line pointer to string buffer.
 * @return if this line contained parseable text that resulted in an assignment.
 */
bool parseLine(uint16_t n, const char* line);

#endif //FIRMWARE_PACKET_H
