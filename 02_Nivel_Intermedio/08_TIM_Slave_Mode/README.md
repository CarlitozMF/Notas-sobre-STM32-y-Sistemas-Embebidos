# 08_TIM_Slave_Mode: Clasificación de materiales mediante digitalización de frecuencia por Hardware

Este proyecto documenta la implementación del modo **Timer Slave (Reset Mode)** para la medición de alta velocidad de señales de frecuencia provenientes de un sensor de color **TCS3200**. Se integra un sistema de visión artificial de 8 estados, un brazo robótico basado en el servo **SG90** con control de trayectoria suave y una arquitectura asíncrona optimizada para la plataforma **Nucleo-F439ZI**.

## 🎯 Objetivos
- **Dominar el modo Slave (Reset)** para automatizar la captura de periodos sin sobrecarga de interrupciones por software.
- **Desarrollar una Máquina de Estados (FSM)** capaz de discriminar entre colores primarios (RGB) y secundarios (CMY) con histéresis.
- **Implementar un Driver de Servomotor Proporcional** con control de velocidad (`speed_dps`) para evitar estrés mecánico.
- **Garantizar la estabilidad de lectura** mediante técnicas de filtrado de rebotes (debounce) y sincronización de filtros ópticos.

---

## 🔩 Teoría de Operación: Slave Mode y Visión Artificial

### 1. El Modo Slave (Reset Mode): Cronometría Automatizada
A diferencia del *Input Capture* convencional donde el software debe reiniciar el contador, el **Slave Mode (Reset)** configura al Timer para que, al detectar un flanco en la entrada, realice dos acciones simultáneas por hardware:
1. Copia el valor del contador (`CNT`) al registro de captura (`CCR`).
2. Reinicia el contador (`CNT = 0`) inmediatamente.

* **Ventaja Técnica:** Elimina el error acumulado por la latencia del software al reiniciar el Timer. El hardware garantiza que cada captura represente el periodo exacto entre dos pulsos de la señal de salida del TCS3200 ($f_{out}$).

### 2. Digitalización de Color: El Sensor TCS3200
El sensor convierte la intensidad de luz filtrada en una onda cuadrada cuya frecuencia es directamente proporcional a la irradiancia. 
* **Escalamiento:** Configurado al **100% (frecuencia típica de 500-600 kHz)** para maximizar la resolución.
* **Multiplexado Óptico:** Mediante los pines `S2` y `S3`, el microcontrolador conmuta secuencialmente entre filtros Rojos, Verdes y Azules para reconstruir el vector de color.

### 3. Lógica de Decisión con Histéresis Agresiva
Para evitar el "parpadeo" en la detección (especialmente en el color Blanco), se implementó un algoritmo de **Histéresis Dinámica**. 
- **Umbral de Entrada:** Se requiere una frecuencia alta (ej. $>30,000$ Hz) para validar el estado Blanco.
- **Umbral de Salida:** Una vez detectado, se permite que la frecuencia baje significativamente (ej. $12,000$ Hz) antes de abandonar el estado. Esto compensa sombras y variaciones de inclinación del material.



---

### 4. Configuración de los Periféricos

#### A. TIM3: Captura de Frecuencia (Modo Slave)
Configurado como el corazón de la adquisición de datos.

| Parámetro | Valor | Justificación Técnica |
| :--- | :--- | :--- |
| **Prescaler (PSC)** | 89 | Base de tiempos de $1 \mu s$ para medir periodos de señales de hasta 1 MHz. |
| **Slave Mode** | Reset Mode | Reinicio automático del contador tras cada flanco capturado. |
| **Trigger Source** | TI1FP1 | Dispara el reset basado en la entrada del canal 1 (PA6). |

#### B. TIM5: Control de Servo de 32 bits
Se utiliza el **TIM5** por su alta resolución (32 bits) para lograr un movimiento del **SG90** sin vibraciones.

| Parámetro | Valor | Justificación Técnica |
| :--- | :--- | :--- |
| **Prescaler (PSC)** | 89 | Resolución de 1 tick = $1 \mu s$. |
| **Counter Period (ARR)** | 19999 | Periodo de 20ms (50Hz), estándar para servos analógicos. |

