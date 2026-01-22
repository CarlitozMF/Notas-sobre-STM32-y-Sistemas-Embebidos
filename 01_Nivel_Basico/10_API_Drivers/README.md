# 10_API_Drivers: Multitarea Cooperativa y Abstracción 🏗️

En este proyecto elevamos la complejidad del software para demostrar la potencia de la **programación modular**. El objetivo es transformar el código monolítico en componentes reutilizables, permitiendo gestionar múltiples periféricos de forma asíncrona.

## 🧱 Arquitectura del Sistema: "Objetos" y Capas
Hemos diseñado el software para que cada periférico sea una instancia independiente. Esto permite ejecutar tareas con diferentes bases de tiempo sin que una bloquee a la otra, creando un entorno de **multitarea cooperativa**.

* **Capa de Aplicación (`main.c`)**: Solo se encarga de la lógica de alto nivel.
* **Capa de Abstracción (`API_LED`, `API_Delay`)**: Oculta la complejidad de los registros y la HAL de ST.

### Configuración de Tareas:
- **LED Verde (usr_ledVerde)**: Latido rápido (250ms).
- **LED Azul (usr_ledAzul)**: Latido medio (500ms).
- **LED Rojo (usr_ledRojo)**: Latido lento (1000ms).
- **UART (Debug)**: Reporte de estado del sistema cada 1 segundo.

## 🛠️ Conceptos Clave Implementados

### 1. Encapsulamiento con Estructuras
En lugar de variables sueltas, agrupamos la información del periférico en `structs`. Esto permite manejar los 3 LEDs con las mismas funciones de la API, simplemente pasando una referencia diferente.

### 2. Paso por Referencia (Punteros)
Las funciones de la API reciben punteros a las estructuras (`API_Delay_Read(delay_t * hdelay)`). Esto es eficiente en memoria y es el estándar en la industria para el desarrollo de drivers.

### 3. Temporización No Bloqueante
Sustituimos definitivamente `HAL_Delay()` por una lógica de consulta de Ticks (`API_Delay`). Esto permite que el CPU "salte" de una tarea a otra instantáneamente si el tiempo no se ha cumplido.

## 💻 El Corazón de la Multitarea
Fijate cómo el `while(1)` se convierte en una lista de tareas clara y legible, gracias a la delegación de complejidad a los drivers:

```c
/* Definición de instancias (Objetos) */
delay_t delayL1, delayL2, delayL3;
led_t ledV = {LD1_Port, LD1_Pin, false};
led_t ledA = {LD2_Port, LD2_Pin, false};
led_t ledR = {LD3_Port, LD3_Pin, false};

int main(void) {
    // ... Inicialización de Periféricos ...

    /* Inicializamos cada timer con su propia base de tiempo */
    delayInit(&delayL1, 200);
    delayInit(&delayL2, 500);
    delayInit(&delayL3, 1000);

    while (1) {
        // Tarea 1: Control del LED Verde
        if (delayRead(&delayL1)) {
            API_LED_Toggle(&ledV);
        }

        // Tarea 2: Control del LED Azul
        if (delayRead(&delayL2)) {
            API_LED_Toggle(&ledA);
        }

        // Tarea 3: Control del LED Rojo + Mensaje UART
        if (delayRead(&delayL3)) {
            API_LED_Toggle(&ledR);
            Debug_Log("SISTEMA OK: Ciclo de 1 segundo completado\r\n");
        }
    }
}
```
## 🔍 ¿Qué estamos demostrando aquí?
1. Reutilización: Usamos la misma lógica para tres propósitos distintos creando instancias de la estructura delay_t.
2. Independencia: El parpadeo del LED Verde no se ve afectado por el tiempo del LED Rojo.
3. Mantenibilidad: Si cambiamos el hardware de los LEDs, el while(1) permanece intacto; solo actualizamos la declaración de los objetos.

---
*En este proyecto nos introducimos al uso de Drivers y Modularidad para organizar el código. Al utilizar etiquetas en el .ioc y encapsular la lógica en estructuras, logramos un código mucho más genérico.*