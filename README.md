# Controlador Multicanal de PPFD para Ambientes Controlados

## Descripción General del Proyecto

Este proyecto forma parte del trabajo final de la Especialización en Sistemas Embebidos (CESE) de la FIUBA. El sistema completo ha sido desarrollado empleando la metodología **Test-Driven Development (TDD)**, siguiendo los lineamientos establecidos por el libro "Test-Driven Development for Embedded C".

El proyecto implementa un **controlador completo de PPFD (Photosynthetic Photon Flux Density)** multicanal para aplicaciones de cultivo en ambientes controlados. El sistema permite:

- **Control automático de intensidad luminosa** basado en setpoint de PPFD
- **Control proporcional (PI)** en lazo cerrado con sensor de PPFD
- **Gestión de calibración** del sensor con datos persistentes
- **Múltiples canales LED** (Rojo, Azul, Far-Red, White)
- **Máquina de estados** para coordinación de modos operativos
- **Configuración por DLI** (Daily Light Integral) con validación de parámetros

## Estado Actual del Desarrollo

### ✅ 205 Tests Unitarios Pasando

El proyecto cuenta con una suite de tests que valida todos los módulos implementados:

| Módulo | Tests | Descripción |
|--------|-------|-------------|
| **ConfigValidator** | 35 | Validación de parámetros DLI, fotoperiodo, rampas |
| **SensorDriver** | 38 | Abstracción de sensor PPFD con calibración |
| **OutputDriver** | 35 | Control de canales LED multicanal |
| **CalibrationManager** | 30 | Gestión de calibración con offset/gain |
| **ControlUnit** | 35 | Control proporcional en lazo cerrado |
| **SystemManager** | 32 | Orquestación y máquina de estados |
| **TOTAL** | **205** | **Cobertura completa del sistema** |

### Arquitectura en Capas

El sistema sigue una arquitectura limpia de 4 capas:

```
┌─────────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                        │
│                                                             │
│  ┌────────────────────────────────────────────────────────┐ │
│  │              SystemManager (Fase 4) ✅                 │ │
│  │  - Máquina de estados: IDLE, MANUAL, AUTO, ERROR       │ │
│  │  - Coordinación de subsistemas                         │ │
│  │  - Run() loop principal                                │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                    INTERFACE LAYER (Fase 3)                 │
│  [ UserInterface | CommunicationInterface ]  ⏳ Pendiente   │
└─────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                   BUSINESS LOGIC LAYER                      │
│                                                             │
│  ┌───────────────────┐      ┌──────────────────────────┐    │
│  │  ControlUnit ✅   │      │ CalibrationManager ✅    │    │
│  │  - Control PI     │      │ - Calibración manual     │    │
│  │  - Lazo cerrado   │      │ - Offset/Gain            │    │
│  │  - Auto/Manual    │      │ - Validación CRC         │    │
│  └───────────────────┘      └──────────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                              │
┌─────────────────────────────────────────────────────────────┐
│                   HARDWARE ABSTRACTION LAYER                │
│                                                             │
│  ┌────────────────┐  ┌───────────────┐  ┌─────────────────┐ │
│  │ ConfigValidator│  │ SensorDriver  │  │ OutputDriver    │ │
│  │ ✅ DLI/Photo   │  │ ✅ PPFD 0-4000│  │ ✅ 4 canales LED│ │
│  │ ✅ Validación  │  │ ✅ Calibración│  │ ✅ PWM/DAC      │ │
│  └────────────────┘  └───────────────┘  └─────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

**Leyenda:**
- ✅ Completado y probado
- ⏳ Planificado (Fase 3)

## Características Principales

### Control Automático (ControlUnit)
- **Control proporcional (PI)** con parámetros configurables (Kp, Ki)
- **Anti-windup** para prevenir saturación del integrador
- **Rango de setpoint**: 0-4000 µmol/m²/s
- **Frecuencia de actualización**: 1 Hz (configurable)
- **Modos operativos**: Manual, Automático, Deshabilitado

### Gestión de Calibración (CalibrationManager)
- **Calibración de dos puntos** (offset + gain)
- **Persistencia de datos** con timestamp
- **Validación CRC** para integridad de datos
- **Aplicación automática** al sensor driver

### Máquina de Estados (SystemManager)
- **Estados**: UNINITIALIZED, IDLE, MANUAL, AUTOMATIC, CALIBRATING, ERROR
- **Transiciones validadas** con reglas estrictas
- **Emergency stop** con deshabilitación inmediata de salidas
- **Recuperación desde ERROR** mediante reset

### Abstracción de Hardware
- **SensorDriver**: Interfaz genérica para sensores PPFD con filtrado
- **OutputDriver**: Control de 4 canales LED independientes (0-100%)
- **Inyección de dependencias**: Hardware interfaces para testeabilidad

## Metodología de Desarrollo: Test-Driven Development

### Ciclo TDD Aplicado

Todo el proyecto fue desarrollado siguiendo el ciclo TDD:

1. **RED (Rojo):** Escritura de test que falla
2. **GREEN (Verde):** Implementación mínima para pasar el test
3. **REFACTOR:** Mejora del código manteniendo tests verdes

### Principios Aplicados

- ✅ **Validación de tests:** Todo test observado en falla antes de implementar
- ✅ **Desarrollo incremental:** Una funcionalidad a la vez
- ✅ **Implementación mínima:** Solo código necesario para satisfacer tests
- ✅ **Asignación estática:** No se usa malloc/free (embedded-friendly)
- ✅ **Separación de concerns:** Arquitectura en capas bien definida
- ✅ **Inyección de dependencias:** Interfaces de hardware mockeables

## Entorno de Testing: Ceedling

El proyecto utiliza **Ceedling** (versión 1.0.1) que integra:

- **Unity:** Framework de assertions para C
- **CMock:** Generador de mocks
- **GCov/GCovr:** Análisis de cobertura de código

### Comandos de Ejecución

```bash
# Ejecución de todos los tests (205 tests)
ceedling test:all

