#include "user_interface.h"
#include "unity.h"
#include <string.h>

static UserInterfaceHandle ui;
static UartInterface mockUart;

// Mock UART state
static char mockUartRxBuffer[256];
static uint16_t mockUartRxIndex;
static uint16_t mockUartRxLen;
static char mockUartTxBuffer[512];
static uint16_t mockUartTxLen;

// Mock UART functions
static void MockSendString(const char * str) {
    uint16_t len = (uint16_t)strlen(str);
    if (mockUartTxLen + len < sizeof(mockUartTxBuffer)) {
        memcpy(&mockUartTxBuffer[mockUartTxLen], str, len);
        mockUartTxLen += len;
        mockUartTxBuffer[mockUartTxLen] = '\0';
    }
}

static bool MockHasData(void) {
    return mockUartRxIndex < mockUartRxLen;
}

static bool MockReadChar(char * c) {
    if (mockUartRxIndex < mockUartRxLen) {
        *c = mockUartRxBuffer[mockUartRxIndex++];
        return true;
    }
    return false;
}

// Test helper
static void SimulateUartInput(const char * input) {
    uint16_t len = (uint16_t)strlen(input);
    memcpy(mockUartRxBuffer, input, len);
    mockUartRxLen = len;
    mockUartRxIndex = 0;
}

void setUp(void) {
    memset(mockUartRxBuffer, 0, sizeof(mockUartRxBuffer));
    mockUartRxIndex = 0;
    mockUartRxLen = 0;
    memset(mockUartTxBuffer, 0, sizeof(mockUartTxBuffer));
    mockUartTxLen = 0;

    mockUart.SendString = MockSendString;
    mockUart.HasData = MockHasData;
    mockUart.ReadChar = MockReadChar;

    ui = UserInterface_Create(&mockUart);
}

void tearDown(void) {
    UserInterface_Destroy(ui);
}

// ========== Lifecycle Tests ==========

void test_UserInterface_CanBeCreated(void) {
    TEST_ASSERT_NOT_NULL(ui);
}

void test_UserInterface_Create_ReturnsNonNullWithValidUart(void) {
    UserInterfaceHandle h = UserInterface_Create(&mockUart);
    TEST_ASSERT_NOT_NULL(h);
}

void test_UserInterface_Create_AcceptsNullUart(void) {
    UserInterfaceHandle h = UserInterface_Create(NULL);
    TEST_ASSERT_NOT_NULL(h);
}

void test_UserInterface_InitializesSuccessfully(void) {
    UserInterfaceError result = UserInterface_Init(ui);
    TEST_ASSERT_EQUAL(UI_OK, result);
}

void test_UserInterface_Init_RejectsNullHandle(void) {
    UserInterfaceError result = UserInterface_Init(NULL);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_Destroy_CanBeCalledSafely(void) {
    UserInterface_Destroy(ui);
    // Should not crash; recreate for tearDown
    ui = UserInterface_Create(&mockUart);
}

void test_UserInterface_Destroy_AcceptsNullHandle(void) {
    UserInterface_Destroy(NULL);
    // Should not crash
}

void test_UserInterface_HasNoCommandAfterInit(void) {
    UserInterface_Init(ui);
    bool hasCmd = true;
    UserInterfaceError result = UserInterface_HasCommand(ui, &hasCmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_FALSE(hasCmd);
}

// ========== ProcessInput - Valid Commands ==========

void test_UserInterface_ProcessInput_ParsesHelp(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "HELP", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_HELP, cmd.type);
}

void test_UserInterface_ProcessInput_ParsesStatus(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "STATUS", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_GET_STATUS, cmd.type);
}

void test_UserInterface_ProcessInput_ParsesEnable(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "ENABLE", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_ENABLE_CONTROL, cmd.type);
}

void test_UserInterface_ProcessInput_ParsesDisable(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "DISABLE", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_DISABLE_CONTROL, cmd.type);
}

void test_UserInterface_ProcessInput_ParsesCalibrate(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "CALIBRATE", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_CALIBRATE, cmd.type);
}

