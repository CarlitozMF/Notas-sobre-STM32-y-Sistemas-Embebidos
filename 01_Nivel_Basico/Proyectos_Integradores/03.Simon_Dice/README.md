# EI_03: Simón Dice - Edición Hardware Pro 🎮

Este es el proyecto final del **Nivel Básico**. Representa la integración total de lógica algorítmica, manejo de memoria y gestión de periféricos de entrada/salida en tiempo real.

## 📍 Objetivos del Proyecto
- Implementar un motor de juego basado en **Arreglos Estáticos**.
- Gestionar la aleatoriedad mediante la función `rand()` y semillas dinámicas.
- Crear una interfaz de usuario (HMI) completa con 4 botones y 4 LEDs.
- Diferenciar estados de éxito y error mediante **animaciones de luces**.

## 🧠 Algoritmo de Juego
1. **Generación:** Se añade un valor aleatorio (1-4) al arreglo `secuencia[]`.
2. **Reproducción:** El microcontrolador recorre el arreglo y activa los LEDs correspondientes.
3. **Validación:** El sistema entra en una **espera activa** de botones. Cada pulsación se compara inmediatamente con el valor guardado en memoria.
4. **Condición de Victoria:** Al alcanzar el `NIVEL_VICTORIA` (definido en 10), el sistema activa una secuencia especial de luces antes de reiniciar.



## 🛠️ Desafíos Técnicos Superados
- **Antirrepetidor:** Se implementó una lógica de bloqueo para asegurar que una pulsación larga no sea contada como múltiples entradas.
- **Debounce Físico:** Gestión del ruido mecánico de los pulsadores mediante retardos estratégicos.
- **UX (User Experience):** Uso de la terminal serial para guiar al usuario y mostrar el puntaje en tiempo real.

## 🏆 Cierre de Nivel
Este proyecto demuestra el dominio de:
- `GPIO` (Input/Output).
- `UART` (Transmisión de strings y recepción de caracteres).
- `Lógica Programable` (Bucles, condicionales y memoria).

---
*¡Nivel Básico completado! Estamos listos para el Nivel Intermedio: Interrupciones, Timers y ADC.*