# Ejecución de tests por módulo
ceedling test:config_validator
ceedling test:sensor_driver
ceedling test:output_driver
ceedling test:calibration_manager
ceedling test:control_unit
ceedling test:system_manager

# Generación de reporte de cobertura
ceedling gcov:all

# Limpieza de artefactos
ceedling clean
```

## Interfaz Pública del Sistema

### SystemManager (Orquestador Principal)

```c
// Estados del sistema
typedef enum {
    SYSTEM_STATE_UNINITIALIZED,
    SYSTEM_STATE_IDLE,
    SYSTEM_STATE_MANUAL,
    SYSTEM_STATE_AUTOMATIC,
    SYSTEM_STATE_CALIBRATING,
    SYSTEM_STATE_ERROR
} SystemState;

// API principal
SystemManagerHandle SystemManager_Create(SensorHardwareInterface *sensorHw,
                                         OutputHardwareInterface *outputHw);
void SystemManager_Destroy(SystemManagerHandle handle);
SystemError SystemManager_Init(SystemManagerHandle handle);
SystemError SystemManager_Run(SystemManagerHandle handle);
SystemError SystemManager_SetState(SystemManagerHandle handle, SystemState state);
SystemError SystemManager_EmergencyStop(SystemManagerHandle handle);
```

### ControlUnit (Control en Lazo Cerrado)

```c
// Modos de control
typedef enum {
    CONTROL_MODE_MANUAL,
    CONTROL_MODE_AUTOMATIC,
    CONTROL_MODE_DISABLED
} ControlMode;

// Parámetros PI
typedef struct {
    float Kp;            // Ganancia proporcional
    float Ki;            // Ganancia integral
    float integralLimit; // Límite anti-windup
} PIParameters;

// API de control
ControlError ControlUnit_SetMode(ControlUnitHandle handle, ControlMode mode);
ControlError ControlUnit_SetSetpoint(ControlUnitHandle handle, uint16_t ppfd_umol);
ControlError ControlUnit_SetPIParameters(ControlUnitHandle handle, PIParameters params);
ControlError ControlUnit_Update(ControlUnitHandle handle);  // Llamar a 1 Hz
```

### CalibrationManager (Gestión de Calibración)

```c
// Calibración manual de dos puntos
CalibrationError CalibrationManager_SetCalibration(CalibrationManagerHandle handle,
                                                   float offset_umol,
                                                   float gain,
                                                   uint32_t timestamp_ms);
