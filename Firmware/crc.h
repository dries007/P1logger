#ifndef FIRMWARE_CRC_H
/*
 * Created by Dries007 on 2021/07/03.
 */
#define FIRMWARE_CRC_H

#include <stdint.h>
#include <stddef.h>

/**
 * CRC16, with correct parameters for DSMR standards (P1 port)
 * Sometimes known as "CRC-16-IBM" but inverted. (0xA001 ISO 0x8005)
 *
 * @param crc  Previous CRC or initial value (0x0000)
 * @param buf  Input bytes
 * @param len  Input length
 * @return crc value
 */
uint16_t crc16(uint16_t crc, const char *buf, int len);

#endif //FIRMWARE_CRC_H
