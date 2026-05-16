# 06. 📄 Anatomía del main.c y Secciones de Usuario

El archivo `main.c` generado automáticamente por el **STM32CubeIDE** tras guardar el `.ioc` es el punto de entrada y el orquestador principal de nuestra aplicación. En esta guía aprenderemos a navegar su estructura interna y, lo más importante, las reglas estrictas que debemos seguir para integrar nuestra lógica sin que el generador de código del fabricante la destruya.

---

## 1. 🏗️ La Estructura de "User Code" (Zonas Seguras)


### 🗂️ Encabezado del Archivo e Inclusiones (Header & Includes)

Al abrir el archivo `main.c`, la sección inicial define los comentarios de propiedad, licencias del ecosistema de ST y, fundamentalmente, la primera zona segura de acoplamiento para nuestras bibliotecas de **Capa 2**.

```c
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
```

* **`USER CODE BEGIN Header`**: Bloque reservado para la documentación personalizada del desarrollador (Nombre del proyecto, descripción de la arquitectura, autor y control de versiones). Todo el texto introducido aquí es ignorado por el compilador y blindado ante el generador del `.ioc`.

### 📦 El Bloque de Inclusión de Librerías (Inclusiones Privadas)
Inmediatamente debajo, se encuentra el punto de anclaje más crítico para conectar la lógica de nuestra aplicación con los periféricos abstractos:

```c
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */
```

* **`#include "main.h"`**: Cabecera maestra generada por el entorno. Su función principal es importar las definiciones nativas de la **HAL de ST** (`stm32f4xx_hal.h`) y centralizar los mapeos macro de las **User Labels** que configuramos de forma gráfica en el configurador.
* **`USER CODE BEGIN Includes`**: **Punto de conexión de nuestra Capa 2.** Cuando desarrollemos nuestros módulos independientes y soberanos (como `user_gpio.h`, `user_adc.h`, o `user_usart.h`), las directivas de inclusión deben declararse estrictamente dentro de este bloque para evitar que el IDE las remueva al recompilar el `.ioc`.


### 🧩 Secciones de Definiciones Privadas (Tipos, Macros y Constantes)

Inmediatamente después de las inclusiones de librerías, el archivo despliega un conjunto de bloques seguros destinados a la configuración interna y personalización del firmware a nivel de preprocesador:

```c
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */
```

* **`USER CODE BEGIN PTD` (Private Typedef)**: Espacio reservado para la declaración de tipos de datos personalizados del desarrollador, tales como `struct`, `union` o definiciones de enumeraciones (`enum`) específicas que requiera este módulo.
* **`USER CODE BEGIN PD` (Private Define)**: Zona segura para la declaración de constantes numéricas, máscaras de bits fijas o identificadores del preprocesador mediante directivas `#define`.
* **`USER CODE BEGIN PM` (Private Macro)**: Bloque destinado a macros de preprocesador con argumentos o funciones inline (ej. `#define MIN(a,b) ((a)<(b)?(a):(b))`) que optimicen cálculos repetitivos o mapeos de bajo nivel sin generar sobrecarga en la pila de ejecución.

### 📊 Variables Privadas y Prototipos de Función (PV & PFP)

A continuación, el archivo maneja las instancias de los periféricos generados por la HAL y las declaraciones previas de las funciones de inicialización:

```c
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */
```

* **`UART_HandleTypeDef huart3`**: Estructura de control (Handler) nativa de la HAL queega de gestionar el estado, la configuración y los buffers del periférico **USART3** (conectado al Virtual COM Port). Al estar fuera de una sección de usuario, es manejada y reescrita de forma automática por el IDE.
* **`USER CODE BEGIN PV` (Private Variables)**: Zona segura designada para declarar nuestras variables globales del módulo. Es el lugar ideal para definir, por ejemplo, las estructuras de nuestras máquinas de estado (FSM) de la **Capa 3** o contadores globales de control.
* **Prototipos de Inicialización (`MX_...`)**: Declaraciones previas de las funciones que configuran el reloj del sistema y los periféricos activados en el `.ioc`. Al ser de tipo `static`, su alcance queda restringido estrictamente a este archivo `main.c`.

### 🚀 El Punto de Entrada de la Aplicación (La Función main)

La función `main(void)` es donde se ejecuta la inicialización obligatoria de la arquitectura y la posterior orquestación de nuestras capas de software:

```c
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
```