---

### 5. Arquitectura de Software: Ejecución Asíncrona
El sistema se aleja del diseño bloqueante de `HAL_Delay()`. El `while(1)` actúa como un planificador de tareas:

1. **`Procesar_Logica_Color()`**: Se ejecuta solo cuando el hardware indica que una medición está lista. Procesa la lógica de 8 estados (Negro, Blanco, Rojo, Verde, Azul, Amarillo, Cian, Magenta).
2. **`SERVO_SG90_Update()`**: Máquina de estados independiente que calcula la posición del brazo basándose en el tiempo transcurrido (`HAL_GetTick()`), permitiendo rampas de aceleración suaves.
3. **`Heartbeat`**: Un LED de diagnóstico parpadea cada 500ms usando el temporizador del sistema para indicar que el kernel del usuario está activo.

```c
// Ejemplo de lógica de discriminación optimizada sin floats (bit-shift)
else if (b > r && g > r) { // Duelo Cian vs Azul
    if (g > (b >> 1)) {    // Si Verde > 50% de Azul (b/2)
        nuevo_estado = ESTADO_CIAN;
    } else {
        nuevo_estado = ESTADO_AZUL;
    }
}
```

---

### 6. Mapeo de Hardware: Nucleo-F439ZI

Para este proyecto se han seleccionado periféricos con acceso directo a canales de Timer de 32 bits y comunicación asíncrona, optimizando el ruteo de señales en la placa:

| Periférico | Pin | Función | Configuración Técnica |
| :--- | :--- | :--- | :--- |
| **TIM3_CH1** | **PA6** | **Entrada de Frecuencia** | Captura del sensor TCS3200 (Slave Mode) |
| **GPIO Out** | **PB2 / PB10** | **Control de Filtros** | Selección de S2 y S3 para multiplexado RGB |
| **TIM5_CH1** | **PA0** | **Control PWM Servo** | Salida de 32 bits para control suave de SG90 |
| **UART3** | **PD8 / PD9** | **Debug Log** | Telemetría serie y calibración en tiempo real |
| **GPIO Out** | **PB0** | **LED de Estado** | Indicador Heartbeat (LD3 - Rojo) |

---

## ⚠️ Lecciones Aprendidas: Integridad de Señal y Debounce

Durante la fase de integración, se identificó una situación crítica con la detección del color **Cian**.

* **Problema:** Debido a que el canal Azul y Verde del sensor TCS3200 tienen una respuesta espectral solapada, se producían "rebotes" lógicos entre el `ESTADO_AZUL` y el `ESTADO_CIAN` bajo ciertas condiciones de iluminación, provocando movimientos erráticos en el servomotor.
* **Solución:** Se implementó un **Filtro de Estabilidad Temporal (Software Debounce)**. El sistema no actualiza la posición del brazo robótico hasta que el sensor confirma el mismo color durante $N$ ciclos de medición consecutivos ($N=2$). Esto garantiza la eliminación de ruidos transitorios y garantiza que el actuador solo responda a estados de color plenamente confirmados.



---

## 🏁 Conclusión

Este laboratorio consolida la importancia de la **delegación de tareas al hardware**. El uso correcto de los **Modos Esclavos (Slave Mode)** en los Timers permite que el microcontrolador digitalice intervalos temporales con precisión de microsegundos sin consumir ciclos de CPU en tareas de cronometría básica. 

La integración de técnicas de **histéresis dinámica** y **filtrado digital** demuestra que es posible transformar un sensor de bajo costo en un sistema de clasificación robusto de grado industrial, permitiendo al núcleo Cortex-M4 enfocarse en la lógica de control de trayectoria y la orquestación asíncrona de periféricos.

---

 *"Nivel Avanzado: Automatización por hardware y procesamiento de señales. El Modo Slave es la clave para la captura de eventos de alta frecuencia donde cada microsegundo cuenta para la precisión del dato final."*