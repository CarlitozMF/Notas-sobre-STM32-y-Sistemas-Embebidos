# 02_TIM_Basic: Gestión de Tiempo y Base de Tiempo con TIM1 ⏱️

Este proyecto documenta el uso de **Timers** de hardware para crear bases de tiempo precisas en la **Nucleo-F439ZI**. El objetivo es demostrar cómo el hardware puede gestionar tareas críticas (como el parpadeo de un LED) de forma asíncrona, liberando al procesador para realizar otras tareas en el bucle principal sin usar funciones bloqueantes como `HAL_Delay()`.

## 🧮 Configuración del Reloj (Clock Tree)

El primer paso crítico es entender la frecuencia de entrada del periférico. Según la configuración de reloj de nuestra placa, el bus **APB2** corre a 90 MHz. Sin embargo, debido a la arquitectura de la familia STM32F4, si el prescaler del bus APB es distinto de 1, la frecuencia del Timer se multiplica automáticamente por 2.

Por lo tanto, el **APB2 Timer clocks** tiene una frecuencia efectiva de **180 MHz**.

> **Referencia del Clock Tree:**
> ![Clock Configuration](./assets/02_TIM_Basic_CLKConf.png)
> *Se observa que el TIM1 recibe 180 MHz para su lógica de conteo.*

## 📏 Matemática del Timer y Fórmulas

Para obtener un parpadeo preciso, aplicamos las siguientes fórmulas de hardware:

### 1. Frecuencia de Conteo ($f_{CNT}$)
Es la velocidad a la que incrementa el contador interno tras pasar por el Prescaler.
$$f_{CNT} = \frac{f_{clk}}{(PSC + 1)}$$

### 2. Tiempo de Interrupción ($T_{Update}$)
Es el intervalo en el que el Timer llega al valor de Auto-Reload (ARR) y dispara la interrupción (Update Event).
$$T_{Update} = \frac{(ARR + 1)}{f_{CNT}}$$

### 3. Cálculo aplicado en este ejemplo:
Para los valores configurados de $PSC = 8999$ y $ARR = 4999$:
* $f_{CNT} = \frac{180.000.000}{9000} = 20.000 \text{ Hz} (20 \text{ kHz})$
* $T_{Update} = \frac{5000}{20.000} = \mathbf{0.25 \text{ s} (250 \text{ ms})}$

## 🔬 Validación con Analizador Lógico

Para verificar que el firmware cumple con los cálculos teóricos, conectamos un analizador lógico al pin **PF13** (`usr_led`). 

> **Captura del Analizador Lógico:**
> ![Analizador Lógico](Captura_Analizador_Logico.png)
> *La medición confirma que el LED permanece en alto 250 ms y en bajo 250 ms (Duty Cycle 50%), resultando en una frecuencia de oscilación de **2 Hz**.*

## 💻 Lógica de Software Implementada

### 1. Interrupción de Hardware (Hard Real-Time)
El parpadeo del LED se gestiona mediante la interrupción de **Update** del TIM1. Esta tarea es prioritaria y ocurre en "background", garantizando que el tiempo de conmutación sea exacto.

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM1) {
        // El estado cambia cada 250ms (T_Update)
        HAL_GPIO_TogglePin(usr_led_GPIO_Port, usr_led_Pin); 
    }
}
```
### 2. Bucle no bloqueante (Soft Real-Time)
En lugar de usar *HAL_Delay()*, empleamos **HAL_GetTick()** (basado en el Systick del núcleo) para enviar logs por USART3 cada 1 segundo. Esto permite que el CPU ejecute miles de ciclos de while(1) mientras espera el tiempo de log, manteniendo el sistema reactivo.

```c
while (1) {
    if (HAL_GetTick() - last_log_time >= 1000) {
        last_log_time = HAL_GetTick();
        sprintf(msg_buffer, "Ciclo de Main: %lu\r\n", loop_counter++);
        Debug_Log(msg_buffer);
    }
    // El CPU está libre para procesar otras tareas aquí
}
```
## 📝 Conclusiones

- Independencia: El LED mantiene su ritmo perfecto aunque el while(1) realice comunicaciones pesadas o cálculos.
- Relación Toggle/Periodo: El periodo total de la señal del LED (500 ms) es el doble del tiempo de interrupción del Timer (250 ms), ya que se requieren dos eventos de Update para completar un ciclo (Encendido + Apagado).
- Importancia de la Medición: El analizador lógico es la herramienta definitiva para validar que el Clock Tree y los registros PSC/ARR están correctamente sincronizados.