* **`USER CODE BEGIN 0`**: Bloque exterior previo al `main()`. Se utiliza comúnmente para declarar variables globales estáticas, funciones de callback o rutinas de interrupción (ISR) personalizadas si no se quieren separar en módulos externos.
* **`USER CODE BEGIN 1`**: Primera sección dentro del `main()`. Es el espacio ideal para definir variables locales de la función principal, estructuras de configuración locales o variables de entorno iniciales antes de alterar los registros del MCU.
* **`HAL_Init()`**: Función crítica de la HAL de ST. Realiza el reset de todos los periféricos a su estado inicial, configura la memoria Flash (Prefetch, Data/Instruction cache) y establece el temporizador del **SysTick** (normalmente configurado para interrumpir cada $1\text{ ms}$) como la base de tiempo global del sistema.
* **`SystemClock_Config()`**: Configura los multiplexores, prescaladores y el bucle de enganche de fase (Main PLL) a partir de la entrada del cristal externo (HSE) para establecer la velocidad del **SYSCLK en 180 MHz** de forma estable.
* **Funciones de Inicialización (`MX_..._Init`)**: Bloque donde se invocan las configuraciones iniciales de los periféricos seleccionados. En este caso, establece los modos eléctricos de los GPIOs y los parámetros de comunicación ($115200\text{, 8, N, 1}$) de la **USART3** mediante el hardware mapping automático de la HAL.
* **`USER CODE BEGIN 2`**: **La zona de inicialización de la Capa 2.** Esta sección se ejecuta una sola vez tras estabilizarse todo el silicio del microcontrolador. Aquí invocamos las funciones de inicialización de nuestros drivers soberanos (ej. `User_GPIO_Init();`, `User_LCD_Init();`).
* **`USER CODE BEGIN WHILE` y el bucle `while(1)`**: El motor de ejecución continua en tiempo real. Dentro del bloque `while` colocamos de manera estricta la llamada a nuestra **Capa 3 (Aplicación / FSM)** para que procese de forma cíclica y no bloqueante las tareas de control.

### 🔧 Desglose de Funciones de Inicialización de Periféricos (Generadas por CubeMX)

Al final del archivo se implementan las funciones encargadas de configurar los registros del microcontrolador basándose en las estructuras de inicialización de la HAL. Analizarlas nos permite entender qué bits se están modificando tras bambalinas.

```c
void SystemClock_Config(void);
static void MX_USART3_UART_Init(void);
static void MX_GPIO_Init(void);
```

* ### 1. ⏱️ `SystemClock_Config(void)`
  Es la función encargada de inicializar el oscilador y configurar el árbol de relojes. Utiliza dos estructuras principales: `RCC_OscInitTypeDef` (para los osciladores) y `RCC_ClkInitTypeDef` (para los buses de datos).
  * **Habilitación de Energía (`__HAL_RCC_PWR_CLK_ENABLE`)**: Activa el reloj del periférico de control de energía (Power Control - PWR) para permitir configuraciones avanzadas de voltaje.
  * **Voltage Scaling (`PWR_REGULATOR_VOLTAGE_SCALE1`)**: Configura el regulador de voltaje interno en Modo 1 (el de máximo rendimiento). Esto es un requisito obligatorio del **RM0090** para poder operar a frecuencias superiores a los $144\text{ MHz}$.
  * **Parámetros del Main PLL**: Configura el bucle de enganche de fase tomando como fuente el oscilador externo **HSE de 8 MHz** provisto por el ST-LINK:
    * $M = 4$: Divide la entrada a $2\text{ MHz}$ ($\frac{8\text{ MHz}}{4}$).
    * $N = 180$: Multiplica el valor en el VCO a $360\text{ MHz}$ ($2\text{ MHz} \times 180$).
    * $P = 2$: Divide la salida del VCO para fijar el **SYSCLK en 180 MHz** ($\frac{360\text{ MHz}}{2}$).
    * $Q = 7$: Divide la salida para el reloj de periféricos específicos (USB, SDIO) a aproximadamente $51.4\text{ MHz}$.
  * **Modo Over-Drive (`HAL_PWREx_EnableOverDrive`)**: Activa el modo de sobre-alimentación interna, indispensable para alcanzar los 180 MHz de forma estable en el silicio sin causar jitter.
  * **Divisores de Bus (AHB / APB)**: Establece las frecuencias de los buses de comunicación internos mediante prescaladores:
    * **AHB Prescaler (`DIV1`)**: Mantiene el reloj del núcleo (**HCLK**) a 180 MHz.
    * **APB1 Prescaler (`DIV4`)**: Divide la frecuencia por 4 para el bus periférico lento, fijando **PCLK1 en 45 MHz** (límite máximo de hardware).
    * **APB2 Prescaler (`DIV2`)**: Divide la frecuencia por 2 para el bus periférico rápido, fijando **PCLK2 en 90 MHz** (límite máximo de hardware).
  * **Flash Latency (`FLASH_LATENCY_5`)**: Modifica los estados de espera (Wait States) del controlador de la memoria Flash. Dado que el núcleo corre a $180\text{ MHz}$ y la memoria Flash física no puede responder a esa velocidad, se configuran 5 ciclos de espera junto con el **ART Accelerator** para evitar cuellos de botella en la lectura de instrucciones.

