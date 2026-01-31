# 📘 Guía de Supervivencia: STM32CubeIDE & HAL Layer

Este directorio contiene la base teórica y técnica para el desarrollo de firmware sobre microcontroladores **STM32**. Entender estas particularidades es clave para evitar errores comunes y escribir código profesional.

---

## 1. El Flujo de Trabajo (Workflow)
El desarrollo en STM32 es un ciclo entre la configuración gráfica y la implementación de lógica:

1.  **Configuración (.ioc):** Uso de STM32CubeMX para asignar funciones a los pines y configurar el reloj del sistema (**Clock Tree**).
2.  **Generación de Código:** El IDE traduce la configuración en archivos `.c` y `.h`.
3.  **Implementación de Usuario:** El código DEBE escribirse entre las etiquetas `/* USER CODE BEGIN */` y `/* USER CODE END */`.
    * *Nota:* Si escribes fuera de estas marcas, el IDE borrará tu código al regenerar el proyecto.



---

## 2. Diccionario de Funciones Esenciales (GPIO & Tiempo)
Estas son las funciones de la librería **HAL** que más utilizamos en el Nivel Básico.

### ⚡ HAL_GPIO_WritePin
Escribe un estado lógico (0 o 1) en un pin de salida.
- **Sintaxis:** `HAL_GPIO_WritePin(Puerto, Pin, Estado);`
- **Ejemplo:** `HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET); // Enciende LED`

### 🔍 HAL_GPIO_ReadPin
Lee el estado actual de un pin configurado como entrada.
- **Sintaxis:** `HAL_GPIO_ReadPin(Puerto, Pin);`
- **Uso:** `if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) { // Botón presionado }`

### ⏳ HAL_Delay
Genera una pausa en la ejecución del programa.
- **Sintaxis:** `HAL_Delay(milisegundos);`
- **Nota:** Es una función **bloqueante**; el microcontrolador no ejecutará otras tareas (excepto interrupciones) mientras espera.



---

## 3. Tipos de Datos Estándar (stdint.h)
Para asegurar que el código sea portátil entre diferentes micros ARM, usamos tipos de longitud fija:

| Tipo | Tamaño | Rango | Uso Común |
| :--- | :--- | :--- | :--- |
| **`uint8_t`** | 8 bits | 0 a 255 | Banderas, buffers UART, estados de juego. |
| **`uint16_t`** | 16 bits | 0 a 65,535 | Valores de ADC, parámetros de PWM. |
| **`uint32_t`** | 32 bits | 0 a 4,294,967,295 | Tiempos de sistema (`HAL_GetTick`). |

---

*Documentación creada por Carlitos MF - Tucumán, Argentina.*