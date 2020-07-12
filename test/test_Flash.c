#include "unity.h"
#include "Flash.h"
#include "mock_IO.h"
#include "FakeMicroTime.h"
#include "MicroTime.h"

ioAddress address;
ioData data;
int8_t result;

void setUp(void)
{
    address = 0x1000;
    data = 0xBEEF;
    result = -1;
}

void tearDown(void)
{
}

void sendCFIQueryCommand(ioAddress offset, ioData expectedData)
{
    IO_Write_Expect(COMMAND_REGISTER, 0x98);
    IO_Read_ExpectAndReturn(offset, expectedData);
}

void test_Flash_WriteSuceeds_ReadyImmediately(void)
{
    int result = 0;

    IO_Write_Expect(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write_Expect(address, data);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, READY_BIT);
    IO_Read_ExpectAndReturn(address, data);

    result = Flash_Write(address, data);

    TEST_ASSERT_EQUAL(FLASH_SUCCESS, result);
}

void test_Flash_ProgramSucceedsNotImmediatelyReady(void)
{
    IO_Write_Expect(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write_Expect(address, data);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, 0);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, 0);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, 0);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, READY_BIT);
    IO_Read_ExpectAndReturn(address, data);

    result = Flash_Write(address, data);

    TEST_ASSERT_EQUAL(FLASH_SUCCESS, result);
}

void test_Flash_WriteFails_VppError(void)
{
    IO_Write_Expect(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write_Expect(address, data);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, READY_BIT | VPP_ERROR_BIT);
    IO_Write_Expect(COMMAND_REGISTER, RESET_COMMAND);

    result = Flash_Write(address, data);

    TEST_ASSERT_EQUAL(FLASH_VPP_ERROR, result);
}

void test_Flash_WriteFails_ProgramError(void)
{
    IO_Write_Expect(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write_Expect(address, data);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, READY_BIT | PROGRAM_ERROR_BIT);
    IO_Write_Expect(COMMAND_REGISTER, RESET_COMMAND);

    result = Flash_Write(address, data);

    TEST_ASSERT_EQUAL(FLASH_PROGRAM_ERROR, result);
}

void test_Flash_WriteFails_ProtectedBlockError(void)
{
    IO_Write_Expect(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write_Expect(address, data);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, READY_BIT | PROTECTED_BLOCK_BIT);
    IO_Write_Expect(COMMAND_REGISTER, RESET_COMMAND);

    result = Flash_Write(address, data);

    TEST_ASSERT_EQUAL(FLASH_PROTECTED_BLOCK_ERROR, result);
}

void test_Flash_WriteFails_ReadFlashBackError(void)
{
    int result = 0;

    IO_Write_Expect(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write_Expect(address, data);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, READY_BIT);
    IO_Read_ExpectAndReturn(address, data - 1);

    result = Flash_Write(address, data);

    TEST_ASSERT_EQUAL(FLASH_READ_BACK_ERROR, result);
}

void test_Flash_WriteSucceeds_IgnoreOtherBitsUntilReady(void)
{
    IO_Write_Expect(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write_Expect(address, data);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, ~READY_BIT);
    IO_Read_ExpectAndReturn(STATUS_REGISTER, READY_BIT);
    IO_Read_ExpectAndReturn(address, data);

    result = Flash_Write(address, data);

    TEST_ASSERT_EQUAL(FLASH_SUCCESS, result);
}

