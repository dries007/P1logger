cmake_minimum_required(VERSION 3.17)
project(Firmware C)

set(CMAKE_C_STANDARD 11)

SET(MCU "atmega128a")
SET(F_CPU "11059200")
SET(CMAKE_SYSTEM_NAME Generic)
# For some reason, these paths have to be absolute, otherwise
# CLion won't be able to find headers etc.
SET(CMAKE_C_COMPILER /usr/bin/avr-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/avr-g++)

SET(CMAKE_C_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os")
SET(CMAKE_C_LINK_FLAGS "-mmcu=${MCU}")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_executable(main main.c packet.c packet.h crc.c crc.h)
add_executable(main_x64 main_x64.c packet.c packet.h crc.c crc.h)
