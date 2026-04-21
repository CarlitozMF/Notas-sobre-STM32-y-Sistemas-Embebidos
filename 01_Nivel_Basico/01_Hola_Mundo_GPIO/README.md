# 01_Hola_Mundo_GPIO - El "Hello World" Embebido

Este proyecto implementa una secuencia de desplazamiento de luz (chaser) utilizando los tres LEDs integrados en la placa **Nucleo-F439ZI**. El objetivo es dominar la manipulación básica de salidas digitales y comprender la gestión de tiempo bloqueante.

## 📍 Configuración de Hardware
Según el manual de usuario **UM1974**, los LEDs están conectados al puerto **GPIOB** en una configuración *Active-High* (se encienden con un '1' lógico):

| LED | Color | Pin | Registro |
| :--- | :--- | :--- | :--- |
| **LD1** | Verde | `PB0` | `GPIOB->ODR` |
| **LD2** | Azul | `PB7` | `GPIOB->ODR` |
| **LD3** | Rojo | `PB14`| `GPIOB->ODR` |



---

## ⚙️ Evolución de la Lógica de Control

### 1. Control Explícito con `HAL_GPIO_WritePin`
En la primera etapa, se fuerza el estado de cada pin individualmente. Es el método más seguro para inicializar estados conocidos.
* `GPIO_PIN_SET`: Pone el pin a 3.3V.
* `GPIO_PIN_RESET`: Pone el pin a 0V.

### 2. Optimización y Atomización con `HAL_GPIO_TogglePin`
La refactorización utiliza la función de conmutación (Toggle). Para lograr el desplazamiento, se aplica una **lógica de estado previo**:
1. Se inicializa el **LD1** encendido fuera del ciclo.
2. Dentro del `while(1)`, se aplica Toggle al LED actual y al siguiente simultáneamente tras un retardo. Esto apaga uno y enciende el otro en el mismo instante del flujo de código.

---

## 🚀 Implementación Final (Desplazamiento Eficiente)

```c
/* Inicialización: El sistema arranca con el LED Verde encendido */
HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET); 

while (1)
{
    HAL_Delay(500); // Latencia bloqueante de 500ms
    
    // Paso 1: Apaga Verde y enciende Azul simultáneamente
    HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
    HAL_GPIO_TogglePin(GPIOB, LD2_Pin);
    
    HAL_Delay(500);
    // Paso 2: Apaga Azul y enciende Rojo simultáneamente
    HAL_GPIO_TogglePin(GPIOB, LD2_Pin);
    HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
    
    HAL_Delay(500);
    // Paso 3: Apaga Rojo y enciende Verde para cerrar el lazo
    HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
    HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
}
```

## ⚠️ Análisis Crítico: La Trampa de HAL_Delay()

Debemos entender que HAL_Delay(ms) es un mecanismo de espera ocupada (Busy-Wait).
* ¿Cómo funciona?: El procesador entra en un bucle cerrado consultando el valor de la variable uwTick (incrementada cada 1ms por la interrupción del SysTick).
* Impacto en CPU: Durante el delay, el microcontrolador desperdicia millones de ciclos de reloj. Si ocurriera un evento crítico (como la pulsación de un botón de emergencia), el programa no lo detectaría hasta que termine el delay.
* Conclusión: Esta técnica es aceptable para prototipos simples, pero en aplicaciones profesionales será reemplazada por Timers y Gestión de Tiempo No Bloqueante (visto en laboratorios posteriores).

---

*Notas sobre la clásica primera práctica de programación adaptada a los sistemas embebidos*

🛠️ **Carlos** | Estudiante de Ing. Electrónica @UTN_FRT.  
🚀 Apasionado Autodidacta por los Sistemas Embebidos.