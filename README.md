# Validador de Configuracion para Control de Iluminacion basado en DLI

## Descripcion General del Proyecto

Este proyecto forma parte del trabajo final de la Especializacion en Sistemas Embebidos (CESE) de la FIUBA. El modulo presentado ha sido desarrollado empleando la metodologia **Test-Driven Development (TDD)**, siguiendo los lineamientos establecidos por James Grenning en su obra "Test-Driven Development for Embedded C".

El modulo implementado es un validador de configuracion para un controlador multicanal de PPFD (Photosynthetic Photon Flux Density) destinado a aplicaciones de cultivo en ambientes controlados. El sistema permite configurar la iluminacion mediante un objetivo de DLI (Daily Light Integral), validando que los parametros ingresados sean alcanzables dentro de las restricciones fisicas del hardware.

## Caso de Uso Principal: Configuracion de Iluminacion por DLI

El sistema implementa el siguiente flujo de interaccion con el usuario:

### Flujo Normal

1. El sistema solicita el valor objetivo de DLI (mol/m^2/dia)
2. El usuario ingresa el valor numerico deseado
3. El sistema solicita la duracion del fotoperiodo (horas de luz por dia)
4. El usuario especifica la cantidad de horas (valor entero)
5. El sistema solicita la configuracion de rampas de encendido y apagado
6. El usuario define los tiempos de rampa en minutos
7. El sistema valida que el DLI objetivo sea alcanzable con los parametros ingresados
8. Si la validacion es exitosa, el sistema calcula y muestra las intensidades luminicas resultantes
9. El usuario confirma la configuracion
10. El sistema persiste los parametros validados

### Flujo Alternativo

Cuando el sistema determina que el DLI objetivo no es alcanzable con los parametros ingresados y las restricciones del hardware, ofrece las siguientes alternativas:

- Sugerir un valor de DLI alcanzable manteniendo los demas parametros
- Proponer un incremento en el fotoperiodo que permita alcanzar el DLI deseado
- Recomendar la reduccion de los tiempos de rampa para maximizar el tiempo efectivo de iluminacion

## Restricciones del Sistema

### Sensor de PPFD
- **Rango de medicion:** 0 - 2000 umol/m^2/s
- El limite superior define la intensidad luminica maxima controlable por el sistema

### Fotoperiodo
- **Valor minimo:** 1 hora
- **Tipo de dato:** Entero sin signo de 8 bits (uint8_t)
- No se admiten valores fraccionarios

### Tiempos de Rampa (Rise/Fall Time)
- **Rango valido:** 30 - 90 minutos
- **Justificacion:** Limitar la corriente de arranque (*inrush current*) en el encendido/apagado del sistema de iluminacion
- Valores inferiores podrian generar transitorios electricos no deseados

### DLI (Daily Light Integral)
- **Tipo de dato:** Entero sin signo de 16 bits (uint16_t)
- **Unidades:** mol/m^2/dia
- **Validacion:** Debe ser alcanzable considerando el fotoperiodo, tiempos de rampa y limite superior del sensor PPFD

## Metodologia de Desarrollo: Test-Driven Development

El desarrollo del modulo se realiza siguiendo estrictamente la metodologia TDD, tal como se describe en la bibliografia de referencia (Grenning, 2011).

### Ciclo TDD

El proceso iterativo consta de tres fases:

1. **RED (Rojo):** Escritura de un test que falla, definiendo el comportamiento esperado
2. **GREEN (Verde):** Implementacion del codigo minimo necesario para que el test pase
3. **REFACTOR (Refactorizacion):** Mejora del codigo manteniendo la funcionalidad validada por los tests

### Principios Aplicados

- **Validacion de tests:** Todo test debe ser observado en estado de falla antes de implementar la funcionalidad correspondiente
- **Desarrollo incremental:** Se implementa una funcionalidad a la vez, guiada por los tests
- **Implementacion minima:** Se escribe unicamente el codigo necesario para satisfacer el test actual
- **Asignacion estatica de memoria:** Apropiada para sistemas embebidos, evitando el uso de asignacion dinamica

## Entorno de Testing: Ceedling

El proyecto utiliza el framework **Ceedling** (version 1.0.1), que integra las siguientes herramientas:

- **Unity:** Framework de assertions para C
- **CMock:** Generador de mocks para interfaces en C
- **GCov/GCovr:** Herramientas de analisis de cobertura de codigo

### Comandos de Ejecucion

```bash
# Ejecucion de todos los tests
ceedling test:all

# Ejecucion de tests del modulo config_validator
ceedling test:config_validator

# Generacion de reporte de cobertura de codigo
ceedling gcov:all
```

## Estado Actual del Desarrollo

### Tests Implementados: 4 de 33 planificados

**Fase 1 - Ciclo de vida del modulo:**
- [x] `test_ConfigValidator_CanBeCreated` - Validacion de creacion de instancia