* ### 2. 🖨️ `MX_USART3_UART_Init(void)`
  Configura la interfaz serie mapeada al Virtual COM Port utilizando la estructura `UART_HandleTypeDef`.
  * **Instancia de Registro (`USART3`)**: Vincula el puntero base del mapa de memoria de la USART3 con el handler de control de la HAL.
  * **Configuración del Frame**: Establece de forma directa los parámetros estándar de telemetría: un baudrate de **115200 bps**, longitud de palabra de **8 bits**, **1 bit de parada** y **sin paridad** (`UART_PARITY_NONE`).
  * **Oversampling 16**: Configura el receptor para tomar 16 muestras por cada bit recibido, aumentando la robustez contra el ruido electromagnético en la línea de transmisión.
  * **Zonas Seguras integradas**: Posee sus propios bloques internos (`/* USER CODE BEGIN USART3_Init X */`) que permiten, por ejemplo, configurar interrupciones de usuario o activar transferencias por DMA antes o después de disparar la función constructora `HAL_UART_Init`.

* ### 3. 🚦 `MX_GPIO_Init(void)`
  Configura los registros de control eléctrico de los pines utilizando la estructura `GPIO_InitTypeDef`.
  * **Habilitación de Relojes de Puertos (`__HAL_RCC_GPIOx_CLK_ENABLE`)**: Activa las señales de reloj en el bus AHB1 para los puertos $A, B, C, D, H$ y $G$. En la arquitectura STM32, cualquier intento de escribir en los registros de un periférico sin haber habilitado su reloj previamente es ignorado por completo por el hardware.
  * **Configuración de Salidas (LEDs de Usuario)**:
    * Fija un estado inicial en bajo (`GPIO_PIN_RESET`) mediante `HAL_GPIO_WritePin` para asegurar que `LD1`, `LD2`, y `LD3` arranquen apagados.
    * Configura el modo en **Push-Pull** (`GPIO_MODE_OUTPUT_PP`) sin resistencias de pull-up ni pull-down (`GPIO_NOPULL`).
    * Establece la velocidad del flanco en **Low** (`GPIO_SPEED_FREQ_LOW`) para minimizar las corrientes parásitas y la radiación de ruido de alta frecuencia en la placa.
  * **Configuración del Botón de Usuario (`USR_BTN_Pin`)**:
    * Configura el pin `PC13` en modo de interrupción externa con flanco de subida (`GPIO_MODE_IT_RISING`). Esto mapea automáticamente la línea al controlador **EXTI** (External Interrupt Controller), permitiendo disparar subrutinas de código al presionar el pulsador.
  * **Zonas Seguras finales**: El bloque final `/* USER CODE BEGIN MX_GPIO_Init_2 */` es sumamente útil si se desea reconfigurar algún registro de GPIO de forma directa mediante máscaras de bits o asignaciones de registros de nuestra **Capa 1** (como el registro `MODER` u `ODR`) justo al terminar la inicialización estándar.

### 📡 Rutinas Externas y Funciones de Callback (USER CODE 4)

Ubicado al final de la estructura principal, justo antes de las funciones de gestión de errores del sistema, se encuentra el último bloque seguro para el desarrollador:

```c
/* USER CODE BEGIN 4 */

/* USER CODE END 4 */
```
* **`USER CODE BEGIN 4`**: Es la zona designada por excelencia para alojar las **funciones de callback de las interrupciones** de la HAL (ej. `HAL_GPIO_EXTI_Callback()` para los flancos del botón, o `HAL_UART_RxCpltCallback()` para la recepción de datos por puerto serie). Al colocar estas funciones aquí, interceptamos los eventos del hardware mapeado por ST y podemos derivar las banderas (*flags*) hacia nuestra **Capa 2** y **Capa 3** de forma limpia y ordenada.

### 🛡️ Gestión de Errores Críticos y Aserciones (Error_Handler & Assert)

Para finalizar la estructura del archivo, el entorno genera las rutinas de seguridad destinadas a atrapar fallos catastróficos en la inicialización del hardware o parámetros fuera de rango:

```c
void Error_Handler(void);
void assert_failed(uint8_t *file, uint32_t line);
```

