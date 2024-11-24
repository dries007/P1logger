/*
 * Created by Dries007 on 2021/07/03.
 */

#include "crc.h"

uint16_t crc16(uint16_t crc, const char *buf, int len)
{
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];      // XOR byte into least sig. byte of crc
        for (int i = 8; i != 0; i--) {  // Loop over each bit
            if ((crc & 0x0001) != 0) {  // If the LSB is set
                crc >>= 1;              // Shift right
                crc ^= 0xA001;          // XOR 0xA001 (Polynomial)
            } else {                    // Else LSB is not set
                crc >>= 1;              // Just shift right
            }
        }
    }

    return crc;
}
