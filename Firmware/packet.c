/*
 * Created by Dries007 on 2021/07/03.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "packet.h"

Packet packet;

/**
 * Parse a value from a line if the prefix is present, optionally skipping an additional timestamp value.
 * The value is stored as if it was an integer, the decimal point is entirely ignored if present.
 *
 * @param inp Input line of text. No null terminator is required.
 * @param len Length of inp.
 * @param prefix The prefix we should expect before accepting this value.
 * @param store The output. Untouched if prefix is a mismatch.
 * @param timestamped If true, an additional timestamp is ignored before the value is read.
 * @return true if a value was stored in the store.
 */
static bool parseValue32(const char* const inp, const size_t len, const char* const prefix, uint32_t* const store, const bool timestamped) {
    size_t i = strlen(prefix);
    if (strncmp(inp, prefix, i) != 0) return false;
    if (timestamped) {
        while (i < len && inp[i++] != '(');
    }
    *store = 0;
    while (i < len && inp[i] != '*') {
        char c = inp[i++];
        if (!isdigit(c)) continue; // Skip over '.'
        *store *= 10;
        *store += c - '0';
    }
    return true;
}

/**
 * Parse a value from a line if the prefix is present.
 * The value is stored as if it was an integer, the decimal point is entirely ignored if present.
 *
 * @param inp Input line of text. No null terminator is required.
 * @param len Length of inp.
 * @param prefix The prefix we should expect before accepting this value.
 * @param store The output. Untouched if prefix is a mismatch.
 * @return true if a value was stored in the store.
 */
static bool parseValue16(const char* const inp, const size_t len, const char* const prefix, uint16_t* const store) {
    size_t i = strlen(prefix);
    if (strncmp(inp, prefix, i) != 0) return false;
    *store = 0;
    while (i < len && inp[i] != '*') {
        char c = inp[i++];
        if (!isdigit(c)) continue; // Skip over '.'
        *store *= 10;
        *store += c - '0';
    }
    return true;
}

/**
 * Parse a value from a line if the prefix is present.
 * The value is stored as if it was an integer, the decimal point is entirely ignored if present.
 *
 * @param inp Input line of text. No null terminator is required.
 * @param len Length of inp.
 * @param prefix The prefix we should expect before accepting this value.
 * @param store The output. Untouched if prefix is a mismatch.
 * @return true if a value was stored in the store.
 */
static bool parseValue8(const char *const inp, const size_t len, const char *const prefix, uint8_t *const store) {
    size_t i = strlen(prefix);
    if (strncmp(inp, prefix, i) != 0) return false;
    *store = 0;
    while (i < len && inp[i] != ')') {
        char c = inp[i++];
        if (!isdigit(c)) continue; // Skip over '.'
        *store *= 10;
        *store += c - '0';
    }
    return true;
}

static inline int doubleDigitNumber(const char *const inp, const size_t i) {
    char c1 = inp[i];
    char c2 = inp[i+1];
    if (!(isdigit(c1) && isdigit(c2))) return -1;
    return (c1 - '0') * 10 + (c2 - '0');
}

/**
 * Parse a value from a line if the prefix is present.
 * The value is stored as if it was an integer, the decimal point is entirely ignored if present.
 *
 * @param inp Input line of text. No null terminator is required.
 * @param len Length of inp.
 * @param prefix The prefix we should expect before accepting this value.
 * @param store The output. Untouched if prefix is a mismatch.
 * @return true if a value was stored in the store.
 */
static bool parseValueTimestamp(const char *const inp, const size_t len, const char *const prefix, uint32_t *const store) {
    if (len < 24) return false;
    size_t i = strlen(prefix);
    if (strncmp(inp, prefix, i) != 0) return false;

//    printf("Timestamp: %s %c\n", inp + i, inp[i+12]);

    // YYMMDDhhmmssz
    // 200512135409S
    struct tm time = {
            .tm_year = doubleDigitNumber(inp, i) + 100,    // year starts at 1900
            .tm_mon = doubleDigitNumber(inp, i + 2) - 1, // Month starts with 0
            .tm_mday = doubleDigitNumber(inp, i + 4),
            .tm_hour = doubleDigitNumber(inp, i + 6),
            .tm_min = doubleDigitNumber(inp, i + 8),
            .tm_sec = doubleDigitNumber(inp, i + 10),
            .tm_isdst = inp[i+12] == 'S' ? 1 : 0,
    };

    *store = mktime(&time);
    return true;
}


bool parseLine(const uint16_t n, const char *line) {
    if (n < 14) return false;

    if (parseValueTimestamp(line, n, TIMESTAMP, &packet.timestamp)) return true;
    if (parseValue8(line, n, TARIFF, &packet.tariff)) return true;
    if (parseValue32(line, n, METER_T1_DELIVERED, &packet.meter_delivered_t1, false)) return true;
    if (parseValue32(line, n, METER_T2_DELIVERED, &packet.meter_delivered_t2, false)) return true;
    if (parseValue32(line, n, METER_T1_INJECTED, &packet.meter_injected_t1, false)) return true;
    if (parseValue32(line, n, METER_T2_INJECTED, &packet.meter_injected_t2, false)) return true;
    if (parseValue16(line, n, SUM_POWER_DELIVERED, &packet.sum_power_delivered)) return true;
    if (parseValue16(line, n, SUM_POWER_INJECTED, &packet.sum_power_injected)) return true;
    if (parseValue16(line, n, POWER_P1_DELIVERED, &packet.power_per_phase_delivered[0])) return true;
    if (parseValue16(line, n, POWER_P2_DELIVERED, &packet.power_per_phase_delivered[1])) return true;
    if (parseValue16(line, n, POWER_P3_DELIVERED, &packet.power_per_phase_delivered[2])) return true;
    if (parseValue16(line, n, POWER_P1_INJECTED, &packet.power_per_phase_injected[0])) return true;
    if (parseValue16(line, n, POWER_P2_INJECTED, &packet.power_per_phase_injected[1])) return true;
    if (parseValue16(line, n, POWER_P3_INJECTED, &packet.power_per_phase_injected[2])) return true;
    if (parseValue16(line, n, VOLTAGE_P1, &packet.voltage_per_phase[0])) return true;
    if (parseValue16(line, n, VOLTAGE_P2, &packet.voltage_per_phase[1])) return true;
    if (parseValue16(line, n, VOLTAGE_P3, &packet.voltage_per_phase[2])) return true;
    if (parseValue16(line, n, CURRENT_P1, &packet.current_per_phase[0])) return true;
    if (parseValue16(line, n, CURRENT_P2, &packet.current_per_phase[1])) return true;
    if (parseValue16(line, n, CURRENT_P3, &packet.current_per_phase[2])) return true;
    //if (parseValue16(line, n, LIMITER, &packet.limiter)) return true;
    //if (parseValue16(line, n, FUSE_SUPERVISION, &packet.fuse_supervision)) return true;
    if (parseValue32(line, n, GAS_VOLUME, &packet.gas_volume, true)) return true;

    return false;
}
