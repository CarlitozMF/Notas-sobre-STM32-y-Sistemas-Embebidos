# 01_EXTI_Pulsadores: Contador de 3 Dígitos con Control EXTI 📑

Este proyecto implementa un contador digital de 0 a 999 sobre tres displays de 7 segmentos multiplexados, **integrando interrupciones externas** para el control de flujo.

# 🚀 Características

- Multiplexación por División de Tiempo: Control de 3 dígitos con bus compartido.
- Tiempo No Bloqueante: Uso de HAL_GetTick() para el avance del contador.
- Control por Interrupciones (EXTI):
    1. Botón Reset: Reinicio instantáneo a 000.
    2. Botón Pause/Play: Congela/Reanuda el conteo.
    3. Filtrado de Hardware: Uso de capacitores de 100nF para eliminar el bounce mecánico.

# 🛠️ Configuración de Hardware

- Displays: 3x 7-Segmentos (Cátodo Común).
- Pines de Datos: SEG_A a SEG_G (GPIO Output).
- Pines de Control: EN1, EN2, EN3 (Transistores NPN).
- Botones Externos: Conectados a GND con configuración Pull-Up interna.
    1. BTN_RESET (PB10) - Falling Edge.
    2. BTN_PAUSE (PB11) - Falling Edge.
    
## 📌 Asignación de Pines (Pinout)

| Componente | Pin STM32 | Etiqueta (User Label) | Modo GPIO |
| :--- | :--- | :--- | :--- |
| Segmentos A-G | PA0 - PA6 | `SEG_A` ... `SEG_G` | Output Push-Pull |
| Habilitador 1 | PB0 | `EN1` | Output Push-Pull |
| Habilitador 2 | PB1 | `EN2` | Output Push-Pull |
| Habilitador 3 | PB2 | `EN3` | Output Push-Pull |
| Botón Reset | PB10 | `BTN_RESET` | EXTI (Falling Edge) |
| Botón Pause | PB11 | `BTN_PAUSE` | EXTI (Falling Edge) |
# 🧠 Lógica de Software

El sistema utiliza el NVIC para priorizar las acciones de los botones sobre el bucle principal. Gracias a los capacitores físicos, el código de la ISR (Interrupt Service Routine) permanece limpio y sin necesidad de delays por software.