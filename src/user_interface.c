#include "user_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UI_INPUT_BUFFER_SIZE 128

struct UserInterface {
    bool initialized;
    UartInterface * uart;
    char inputBuffer[UI_INPUT_BUFFER_SIZE];
    uint8_t inputIndex;
    bool commandReady;
    Command pendingCommand;
};

static struct UserInterface instance;

static bool isValidHandle(UserInterfaceHandle handle) {
    return handle != NULL;
}

// ========== Command Lookup Table ==========

typedef struct {
    const char * name;
    CommandType type;
    uint8_t numArgs;
} CommandEntry;

static const CommandEntry commandTable[] = {
    {"SET_DLI", CMD_SET_DLI, 1},
    {"GET_DLI", CMD_GET_DLI, 0},
    {"SET_PHOTOPERIOD", CMD_SET_PHOTOPERIOD, 1},
    {"GET_PHOTOPERIOD", CMD_GET_PHOTOPERIOD, 0},
    {"SET_RATIO", CMD_SET_RATIO, 2},
    {"GET_RATIO", CMD_GET_RATIO, 0},
    {"ENABLE", CMD_ENABLE_CONTROL, 0},
    {"DISABLE", CMD_DISABLE_CONTROL, 0},
    {"CALIBRATE", CMD_CALIBRATE, 0},
    {"STATUS", CMD_GET_STATUS, 0},
    {"RESET", CMD_RESET, 0},
    {"HELP", CMD_HELP, 0},
};

#define COMMAND_TABLE_SIZE (sizeof(commandTable) / sizeof(commandTable[0]))

// ========== Static Helpers ==========

static void ToUpperCase(char * str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = (char)(str[i] - ('a' - 'A'));
        }
    }
}

static const char * SkipWhitespace(const char * str) {
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    return str;
}

static UserInterfaceError ParseCommand(const char * input, Command * cmd) {
    char workBuffer[UI_INPUT_BUFFER_SIZE];
    memset(cmd, 0, sizeof(Command));
    cmd->type = CMD_UNKNOWN;

    input = SkipWhitespace(input);

    if (*input == '\0') {
        return UI_ERROR_INVALID_COMMAND;
    }

    // Copy input to command buffer
    strncpy(cmd->buffer, input, sizeof(cmd->buffer) - 1);
    cmd->buffer[sizeof(cmd->buffer) - 1] = '\0';

    // Copy to work buffer for parsing
    strncpy(workBuffer, input, sizeof(workBuffer) - 1);
    workBuffer[sizeof(workBuffer) - 1] = '\0';
    ToUpperCase(workBuffer);

    // Extract command name (first token)
    char * token = strtok(workBuffer, " \t");
    if (token == NULL) {
        return UI_ERROR_INVALID_COMMAND;
    }

    // Look up command in table
    bool found = false;
    uint8_t numArgs = 0;
    for (uint8_t i = 0; i < COMMAND_TABLE_SIZE; i++) {
        if (strcmp(token, commandTable[i].name) == 0) {
            cmd->type = commandTable[i].type;
            numArgs = commandTable[i].numArgs;
            found = true;
            break;
        }
    }

    if (!found) {
        return UI_ERROR_INVALID_COMMAND;
    }

    // Extract arguments
    // strtoul returns unsigned long per C99; no fixed-width alternative exists
    if (numArgs >= 1) {
        token = strtok(NULL, " \t");
        if (token == NULL) {
            cmd->type = CMD_UNKNOWN;
            return UI_ERROR_INVALID_COMMAND;
        }
        char * endptr;
        unsigned long val = strtoul(token, &endptr, 10);
        if (endptr == token) {
            cmd->type = CMD_UNKNOWN;
            return UI_ERROR_INVALID_COMMAND;
        }
        cmd->arg1 = (uint16_t)val;
    }

    if (numArgs >= 2) {
        token = strtok(NULL, " \t");
        if (token == NULL) {
            cmd->type = CMD_UNKNOWN;
            return UI_ERROR_INVALID_COMMAND;
        }
        char * endptr;
        unsigned long val = strtoul(token, &endptr, 10);
        if (endptr == token) {
            cmd->type = CMD_UNKNOWN;
            return UI_ERROR_INVALID_COMMAND;
        }
        cmd->arg2 = (uint16_t)val;
    }

    return UI_OK;
}

// ========== Lifecycle ==========

UserInterfaceHandle UserInterface_Create(UartInterface * uart) {
    memset(&instance, 0, sizeof(instance));
    instance.initialized = false;
    instance.uart = uart;
    instance.inputIndex = 0;
    instance.commandReady = false;
    return &instance;
}

void UserInterface_Destroy(UserInterfaceHandle handle) {
    if (isValidHandle(handle)) {
        handle->initialized = false;
        handle->commandReady = false;
        handle->inputIndex = 0;
        handle->uart = NULL;
    }
}