**Fase 2 - Estado inicial:**
- [x] `test_ConfigValidator_HasKnownInitialState` - Verificacion del estado inicial (DLI = 0)

**Fase 3 - Operaciones basicas de parametros:**
- [x] `test_ConfigValidator_AcceptsValidDLI` - Validacion de asignacion de DLI valido
- [x] `test_ConfigValidator_AcceptsValidPhotoperiod` - Validacion de asignacion de fotoperiodo valido

### Funcionalidades Pendientes

Las siguientes funcionalidades seran implementadas siguiendo la misma metodologia TDD:

- Asignacion y validacion de tiempos de rampa (rise/fall time)
- Rechazo de parametros invalidos (valores negativos, fuera de rango)
- Validacion de coherencia entre parametros (rampas vs. fotoperiodo)
- Calculo de alcanzabilidad del DLI objetivo
- Calculo de intensidad luminica maxima requerida
- Generacion de sugerencias para configuraciones invalidas

## Interfaz Publica del Modulo (API)

### Tipos de Datos

```c
// Handle opaco para instancia del validador
typedef struct ConfigValidator * ConfigValidatorHandle;

// Codigos de error retornados por las operaciones
typedef enum {
    CONFIG_OK = 0
} ConfigValidatorError;
```

### Funciones Implementadas

#### Gestion del ciclo de vida
```c
ConfigValidatorHandle ConfigValidator_Create(void);
void ConfigValidator_Destroy(ConfigValidatorHandle handle);
```

#### Configuracion de DLI
```c
ConfigValidatorError ConfigValidator_SetDLI(ConfigValidatorHandle handle, uint16_t dli);
ConfigValidatorError ConfigValidator_GetDLI(ConfigValidatorHandle handle, uint16_t * dli);
```

#### Configuracion de Fotoperiodo
```c
ConfigValidatorError ConfigValidator_SetPhotoperiod(ConfigValidatorHandle handle, uint8_t photoperiod);
ConfigValidatorError ConfigValidator_GetPhotoperiod(ConfigValidatorHandle handle, uint8_t * photoperiod);
```

### Funciones Planificadas (Pendientes de Implementacion)

- `ConfigValidator_SetRiseTime()` / `ConfigValidator_GetRiseTime()`
- `ConfigValidator_SetFallTime()` / `ConfigValidator_GetFallTime()`
- `ConfigValidator_ValidateConfiguration()`
- `ConfigValidator_GetMaxIntensity()`
- `ConfigValidator_GetSuggestedDLI()`
- `ConfigValidator_GetSuggestedPhotoperiod()`
- `ConfigValidator_GetSuggestedRampTimes()`

## Estructura del Repositorio

```
proyecto-final-cese/
├── inc/                        # Archivos de cabecera publicos
│   └── config_validator.h      # Interfaz del modulo validador
├── src/                        # Codigo fuente de implementacion
│   └── config_validator.c      # Implementacion del validador
├── test/                       # Suite de tests unitarios
│   ├── test_config_validator.c # Tests del modulo validador
│   └── support/                # Codigo auxiliar para testing
├── build/                      # Artefactos de compilacion (ignorado por git)
├── project.yml                 # Configuracion de Ceedling
├── makefile                    # Configuracion de compilacion alternativa
└── README.md                   # Presente documento
```

## Compilacion y Ejecucion

### Entorno de Testing (Ceedling)
```bash
# Compilacion y ejecucion de tests
ceedling test:all

# Limpieza de artefactos
ceedling clean
```

### Compilacion para Produccion (Makefile)
```bash
# Compilacion del proyecto
make all

# Limpieza de objetos y ejecutables
make clean
```

## Stack Tecnologico

- **Lenguaje:** C (estandar C99)
- **Framework de Testing:** Ceedling 1.0.1
- **Framework de Assertions:** Unity
- **Framework de Mocking:** CMock
- **Compilador:** GCC con flags de advertencia habilitados (-Wall -Wextra -Werror -pedantic)
- **Analisis de Cobertura:** GCov + GCovr
- **Control de Versiones:** Git
- **Formato de Codigo:** clang-format
- **Hooks de Pre-commit:** pre-commit framework

## Referencias Bibliograficas

- Grenning, J. W. (2011). *Test-Driven Development for Embedded C*. Pragmatic Bookshelf.
- Documentacion oficial de Ceedling. ThrowTheSwitch.org. Disponible en: http://throwtheswitch.org/
- Documentacion de Unity Test Framework. GitHub. Disponible en: https://github.com/ThrowTheSwitch/Unity

## Autor

**Fernando Folmer**
Estudiante de la Especializacion en Sistemas Embebidos (CESE)
Facultad de Ingenieria - Universidad de Buenos Aires (FIUBA)

## Licencia

Proyecto academico desarrollado en el marco del trabajo final de la Especializacion en Sistemas Embebidos.
