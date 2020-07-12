#include <stdbool.h>
#include "Flash.h"
#include "MicroTime.h"


static int8_t writeError(ioData status);
static void resetAndReadDummyData(void);
static bool isTimeoutReached(uint32_t startTime);
static int8_t eraseStatus(ioData status);


int8_t Flash_Write(ioAddress address, ioData data)
{
    ioData status = 0;
    uint32_t timestamp = MicroTime_Get();

    IO_Write(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write(address, data);

    while ((status & READY_BIT) == 0) {
        status = IO_Read(STATUS_REGISTER);
        if ((isTimeoutReached(timestamp)))
            return FLASH_TIMEOUT_ERROR;
    }

    if (status != READY_BIT)
        return writeError(status);

    if (IO_Read(address) != data)
        return FLASH_READ_BACK_ERROR;

    return FLASH_SUCCESS;
}

int8_t Flash_EraseSupendAndResume(void)
{
    ioData status = 0;
    uint32_t timestamp = MicroTime_Get();

    IO_Write(COMMAND_REGISTER, PROGRAM_ERASE_SUSPEND_COMMAND);
    IO_Write(COMMAND_REGISTER, READ_STATUS_REGISTER);

    while ((status & READY_BIT) == 0) {
        status = IO_Read(STATUS_REGISTER);
        if (isTimeoutReached(timestamp))
            return FLASH_TIMEOUT_ERROR;
    }

    return eraseStatus(status);
}

ioData Flash_Query_CFI(ioAddress queryOffset)
{
    IO_Write(COMMAND_REGISTER, READ_CFI_QUERY_COMMAND);
    return IO_Read(queryOffset);
}

ioData Flash_CFIQuery_GetManufacturerData(void)
{
    return Flash_Query_CFI(CFI_MANUFACTURER_CODE_OFFSET);
}

void Flash_CFIQuery_GetQueryUniqueString(ioData *string)
{
    string[0] = Flash_Query_CFI(CFI_QUERY_UNIQUE_STRING_1_OFFSET);
    string[1] = Flash_Query_CFI(CFI_QUERY_UNIQUE_STRING_2_OFFSET);
    string[2] = Flash_Query_CFI(CFI_QUERY_UNIQUE_STRING_3_OFFSET);
}

ioData Flash_CFIQuery_GetVddLogicSupplyMinimumVoltage(void)
{
    return Flash_Query_CFI(CFI_VDD_LOGIC_SUPPLY_MIN_VOLTAGE_OFFSET);
}

static int8_t writeError(ioData status)
{
    IO_Write(COMMAND_REGISTER, RESET_COMMAND);

    if (status & VPP_ERROR_BIT)
        return FLASH_VPP_ERROR;
    else if (status & PROGRAM_ERROR_BIT)
        return FLASH_PROGRAM_ERROR;
    else if (status & PROTECTED_BLOCK_BIT)
        return FLASH_PROTECTED_BLOCK_ERROR;
    else
        return FLASH_UNKNOWN_PROGRAM_ERROR;
}

static void resetAndReadDummyData(void)
{
    IO_Write(COMMAND_REGISTER, RESET_COMMAND);
    IO_Read(DUMMY_ADDRESS);
}

static bool isTimeoutReached(uint32_t startTime)
{
    return (MicroTime_Get() - startTime >= FLASH_WRITE_TIMEOUT_IN_MICROSECONDS);
}

static int8_t eraseStatus(ioData status)
{
    if ((status & ERASE_COMPLETE_BIT) == 0) {
        resetAndReadDummyData();
        return FLASH_ERASE_COMPLETE;
    } else {
        resetAndReadDummyData();
        IO_Write(COMMAND_REGISTER, ERASE_CONFIRM_COMMAND);
        return FLASH_ERASE_CONTINUES;
    }
}
