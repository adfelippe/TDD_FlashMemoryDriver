#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
#include "IO.h"

#define COMMAND_REGISTER                        0x0
#define STATUS_REGISTER                         0x0
#define PROGRAMM_COMMAND                        0x40
#define RESET_COMMAND                           0xFF
#define READY_BIT                               (1 << 7)
#define PROGRAM_ERROR_BIT                       (1 << 4)
#define VPP_ERROR_BIT                           (1 << 3)
#define PROTECTED_BLOCK_BIT                     (1 << 1)
#define FLASH_WRITE_TIMEOUT_IN_MICROSECONDS     5000

enum
{
    FLASH_SUCCESS = 0,
    FLASH_VPP_ERROR = -1,
    FLASH_PROGRAM_ERROR = -2,
    FLASH_PROTECTED_BLOCK_ERROR = -3,
    FLASH_READ_BACK_ERROR = -4,
    FLASH_TIMEOUT_ERROR = -5,
    FLASH_UNKNOWN_PROGRAM_ERROR = -6
};


int8_t Flash_Write(ioAddress addr, ioData data);

#endif // FLASH_H