void test_Flash_WriteFails_Timeout(void)
{
    FakeMicroTime_Init(0, 500);

    IO_Write_Expect(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write_Expect(address, data);

    for (int i = 0; i < 10; i++)
        IO_Read_ExpectAndReturn(STATUS_REGISTER, ~READY_BIT);

    result = Flash_Write(address, data);

    TEST_ASSERT_EQUAL(FLASH_TIMEOUT_ERROR, result);
}

void test_Flash_TimeoutAtEndOfTime(void)
{
    FakeMicroTime_Init(0xFFFFFFFF, 500);

    IO_Write_Expect(COMMAND_REGISTER, PROGRAMM_COMMAND);
    IO_Write_Expect(address, data);

    for (int i = 0; i < 10; i++)
        IO_Read_ExpectAndReturn(STATUS_REGISTER, ~READY_BIT);

    result = Flash_Write(address, data);

    TEST_ASSERT_EQUAL(FLASH_TIMEOUT_ERROR, result);
}

void test_Flash_EraseSuspendSucceedsImmediatelyWhenEraseComplete(void)
{
    IO_Write_Expect(0x0, 0xB0);
    IO_Write_Expect(0x0, 0x70);

    IO_Read_ExpectAndReturn(0x00, 1 << 7);

    IO_Write_Expect(0x0, 0xFF);
    IO_Read_ExpectAndReturn(0xA5, 0xA5);

    result = Flash_EraseSupendAndResume();

    TEST_ASSERT_EQUAL(FLASH_ERASE_COMPLETE, result);
}

void test_Flash_EraseSuspendSucceedsImmediatelyWhenEraseContinues(void)
{
    IO_Write_Expect(0x0, 0xB0);
    IO_Write_Expect(0x0, 0x70);

    IO_Read_ExpectAndReturn(0x00, (1 << 7 | 1 << 6));

    IO_Write_Expect(0x0, 0xFF);
    IO_Read_ExpectAndReturn(0xA5, 0xA5);
    IO_Write_Expect(0x0, 0xD0);

    result = Flash_EraseSupendAndResume();

    TEST_ASSERT_EQUAL(FLASH_ERASE_CONTINUES, result);
}

void test_Flash_EraseSuspendFailsTimeout(void)
{
    IO_Write_Expect(0x0, 0xB0);
    IO_Write_Expect(0x0, 0x70);

    for (int i = 0; i < 10; i++)
        IO_Read_ExpectAndReturn(0x00, ~(1 << 7));

    result = Flash_EraseSupendAndResume();

    TEST_ASSERT_EQUAL(FLASH_TIMEOUT_ERROR, result);
}

void test_Flash_FlashQueryCFIReturnsCorrectly(void)
{
    sendCFIQueryCommand(0x00, 0x0020);

    result = Flash_Query_CFI(CFI_MANUFACTURER_CODE_OFFSET);

    TEST_ASSERT_EQUAL(CFI_MANUFACTURER_CODE_DATA, result);
}

void test_Flash_CFIQuery_GetManufacturerDataSucceeds(void)
{
    sendCFIQueryCommand(0x00, 0x0020);

    result = Flash_CFIQuery_GetManufacturerData();

    TEST_ASSERT_EQUAL(CFI_MANUFACTURER_CODE_DATA, result);
}

void test_Flash_CFIQuery_GetManufacturerDataFailsIfWrongOffset(void)
{
    sendCFIQueryCommand(0x00, 0x0021);

    result = Flash_CFIQuery_GetManufacturerData();

    TEST_ASSERT_NOT_EQUAL(CFI_MANUFACTURER_CODE_DATA, result);
}

void test_Flash_CFIQuery_GetQueryUniqueStringSucceeds(void)
{
    ioData uniqueASCIIString[4] = {'\0', '\0', '\0', '\0'};
    sendCFIQueryCommand(0x10, 0x0051);
    sendCFIQueryCommand(0x11, 0x0052);
    sendCFIQueryCommand(0x12, 0x0059);

    Flash_CFIQuery_GetQueryUniqueString(uniqueASCIIString);

    TEST_ASSERT_EQUAL(CFI_QUERY_UNIQUE_STRING_1_DATA, uniqueASCIIString[0]);
    TEST_ASSERT_EQUAL(CFI_QUERY_UNIQUE_STRING_2_DATA, uniqueASCIIString[1]);
    TEST_ASSERT_EQUAL(CFI_QUERY_UNIQUE_STRING_3_DATA, uniqueASCIIString[2]);
}

void test_Flash_CFIQuery_GetVddLogicSupplyMinimumVoltageSucceeds(void)
{
    sendCFIQueryCommand(0x1B, 0x0027);

    result = Flash_CFIQuery_GetVddLogicSupplyMinimumVoltage();

    TEST_ASSERT_EQUAL(CFI_VDD_LOGIC_SUPPLY_MIN_VOLTAGE_DATA, result);
}
