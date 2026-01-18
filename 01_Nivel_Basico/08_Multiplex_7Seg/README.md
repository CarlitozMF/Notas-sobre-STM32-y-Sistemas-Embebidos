# 08_Multiplexacion_Manual: Control de 3 Dígitos con POV 🔢

Este proyecto demuestra la técnica de **Persistencia de Visión (POV)** para controlar múltiples displays de 7 segmentos compartiendo un único bus de datos, optimizando el uso de pines GPIO en la placa **Nucleo-F439ZI**.

## 📍 Objetivos del Proyecto
- Implementar **Multiplexación por División de Tiempo**.
- Gestionar hardware con transistores de habilitación (Enable) y bus de datos compartido.
- Utilizar etiquetas personalizadas (**User Labels**) en el archivo `.ioc` para mayor legibilidad.
- Introducir el concepto de **Tiempo No Bloqueante** mediante `HAL_GetTick()`.

## 🧠 El Concepto: Persistencia de Visión
Como los 3 displays comparten los cables de los segmentos (A-G), no podemos encenderlos todos a la vez con números distintos. La solución es encender un solo dígito a la vez a una frecuencia tan alta (mínimo 60Hz) que el ojo humano perciba los tres como si estuvieran encendidos simultáneamente.

## 🛠️ Particularidades del Código

### ⏱️ Gestión de Tiempo con `HAL_GetTick()`
A diferencia de los proyectos anteriores donde usábamos `HAL_Delay()` para todo, aquí introducimos `HAL_GetTick()`.
- **¿Qué es?**: Es una función de la HAL que devuelve la cantidad de milisegundos transcurridos desde que el microcontrolador se encendió.
- **¿Para qué sirve?**: Nos permite crear una "alarma" para incrementar nuestro contador (cada 100ms) **sin detener la ejecución del barrido del display**.

```c
if (HAL_GetTick() - ultimo_tiempo >= 100) {
    ultimo_tiempo = HAL_GetTick();
    contador_global++; // Esto sucede en segundo plano mientras el display brilla
}
```

### 🏷️ Uso de User Labels
Se configuraron etiquetas en el STM32CubeIDE para desacoplar el hardware del software:
- SEG_A ... SEG_G: Bus de datos de 7 bits.
- EN1, EN2, EN3: Control de transistores (Ánodo/Cátodo común).

### 📊 Diagrama de Flujo del Barrido
- Apagar habilitadores (EN1, EN2, EN3) -> Evita efecto "fantasma".
- Cargar datos en el bus (Segmentos A-G).
- Encender habilitador correspondiente.
- Pequeña espera (Persistencia).
- Repetir con el siguiente dígito.

---
*En este proyecto nos introducimos al uso de etiquetas en el .ioc para hacer un código mas generico y además, la función Display_Mux_Scan() todavía vive en el bucle principal (while(1)). Si el CPU se ocupa en otra tarea pesada, el display parpadeará.*