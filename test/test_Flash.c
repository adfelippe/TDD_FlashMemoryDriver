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
