#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
#include "IO.h"

#define COMMAND_REGISTER                        0x0
#define STATUS_REGISTER                         0x0
#define READ_STATUS_REGISTER                    0x70
#define PROGRAMM_COMMAND                        0x40
#define PROGRAM_ERASE_SUSPEND_COMMAND           0xB0
#define ERASE_CONFIRM_COMMAND                   0xD0
#define READ_CFI_QUERY_COMMAND                  0x98
#define RESET_COMMAND                           0xFF
#define DUMMY_ADDRESS                           0xA5
#define READY_BIT                               (1 << 7)
#define ERASE_COMPLETE_BIT                      (1 << 6)
#define PROGRAM_ERROR_BIT                       (1 << 4)
#define VPP_ERROR_BIT                           (1 << 3)
#define PROTECTED_BLOCK_BIT                     (1 << 1)
#define FLASH_WRITE_TIMEOUT_IN_MICROSECONDS     5000

#define CFI_MANUFACTURER_CODE_OFFSET            0x00
#define CFI_MANUFACTURER_CODE_DATA              0x0020
#define CFI_QUERY_UNIQUE_STRING_1_OFFSET        0x10
#define CFI_QUERY_UNIQUE_STRING_2_OFFSET        0x11
#define CFI_QUERY_UNIQUE_STRING_3_OFFSET        0x12
#define CFI_QUERY_UNIQUE_STRING_1_DATA          0x0051
#define CFI_QUERY_UNIQUE_STRING_2_DATA          0x0052
#define CFI_QUERY_UNIQUE_STRING_3_DATA          0x0059
#define CFI_VDD_LOGIC_SUPPLY_MIN_VOLTAGE_OFFSET 0x1B
#define CFI_VDD_LOGIC_SUPPLY_MIN_VOLTAGE_DATA   0x0027

enum
{
    FLASH_ERASE_CONTINUES = 2,
    FLASH_ERASE_COMPLETE = 1,
    FLASH_SUCCESS = 0,
    FLASH_VPP_ERROR = -1,
    FLASH_PROGRAM_ERROR = -2,
    FLASH_PROTECTED_BLOCK_ERROR = -3,
    FLASH_READ_BACK_ERROR = -4,
    FLASH_TIMEOUT_ERROR = -5,
    FLASH_UNKNOWN_PROGRAM_ERROR = -6
};


enum { EMPTY_CFI_FLASH_QUERY_DATA = 0 };


int8_t Flash_Write(ioAddress addr, ioData data);
int8_t Flash_EraseSupendAndResume(void);
ioData Flash_Query_CFI(ioAddress addr);
ioData Flash_CFIQuery_GetManufacturerData(void);
void Flash_CFIQuery_GetQueryUniqueString(ioData *string);
ioData Flash_CFIQuery_GetVddLogicSupplyMinimumVoltage(void);


#endif // FLASH_H