void test_UserInterface_ProcessInput_ParsesReset(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "RESET", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_RESET, cmd.type);
}

void test_UserInterface_ProcessInput_ParsesGetDLI(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "GET_DLI", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_GET_DLI, cmd.type);
}

void test_UserInterface_ProcessInput_ParsesGetPhotoperiod(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "GET_PHOTOPERIOD", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_GET_PHOTOPERIOD, cmd.type);
}

void test_UserInterface_ProcessInput_ParsesGetRatio(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "GET_RATIO", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_GET_RATIO, cmd.type);
}

void test_UserInterface_ProcessInput_ParsesSetDLI(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "SET_DLI 500", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_SET_DLI, cmd.type);
    TEST_ASSERT_EQUAL_UINT16(500, cmd.arg1);
}

void test_UserInterface_ProcessInput_ParsesSetPhotoperiod(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "SET_PHOTOPERIOD 16", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_SET_PHOTOPERIOD, cmd.type);
    TEST_ASSERT_EQUAL_UINT16(16, cmd.arg1);
}

void test_UserInterface_ProcessInput_ParsesSetRatio(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "SET_RATIO 40 30", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_SET_RATIO, cmd.type);
    TEST_ASSERT_EQUAL_UINT16(40, cmd.arg1);
    TEST_ASSERT_EQUAL_UINT16(30, cmd.arg2);
}

// ========== ProcessInput - Argument Extraction ==========

void test_UserInterface_ProcessInput_SetDLIStoresValueInArg1(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterface_ProcessInput(ui, "SET_DLI 12345", &cmd);
    TEST_ASSERT_EQUAL_UINT16(12345, cmd.arg1);
}

void test_UserInterface_ProcessInput_SetRatioStoresArg1AndArg2(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterface_ProcessInput(ui, "SET_RATIO 50 25", &cmd);
    TEST_ASSERT_EQUAL_UINT16(50, cmd.arg1);
    TEST_ASSERT_EQUAL_UINT16(25, cmd.arg2);
}

void test_UserInterface_ProcessInput_CopiesRawInputToBuffer(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterface_ProcessInput(ui, "SET_DLI 500", &cmd);
    TEST_ASSERT_EQUAL_STRING("SET_DLI 500", cmd.buffer);
}

void test_UserInterface_ProcessInput_HandlesLargeArgumentValues(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterface_ProcessInput(ui, "SET_DLI 65535", &cmd);
    TEST_ASSERT_EQUAL_UINT16(65535, cmd.arg1);
}

// ========== ProcessInput - Error Handling ==========

void test_UserInterface_ProcessInput_RejectsNullHandle(void) {
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(NULL, "HELP", &cmd);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_ProcessInput_RejectsNullInput(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, NULL, &cmd);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_ProcessInput_RejectsNullCommand(void) {
    UserInterface_Init(ui);
    UserInterfaceError result = UserInterface_ProcessInput(ui, "HELP", NULL);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_ProcessInput_RequiresInit(void) {
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "HELP", &cmd);
    TEST_ASSERT_EQUAL(UI_ERROR_NOT_INITIALIZED, result);
}

void test_UserInterface_ProcessInput_ReturnsInvalidForEmptyString(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "", &cmd);
    TEST_ASSERT_EQUAL(UI_ERROR_INVALID_COMMAND, result);
}

void test_UserInterface_ProcessInput_ReturnsInvalidForUnknownCommand(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "FOOBAR", &cmd);
    TEST_ASSERT_EQUAL(UI_ERROR_INVALID_COMMAND, result);
    TEST_ASSERT_EQUAL(CMD_UNKNOWN, cmd.type);
}

void test_UserInterface_ProcessInput_ReturnsInvalidForMissingArg(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "SET_DLI", &cmd);
    TEST_ASSERT_EQUAL(UI_ERROR_INVALID_COMMAND, result);
}

void test_UserInterface_ProcessInput_IsCaseInsensitive(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "help", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_HELP, cmd.type);
}