* **`Error_Handler(void)`**: Es la función de contención a la que se deriva el flujo si cualquier inicialización crítica de la HAL (`HAL_Init`, `SystemClock_Config` o los periféricos) falla y no devuelve un estado operativo (`HAL_OK`).
  * **Mecanismo de Bloqueo**: Ejecuta de forma nativa la macro `__disable_irq()` para deshabilitar todas las interrupciones del procesador Cortex-M4 y entra en un bucle infinito `while(1)`. Esto previene que el firmware continúe ejecutándose en un estado inestable o impredecible que ponga en riesgo la integridad eléctrica del hardware.
  * **`USER CODE BEGIN Error_Handler_Debug`**: Zona segura clave para agregar telemetría de emergencia. Aquí podés escribir código para encender un LED de alarma (como el LED rojo de la Nucleo), realizar un parpadeo de patrón binario de diagnóstico, o forzar un volcado de registros antes del cuelgue del sistema.

* **`assert_failed(uint8_t *file, uint32_t line)`**: Función de depuración condicional que solo se compila si la macro `USE_FULL_ASSERT` está habilitada en la configuración del proyecto.
  * **Validación de Parámetros**: Se dispara cuando una función interna de la HAL recibe un argumento inválido (por ejemplo, intentar configurar un número de pin que no existe en el puerto seleccionado).
  * Recibe como parámetros el puntero al archivo de origen (`*file`) y el número exacto de línea (`line`) donde ocurrió la aserción.
  * **`USER CODE BEGIN 6`**: Bloque seguro ideal para conectar un `printf` hacia la **USART3**, permitiendo imprimir en la terminal de la PC el archivo y la línea exacta del error durante la etapa de debug con GDB, acelerando drásticamente la localización de fallos en el código.


## 🧠 Convivencia de la Arquitectura de 3 Capas
Para mantener la portabilidad y evitar que el software quede acoplado en exceso al ecosistema de ST, el `main.c` debe funcionar únicamente como un **orquestador de alto nivel**:

* **Capa 1 (Hardware Mapping)**: No se toca en este archivo; vive en sus propias estructuras de registros independientes de bajo nivel.
* **Capa 2 (Drivers Propios)**: Se inicializan dentro de `USER CODE BEGIN 2` y exponen una API limpia que oculta la complejidad del silicio.
* **Capa 3 (Aplicación)**: Toma el control completo dentro del bucle `while (1)`. La lógica de la máquina de estados consume las funciones abstractas de la Capa 2 y desconoce por completo que corre sobre un hardware STM32.

> [!NOTE]
> Aunque el generador inserte llamadas como `MX_GPIO_Init()`, nosotros podemos usar posteriormente nuestras funciones de Capa 2 para reconfigurar registros de forma directa o aplicar técnicas de bit-shifting atómicas.

---

## 🛡️ Buenas Prácticas de Codificación
1. **Bloqueo Cero**: Evitar terminantemente el uso de `HAL_Delay()` dentro del bucle principal, ya que congela la CPU e impide el determinismo y la velocidad de reacción de la FSM. En su lugar, utilizaremos banderas de timers o contadores no bloqueantes.
2. **Abstracción de Pines**: Nunca uses números de pines directamente (ej. `0x0001`). Utiliza siempre las macros generadas por las etiquetas que configuramos en el IOC (ej. `LED_VERDE_Pin`).
3. **Higiene del main**: Si una lógica de control de la FSM supera las pocas líneas, debe ser modularizada en un archivo `.c` separado perteneciente a la Capa 3, manteniendo el `main.c` lo más limpio y legible posible.

---

## 🏁 Conclusión
El archivo `main.c` generado por el entorno es un excelente punto de partida para estabilizar la infraestructura de hardware de nuestro **STM32F439ZI** a 180 MHz sin perder tiempo en la configuración inicial del árbol de relojes.

Sin embargo, el verdadero valor de nuestra ingeniería radica en **saber trazar la línea divisoria**: entender que las funciones `MX_..._Init` y la HAL son solo herramientas de soporte para la infraestructura básica. Al dominar la ubicación y el propósito de cada bloque `USER CODE`, garantizamos que nuestra **Arquitectura de 3 Capas** mantenga la soberanía del código, la modularidad y una portabilidad real hacia cualquier otro ecosistema bare-metal o futuras placas de laboratorio.

---

## 🧭 Mapa de Ruta
El entorno ya está configurado y el código base generado de forma segura. A partir de la siguiente guía, abandonamos las herramientas gráficas del fabricante para empezar el desarrollo puro de software desde cero:

1. **Guía 07:** Implementación de la Capa 1 y Mapeo de Registros en C (GPIO).

---
**Siguiente paso:** [07_Hardware_Mapping](../07_Hardware_Mapping/README.md)

🛠️ **Carlos** | Estudiante de Ing. Electrónica @UTN_FRT.  
🚀 Apasionado Autodidacta por los Sistemas Embebidos.