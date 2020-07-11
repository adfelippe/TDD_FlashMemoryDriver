#include "Flash.h"
#include "MicroTime.h"


static int8_t writeError(ioData status);


int8_t Flash_Write(ioAddress address, ioData data)
{
    ioData status = 0;
    uint32_t timestamp = MicroTime_Get();

    IO_Write(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write(address, data);

    while ((status & READY_BIT) == 0) {
        status = IO_Read(STATUS_REGISTER);
        if (MicroTime_Get() - timestamp >= FLASH_WRITE_TIMEOUT_IN_MICROSECONDS)
            return FLASH_TIMEOUT_ERROR;
    }

    if (status != READY_BIT)
        return writeError(status);

    if (IO_Read(address) != data)
        return FLASH_READ_BACK_ERROR;

    return FLASH_SUCCESS;
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