UserInterfaceError UserInterface_Init(UserInterfaceHandle handle) {
    if (!isValidHandle(handle)) {
        return UI_ERROR_NULL_POINTER;
    }
    handle->initialized = true;
    return UI_OK;
}

// ========== Command Processing ==========

UserInterfaceError UserInterface_ProcessInput(UserInterfaceHandle handle, const char * input,
                                              Command * cmd) {
    if (!isValidHandle(handle)) {
        return UI_ERROR_NULL_POINTER;
    }
    if (input == NULL || cmd == NULL) {
        return UI_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return UI_ERROR_NOT_INITIALIZED;
    }
    return ParseCommand(input, cmd);
}

UserInterfaceError UserInterface_SendResponse(UserInterfaceHandle handle, const char * response) {
    if (!isValidHandle(handle)) {
        return UI_ERROR_NULL_POINTER;
    }
    if (response == NULL) {
        return UI_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return UI_ERROR_NOT_INITIALIZED;
    }
    if (handle->uart != NULL && handle->uart->SendString != NULL) {
        handle->uart->SendString(response);
    }
    return UI_OK;
}

// ========== Status Display ==========

UserInterfaceError UserInterface_DisplayPPFD(UserInterfaceHandle handle, uint16_t measured,
                                             uint16_t setpoint) {
    if (!isValidHandle(handle)) {
        return UI_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return UI_ERROR_NOT_INITIALIZED;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "PPFD: %u/%u umol/m2/s\r\n", measured, setpoint);
    if (handle->uart != NULL && handle->uart->SendString != NULL) {
        handle->uart->SendString(buf);
    }
    return UI_OK;
}

UserInterfaceError UserInterface_DisplayStatus(UserInterfaceHandle handle, const char * status) {
    if (!isValidHandle(handle)) {
        return UI_ERROR_NULL_POINTER;
    }
    if (status == NULL) {
        return UI_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return UI_ERROR_NOT_INITIALIZED;
    }
    if (handle->uart != NULL && handle->uart->SendString != NULL) {
        handle->uart->SendString(status);
    }
    return UI_OK;
}

UserInterfaceError UserInterface_DisplayError(UserInterfaceHandle handle, const char * error) {
    if (!isValidHandle(handle)) {
        return UI_ERROR_NULL_POINTER;
    }
    if (error == NULL) {
        return UI_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return UI_ERROR_NOT_INITIALIZED;
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "ERROR: %s\r\n", error);
    if (handle->uart != NULL && handle->uart->SendString != NULL) {
        handle->uart->SendString(buf);
    }
    return UI_OK;
}

// ========== Non-Blocking I/O ==========

UserInterfaceError UserInterface_Update(UserInterfaceHandle handle) {
    if (!isValidHandle(handle)) {
        return UI_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return UI_ERROR_NOT_INITIALIZED;
    }
    if (handle->uart == NULL || handle->uart->HasData == NULL || handle->uart->ReadChar == NULL) {
        return UI_OK;
    }

    while (handle->uart->HasData()) {
        char c;
        if (!handle->uart->ReadChar(&c)) {
            break;
        }

        // Ignore carriage return, use newline as command terminator
        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            // Ignore empty lines
            if (handle->inputIndex == 0) {
                continue;
            }
            handle->inputBuffer[handle->inputIndex] = '\0';
            ParseCommand(handle->inputBuffer, &handle->pendingCommand);
            handle->commandReady = true;
            handle->inputIndex = 0;
            return UI_OK;
        }

        if (handle->inputIndex >= UI_INPUT_BUFFER_SIZE - 1) {
            handle->inputIndex = 0;
            return UI_ERROR_BUFFER_OVERFLOW;
        }

        handle->inputBuffer[handle->inputIndex++] = c;
    }

    return UI_OK;
}

UserInterfaceError UserInterface_HasCommand(UserInterfaceHandle handle, bool * hasCommand) {
    if (!isValidHandle(handle)) {
        return UI_ERROR_NULL_POINTER;
    }
    if (hasCommand == NULL) {
        return UI_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return UI_ERROR_NOT_INITIALIZED;
    }
    *hasCommand = handle->commandReady;
    return UI_OK;
}

UserInterfaceError UserInterface_GetCommand(UserInterfaceHandle handle, Command * cmd) {
    if (!isValidHandle(handle)) {
        return UI_ERROR_NULL_POINTER;
    }
    if (cmd == NULL) {
        return UI_ERROR_NULL_POINTER;
    }
    if (!handle->initialized) {
        return UI_ERROR_NOT_INITIALIZED;
    }
    *cmd = handle->pendingCommand;
    handle->commandReady = false;
    return UI_OK;
}
