#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct UserInterface * UserInterfaceHandle;

// Hardware abstraction: function pointers for UART communication
typedef void (*UartSendString_t)(const char * str);
typedef bool (*UartHasData_t)(void);
typedef bool (*UartReadChar_t)(char * c);

typedef struct {
    UartSendString_t SendString;
    UartHasData_t HasData;
    UartReadChar_t ReadChar;
} UartInterface;

typedef enum {
    UI_OK = 0,
    UI_ERROR_NULL_POINTER,
    UI_ERROR_NOT_INITIALIZED,
    UI_ERROR_INVALID_COMMAND,
    UI_ERROR_BUFFER_OVERFLOW,
    UI_ERROR_UART_TIMEOUT
} UserInterfaceError;

typedef enum {
    CMD_SET_DLI,
    CMD_GET_DLI,
    CMD_SET_PHOTOPERIOD,
    CMD_GET_PHOTOPERIOD,
    CMD_SET_RATIO,
    CMD_GET_RATIO,
    CMD_ENABLE_CONTROL,
    CMD_DISABLE_CONTROL,
    CMD_CALIBRATE,
    CMD_GET_STATUS,
    CMD_RESET,
    CMD_HELP,
    CMD_UNKNOWN
} CommandType;

typedef struct {
    CommandType type;
    uint16_t arg1;
    uint16_t arg2;
    char buffer[64];
} Command;

// Lifecycle
UserInterfaceHandle UserInterface_Create(UartInterface * uart);
void UserInterface_Destroy(UserInterfaceHandle handle);
UserInterfaceError UserInterface_Init(UserInterfaceHandle handle);

// Command processing
UserInterfaceError UserInterface_ProcessInput(UserInterfaceHandle handle, const char * input,
                                              Command * cmd);
UserInterfaceError UserInterface_SendResponse(UserInterfaceHandle handle, const char * response);

// Status display
UserInterfaceError UserInterface_DisplayPPFD(UserInterfaceHandle handle, uint16_t measured,
                                             uint16_t setpoint);
UserInterfaceError UserInterface_DisplayStatus(UserInterfaceHandle handle, const char * status);
UserInterfaceError UserInterface_DisplayError(UserInterfaceHandle handle, const char * error);

// Non-blocking I/O
UserInterfaceError UserInterface_Update(UserInterfaceHandle handle);
UserInterfaceError UserInterface_HasCommand(UserInterfaceHandle handle, bool * hasCommand);
UserInterfaceError UserInterface_GetCommand(UserInterfaceHandle handle, Command * cmd);

#endif // USER_INTERFACE_H