CalibrationError CalibrationManager_GetCalibration(CalibrationManagerHandle handle,
                                                   CalibrationInfo *calibration);
CalibrationError CalibrationManager_ApplyToSensor(CalibrationManagerHandle handle);
CalibrationError CalibrationManager_IsValid(CalibrationManagerHandle handle, bool *isValid);
```

## Estructura del Repositorio

```
proyecto-final-cese/
├── inc/                           # Archivos de cabecera públicos
│   ├── config_validator.h         # Validador de configuración DLI
│   ├── sensor_driver.h            # Driver abstracto de sensor PPFD
│   ├── output_driver.h            # Driver de salidas LED multicanal
│   ├── calibration_manager.h     # Gestor de calibración
│   ├── control_unit.h            # Unidad de control PI
│   └── system_manager.h          # Orquestador del sistema
├── src/                          # Código fuente de implementación
│   ├── config_validator.c
│   ├── sensor_driver.c
│   ├── output_driver.c
│   ├── calibration_manager.c
│   ├── control_unit.c
│   └── system_manager.c
├── test/                         # Suite de tests unitarios (205 tests)
│   ├── test_config_validator.c   # 35 tests
│   ├── test_sensor_driver.c      # 38 tests
│   ├── test_output_driver.c      # 35 tests
│   ├── test_calibration_manager.c # 30 tests
│   ├── test_control_unit.c       # 35 tests
│   ├── test_system_manager.c     # 32 tests
│   └── support/                  # Código auxiliar para testing
├── docs/                         # Documentación del proyecto
│   └── ARCHITECTURE_LAYERS.md    # Descripción de arquitectura
├── build/                        # Artefactos de compilación (ignorado)
├── project.yml                   # Configuración de Ceedling
└── README.md                     # Presente documento
```

## Próximos Pasos

### Fase 3: Interface Layer (Planificado)
- **UserInterface**: CLI por UART para configuración manual
- **CommunicationInterface**: Protocolo Puerto G para telemetría remota

### Integración con Hardware STM32F446RE
- Implementación de hardware interfaces reales
- Testing en plataforma objetivo
- Ajuste de parámetros PI según respuesta del sistema
- Medición de rendimiento en tiempo real

### Características Avanzadas
- Control de distribución espectral (ratio R:B:FR)
- Calibración automática con lámpara de referencia
- Detección de drift del sensor
- Scheduling de perfiles de luz diarios

## Stack Tecnológico

- **Lenguaje:** C (estándar C99)
- **Target:** STM32F446RE (ARM Cortex-M4)
- **Framework de Testing:** Ceedling 1.0.1
- **Framework de Assertions:** Unity
- **Framework de Mocking:** CMock
- **Compilador:** GCC con flags estrictos (-Wall -Wextra -Werror)
- **Análisis de Cobertura:** GCov + GCovr
- **Control de Versiones:** Git
- **Formato de Código:** clang-format
- **Hooks de Pre-commit:** pre-commit framework
- **CI/CD:** GitHub Actions (tests automáticos en cada push)

## Referencias Bibliográficas

- Grenning, J. W. (2011). *Test-Driven Development for Embedded C*. Pragmatic Bookshelf.
- Documentación oficial de Ceedling. ThrowTheSwitch.org. http://throwtheswitch.org/
- Documentación de Unity Test Framework. https://github.com/ThrowTheSwitch/Unity
- STM32F446RE Reference Manual. STMicroelectronics.

## Autor

**Fernando Folmer**
Estudiante de la Especialización en Sistemas Embebidos (CESE)
Facultad de Ingeniería - Universidad de Buenos Aires (FIUBA)

## Licencia

Proyecto académico desarrollado en el marco del trabajo final de la Especialización en Sistemas Embebidos.
