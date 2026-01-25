# EI_03: Simón Dice - Lógica Algorítmica y Gestión de Memoria 🎮

Este proyecto integrador del **Nivel Básico** representa un desafío de alta complejidad lógica. Aquí se integran el manejo de arreglos estáticos, la generación de números pseudoaleatorios y la gestión de periféricos de entrada/salida (I/O) en tiempo real sobre la placa **Nucleo-F439ZI**.

## 📍 Objetivos del Proyecto
- Implementar un motor de juego basado en **Arreglos Estáticos** y comparación de índices en memoria.
- Gestionar la aleatoriedad mediante la función `rand()` y el uso del reloj de sistema (`HAL_GetTick()`) como semilla dinámica para garantizar partidas únicas.
- Diseñar una **Interfaz Hombre-Máquina (HMI)** completa con 4 canales de entrada (Pulsadores) y 4 de salida (LEDs) sincronizados.
- Programar rutinas de **Feedback Visual** diferenciadas para los estados de *Éxito*, *Error* y *Victoria*.

---

## 🧠 Algoritmo de Control (Main Loop)

El sistema opera bajo un ciclo cerrado de cuatro fases críticas que aseguran la sincronización exacta entre el hardware y el usuario:



1. **Generación:** Se utiliza `rand() % 4 + 1` para añadir un nuevo paso a la secuencia. La semilla se inicializa con `srand(HAL_GetTick())` al momento del arranque.
2. **Reproducción:** El MCU recorre el arreglo `secuencia[]` y utiliza la función `Simon_EncenderUno()` para ejecutar la coreografía de luces.
3. **Validación:** El sistema entra en una **espera activa**. Se utiliza un **Antirrepetidor** (`while(Simon_CualquierBotonPresionado())`) para garantizar que el usuario deba soltar el botón antes de validar el siguiente paso.
4. **Condición de Victoria:** Al alcanzar el `NIVEL_VICTORIA` (definido en 10), se dispara una animación especial de luces (`Win_Animation`) antes de reiniciar el progreso.

---

## 🛠️ Desafíos de Ingeniería Superados

### 1. Detección de Entrada y Debounce
Se configuraron 4 pines en modo `INPUT` con **Pull-up interna**. La detección se realiza por nivel bajo (`GPIO_PIN_RESET`). Se incluyó un `HAL_Delay(10)` dentro del bucle de espera para reducir el consumo de recursos del CPU y un delay de `50ms` post-pulsación para absorber el rebote mecánico.

### 2. Telemetría por UART
El sistema utiliza la `USART3` a **115200 bps** para informar el estado del juego en tiempo real:
- Nivel actual alcanzado.
- Botón detectado por el microcontrolador.
- Notificación de *Game Over* o *Victoria* con mensajes ASCII.

### 3. Animaciones de Estado
Las animaciones de victoria y error manipulan directamente los registros de los puertos `GPIOE` y `GPIOB`, demostrando un control preciso de los periféricos sin interferir con la lógica de memoria.

```c
/* Ejemplo de validación en tiempo real con Antirrepetidor */
uint8_t botonUser = Simon_EsperarBoton(); // Bloqueante hasta detectar presión
Simon_EncenderUno(botonUser);             // Feedback visual inmediato
while(Simon_CualquierBotonPresionado());   // Bloqueo de liberación (Obliga a soltar el botón)
```
## 🚀 Hacia la Integración Maestro: Proyecto 04

Este proyecto consolida el *dominio* de la **Lógica Programable.** Hemos agotado las posibilidades del código secuencial y estamos listos para el último paso del nivel: la Integración de APIs, donde uniremos todos estos conceptos bajo una arquitectura modular de drivers antes de saltar al mundo de las Interrupciones.

---
*En los sistemas embebidos, la elegancia del código se mide por su capacidad de responder con precisión a los tiempos y ruidos del mundo físico.*