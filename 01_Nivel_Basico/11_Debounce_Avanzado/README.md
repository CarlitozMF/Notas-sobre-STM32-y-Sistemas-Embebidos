# 11_Debounce_Avanzado: Integración de MEF y Drivers Profesionales 🏆

Este es el proyecto final del **Nivel Básico**. Representa la integración total de los pilares de la ingeniería de sistemas embebidos: **Máquinas de Estado Finitas (MEF)**, **Abstracción de Hardware** y **Temporización No Bloqueante**. El objetivo es crear un driver de antirrebote (debounce) robusto que entregue una señal lógica limpia al sistema.

## 🧠 El Problema: El Rebote Mecánico (Bouncing)
Cuando un interruptor mecánico se cierra, las láminas metálicas vibran y colisionan durante unos milisegundos antes de establecer un contacto estable. 



Para un microcontrolador como el **STM32F439ZI**, estos rebotes de ~20-40ms se interpretan como cientos de pulsaciones falsas. La solución profesional no es "congelar" el micro con un delay, sino implementar una MEF que filtre el ruido de forma asíncrona.

---

## 🏗️ Arquitectura del Driver: Genérico y Reentrante
A diferencia de una MEF simple, este driver es **reentrante**. Esto significa que la lógica está desacoplada de los datos; podemos controlar múltiples botones simultáneamente creando diferentes instancias de la estructura `button_t`.

### Estados de la MEF (Modelo de Moore):
1. **`BUTTON_UP`**: Estado de reposo (contacto abierto).
2. **`BUTTON_FALLING`**: Se detecta un cambio inicial. Se inicia la ventana de validación (40ms).
3. **`BUTTON_DOWN`**: La señal se mantuvo estable. Pulsación confirmada.
4. **`BUTTON_RISING`**: Se detecta la apertura del contacto. Se inicia la validación de liberación.



---

## 💻 Implementación de Alto Nivel

La potencia de este diseño es el uso de **Drivers jerárquicos**: la `API_Debounce` utiliza internamente nuestra `API_Delay`. Esto mantiene el `main.c` extremadamente limpio y enfocado en la aplicación:

```c
/* Instancia de objeto para el botón azul de la placa (Active Low) */
button_t botonUsuario = {GPIOC, GPIO_PIN_13, true}; 

int main(void) {
    // ... Inicialización de periféricos y UART ...
    debounceFSM_Init(&botonUsuario); 

    while (1) {
        /* Tarea 1: Actualización constante de la MEF del botón */
        // Proceso asíncrono que filtra ruidos en background
        debounceFSM_Update(&botonUsuario);

        /* Tarea 2: Lógica de aplicación basada en eventos validados */
        if (readKey(&botonUsuario)) {
            API_LED_Toggle(&usr_ledRojo);
            Debug_Log("EVENTO: Pulsación filtrada y confirmada!\r\n");
        }
        
        // Tarea 3: El CPU está libre para procesar otras lógicas...
    }
}
```

## 🔍 ¿Qué estamos demostrando aquí?

1. **Jerarquía de Drivers:** Un driver de alto nivel consume servicios de un driver de bajo nivel, siguiendo las mejores prácticas de la industria.

2. **Robustez Determinista:** El sistema ignora ruidos eléctricos y mecánicos, asegurando que un evento de software corresponda exactamente a una acción del usuario.

3. **Multitarea Cooperativa:** Mientras la FSM espera los 40ms de validación, el procesador sigue ejecutando el resto de las tareas del while(1), eliminando el tiempo ocioso.

# 🏁 Cierre del Nivel Básico

Con este laboratorio, hemos transformado el microcontrolador en un sistema capaz de gestionar eventos del mundo real con precisión profesional.

Herramientas dominadas:

* Mapeo de hardware mediante estructuras.

* Comunicación serie para telemetría.

* Máquinas de estado para lógica de control.

* Temporización asíncrona mediante Ticks.

*Hacia el Nivel Intermedio: Hemos llegado al límite del Polling. En el siguiente nivel, aprenderemos a configurar el hardware para que el CPU no tenga que "preguntar" constantemente; usaremos Interrupciones (EXTI) para que el hardware nos avise cuando algo ocurra.*

---
*La ingeniería no consiste en evitar los problemas físicos del hardware, sino en diseñar el software capaz de absorberlos y entregar una respuesta lógica perfecta.*