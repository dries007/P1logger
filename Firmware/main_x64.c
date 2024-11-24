#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "packet.h"
#include "crc.h"

void error(const char *msg) {
    puts(msg);
    exit(-1);
}

void printPacket() {
    time_t timestamp = packet.timestamp;
    struct tm* time = gmtime(&timestamp);
    printf("timestamp: %d -> %s\n", packet.timestamp, asctime(time));
    printf("meter_delivered_t1: %d\n", packet.meter_delivered_t1);
    printf("meter_delivered_t2: %d\n", packet.meter_delivered_t2);
    printf("meter_injected_t1: %d\n", packet.meter_injected_t1);
    printf("meter_injected_t2: %d\n", packet.meter_injected_t2);
    printf("sum_power_delivered: %d\n", packet.sum_power_delivered);
    printf("sum_power_injected: %d\n", packet.sum_power_injected);
    printf("power_per_phase_delivered 0: %d\n", packet.power_per_phase_delivered[0]);
    printf("power_per_phase_delivered 1: %d\n", packet.power_per_phase_delivered[1]);
    printf("power_per_phase_delivered 2: %d\n", packet.power_per_phase_delivered[2]);
    printf("power_per_phase_injected 0: %d\n", packet.power_per_phase_injected[0]);
    printf("power_per_phase_injected 1: %d\n", packet.power_per_phase_injected[1]);
    printf("power_per_phase_injected 2: %d\n", packet.power_per_phase_injected[2]);
    printf("voltage_per_phase 0: %d\n", packet.voltage_per_phase[0]);
    printf("voltage_per_phase 1: %d\n", packet.voltage_per_phase[1]);
    printf("voltage_per_phase 2: %d\n", packet.voltage_per_phase[2]);
    printf("current_per_phase 0: %d\n", packet.current_per_phase[0]);
    printf("current_per_phase 1: %d\n", packet.current_per_phase[1]);
    printf("current_per_phase 2: %d\n", packet.current_per_phase[2]);
//    printf("limiter: %d\n", packet.limiter);
//    printf("fuse_supervision: %d\n", packet.fuse_supervision);
    printf("gas_volume: %d\n", packet.gas_volume);
    printf("tariff: %d\n", packet.tariff);
    printf("checksum: 0x%X\n", packet.checksum);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        error("Wrong nr of args. Must be 1 arg, packet filename.");
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL) {
        error("Failed to open file.");
    }

    while (!feof(fp)) {
        memset(&packet, 0xFF, sizeof(Packet));

        char * line = NULL;
        size_t len = 0;
        ssize_t read;
        uint16_t crc;
        while ((read = getline(&line, &len, fp)) != -1) {
            crc = crc16(crc, line, line[0] != '!' ? read : 1);

            printf("Read n=%2zu: %s", read, line);
            if (parseLine(read, line)) {
//            puts("Y");
            } else {
//            puts("N");
            }

            if (line[0] == '!') {
                break;
            }
        }

        printf("CRC: %X\n", crc);
    }



//    packet.checksum = crc16(0, (void*)&packet, sizeof(Packet)-2);
//    printPacket();
}