void test_UserInterface_ProcessInput_RejectsNonNumericArg(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "SET_DLI abc", &cmd);
    TEST_ASSERT_EQUAL(UI_ERROR_INVALID_COMMAND, result);
    TEST_ASSERT_EQUAL(CMD_UNKNOWN, cmd.type);
}

void test_UserInterface_ProcessInput_RejectsNonNumericSecondArg(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "SET_RATIO 40 xyz", &cmd);
    TEST_ASSERT_EQUAL(UI_ERROR_INVALID_COMMAND, result);
    TEST_ASSERT_EQUAL(CMD_UNKNOWN, cmd.type);
}

void test_UserInterface_ProcessInput_TrimsLeadingWhitespace(void) {
    UserInterface_Init(ui);
    Command cmd;
    UserInterfaceError result = UserInterface_ProcessInput(ui, "  HELP", &cmd);
    TEST_ASSERT_EQUAL(UI_OK, result);
    TEST_ASSERT_EQUAL(CMD_HELP, cmd.type);
}

// ========== SendResponse Tests ==========

void test_UserInterface_SendResponse_RejectsNullHandle(void) {
    UserInterfaceError result = UserInterface_SendResponse(NULL, "OK");
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_SendResponse_RejectsNullString(void) {
    UserInterface_Init(ui);
    UserInterfaceError result = UserInterface_SendResponse(ui, NULL);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_SendResponse_RequiresInit(void) {
    UserInterfaceError result = UserInterface_SendResponse(ui, "OK");
    TEST_ASSERT_EQUAL(UI_ERROR_NOT_INITIALIZED, result);
}

void test_UserInterface_SendResponse_CallsUartSendString(void) {
    UserInterface_Init(ui);
    UserInterface_SendResponse(ui, "OK\r\n");
    TEST_ASSERT_EQUAL_STRING("OK\r\n", mockUartTxBuffer);
}

// ========== DisplayPPFD Tests ==========

void test_UserInterface_DisplayPPFD_RejectsNullHandle(void) {
    UserInterfaceError result = UserInterface_DisplayPPFD(NULL, 100, 200);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_DisplayPPFD_RequiresInit(void) {
    UserInterfaceError result = UserInterface_DisplayPPFD(ui, 100, 200);
    TEST_ASSERT_EQUAL(UI_ERROR_NOT_INITIALIZED, result);
}

void test_UserInterface_DisplayPPFD_FormatsAndSends(void) {
    UserInterface_Init(ui);
    UserInterface_DisplayPPFD(ui, 450, 500);
    TEST_ASSERT_EQUAL_STRING("PPFD: 450/500 umol/m2/s\r\n", mockUartTxBuffer);
}

// ========== DisplayStatus Tests ==========

void test_UserInterface_DisplayStatus_RejectsNullHandle(void) {
    UserInterfaceError result = UserInterface_DisplayStatus(NULL, "IDLE");
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_DisplayStatus_RejectsNullString(void) {
    UserInterface_Init(ui);
    UserInterfaceError result = UserInterface_DisplayStatus(ui, NULL);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_DisplayStatus_RequiresInit(void) {
    UserInterfaceError result = UserInterface_DisplayStatus(ui, "IDLE");
    TEST_ASSERT_EQUAL(UI_ERROR_NOT_INITIALIZED, result);
}

void test_UserInterface_DisplayStatus_SendsStatusString(void) {
    UserInterface_Init(ui);
    UserInterface_DisplayStatus(ui, "IDLE");
    TEST_ASSERT_EQUAL_STRING("IDLE", mockUartTxBuffer);
}

// ========== DisplayError Tests ==========

void test_UserInterface_DisplayError_RejectsNullHandle(void) {
    UserInterfaceError result = UserInterface_DisplayError(NULL, "timeout");
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_DisplayError_RejectsNullString(void) {
    UserInterface_Init(ui);
    UserInterfaceError result = UserInterface_DisplayError(ui, NULL);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_DisplayError_RequiresInit(void) {
    UserInterfaceError result = UserInterface_DisplayError(ui, "timeout");
    TEST_ASSERT_EQUAL(UI_ERROR_NOT_INITIALIZED, result);
}

void test_UserInterface_DisplayError_FormatsWithErrorPrefix(void) {
    UserInterface_Init(ui);
    UserInterface_DisplayError(ui, "timeout");
    TEST_ASSERT_EQUAL_STRING("ERROR: timeout\r\n", mockUartTxBuffer);
}

// ========== Update Tests ==========

void test_UserInterface_Update_RejectsNullHandle(void) {
    UserInterfaceError result = UserInterface_Update(NULL);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_Update_RequiresInit(void) {
    UserInterfaceError result = UserInterface_Update(ui);
    TEST_ASSERT_EQUAL(UI_ERROR_NOT_INITIALIZED, result);
}

void test_UserInterface_Update_ReturnsOkWhenNoData(void) {
    UserInterface_Init(ui);
    // No data in mock RX buffer
    UserInterfaceError result = UserInterface_Update(ui);
    TEST_ASSERT_EQUAL(UI_OK, result);
}

void test_UserInterface_Update_DetectsNewlineAsCommandEnd(void) {
    UserInterface_Init(ui);
    SimulateUartInput("HELP\n");
    UserInterfaceError result = UserInterface_Update(ui);
    TEST_ASSERT_EQUAL(UI_OK, result);

    bool hasCmd = false;
    UserInterface_HasCommand(ui, &hasCmd);
    TEST_ASSERT_TRUE(hasCmd);
}

void test_UserInterface_Update_DetectsCarriageReturnNewline(void) {
    UserInterface_Init(ui);
    SimulateUartInput("HELP\r\n");
    UserInterface_Update(ui);

    bool hasCmd = false;
    UserInterface_HasCommand(ui, &hasCmd);
    TEST_ASSERT_TRUE(hasCmd);
}

void test_UserInterface_Update_SetsCommandReadyOnNewline(void) {
    UserInterface_Init(ui);
    SimulateUartInput("STATUS\n");
    UserInterface_Update(ui);

    Command cmd;
    UserInterface_GetCommand(ui, &cmd);
    TEST_ASSERT_EQUAL(CMD_GET_STATUS, cmd.type);
}

void test_UserInterface_Update_ReturnsOverflowWhenBufferFull(void) {
    UserInterface_Init(ui);
    // Fill with 128+ characters without newline
    char longInput[140];
    memset(longInput, 'A', sizeof(longInput));
    longInput[139] = '\0';
    SimulateUartInput(longInput);
    UserInterfaceError result = UserInterface_Update(ui);
    TEST_ASSERT_EQUAL(UI_ERROR_BUFFER_OVERFLOW, result);
}

void test_UserInterface_Update_IgnoresEmptyLines(void) {
    UserInterface_Init(ui);
    SimulateUartInput("\n");
    UserInterface_Update(ui);

    bool hasCmd = false;
    UserInterface_HasCommand(ui, &hasCmd);
    TEST_ASSERT_FALSE(hasCmd);
}

void test_UserInterface_Update_ParsesCommandWithArguments(void) {
    UserInterface_Init(ui);
    SimulateUartInput("SET_DLI 500\n");
    UserInterface_Update(ui);

    Command cmd;
    UserInterface_GetCommand(ui, &cmd);
    TEST_ASSERT_EQUAL(CMD_SET_DLI, cmd.type);
    TEST_ASSERT_EQUAL_UINT16(500, cmd.arg1);
}

// ========== HasCommand Tests ==========

void test_UserInterface_HasCommand_RejectsNullHandle(void) {
    bool hasCmd = false;
    UserInterfaceError result = UserInterface_HasCommand(NULL, &hasCmd);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_HasCommand_RejectsNullPointer(void) {
    UserInterface_Init(ui);
    UserInterfaceError result = UserInterface_HasCommand(ui, NULL);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_HasCommand_RequiresInit(void) {
    bool hasCmd = false;
    UserInterfaceError result = UserInterface_HasCommand(ui, &hasCmd);
    TEST_ASSERT_EQUAL(UI_ERROR_NOT_INITIALIZED, result);
}

void test_UserInterface_HasCommand_ReturnsFalseInitially(void) {
    UserInterface_Init(ui);
    bool hasCmd = true;
    UserInterface_HasCommand(ui, &hasCmd);
    TEST_ASSERT_FALSE(hasCmd);
}

void test_UserInterface_HasCommand_ReturnsTrueAfterCompleteInput(void) {
    UserInterface_Init(ui);
    SimulateUartInput("HELP\n");
    UserInterface_Update(ui);

    bool hasCmd = false;
    UserInterface_HasCommand(ui, &hasCmd);
    TEST_ASSERT_TRUE(hasCmd);
}

// ========== GetCommand Tests ==========

void test_UserInterface_GetCommand_RejectsNullHandle(void) {
    Command cmd;
    UserInterfaceError result = UserInterface_GetCommand(NULL, &cmd);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_GetCommand_RejectsNullPointer(void) {
    UserInterface_Init(ui);
    UserInterfaceError result = UserInterface_GetCommand(ui, NULL);
    TEST_ASSERT_EQUAL(UI_ERROR_NULL_POINTER, result);
}

void test_UserInterface_GetCommand_RequiresInit(void) {
    Command cmd;
    UserInterfaceError result = UserInterface_GetCommand(ui, &cmd);
    TEST_ASSERT_EQUAL(UI_ERROR_NOT_INITIALIZED, result);
}

void test_UserInterface_GetCommand_ClearsCommandReadyFlag(void) {
    UserInterface_Init(ui);
    SimulateUartInput("HELP\n");
    UserInterface_Update(ui);

    Command cmd;
    UserInterface_GetCommand(ui, &cmd);

    bool hasCmd = true;
    UserInterface_HasCommand(ui, &hasCmd);
    TEST_ASSERT_FALSE(hasCmd);
}

// ========== Integration Tests ==========

void test_UserInterface_FullCycle_UpdateThenHasCommandThenGet(void) {
    UserInterface_Init(ui);
    SimulateUartInput("SET_PHOTOPERIOD 18\n");
    UserInterface_Update(ui);

    bool hasCmd = false;
    UserInterface_HasCommand(ui, &hasCmd);
    TEST_ASSERT_TRUE(hasCmd);

    Command cmd;
    UserInterface_GetCommand(ui, &cmd);
    TEST_ASSERT_EQUAL(CMD_SET_PHOTOPERIOD, cmd.type);
    TEST_ASSERT_EQUAL_UINT16(18, cmd.arg1);

    // Command consumed
    UserInterface_HasCommand(ui, &hasCmd);
    TEST_ASSERT_FALSE(hasCmd);
}

void test_UserInterface_MultipleCommandsSequential(void) {
    UserInterface_Init(ui);

    // First command
    SimulateUartInput("HELP\n");
    UserInterface_Update(ui);
    Command cmd;
    UserInterface_GetCommand(ui, &cmd);
    TEST_ASSERT_EQUAL(CMD_HELP, cmd.type);

    // Second command
    SimulateUartInput("STATUS\n");
    UserInterface_Update(ui);
    UserInterface_GetCommand(ui, &cmd);
    TEST_ASSERT_EQUAL(CMD_GET_STATUS, cmd.type);
}

void test_UserInterface_NullUart_DoesNotCrashOnUpdate(void) {
    UserInterface_Destroy(ui);
    ui = UserInterface_Create(NULL);
    UserInterface_Init(ui);
    UserInterfaceError result = UserInterface_Update(ui);
    TEST_ASSERT_EQUAL(UI_OK, result);
}

void test_UserInterface_NullUart_DoesNotCrashOnSendResponse(void) {
    UserInterface_Destroy(ui);
    ui = UserInterface_Create(NULL);
    UserInterface_Init(ui);
    UserInterfaceError result = UserInterface_SendResponse(ui, "test");
    TEST_ASSERT_EQUAL(UI_OK, result);
}
