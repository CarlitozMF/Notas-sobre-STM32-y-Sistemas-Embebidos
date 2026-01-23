# 11_Debounce_Avanzado: Integración de MEF y Drivers Profesionales 🛠️

Este es el proyecto final del **Nivel Básico**. Representa la integración total de los conceptos aprendidos: Máquinas de Estado Finita (MEF), Abstracción de Hardware y Temporización No Bloqueante. El objetivo es crear un driver de antirrebote (debounce) que entregue una señal limpia al sistema.

## 🧠 El Problema: El Rebote Mecánico (Bouncing)
Cuando presionamos un botón, las láminas metálicas no hacen contacto perfecto instantáneamente, sino que "rebotan" durante unos milisegundos. Esto genera múltiples flancos que el microcontrolador interpreta como varias pulsaciones.

**Solución**: Implementar una MEF que "espere" un tiempo determinado (40ms) hasta que la señal se estabilice antes de confirmar la pulsación.


## 🏗️ Arquitectura del Driver
A diferencia del ejemplo de MEF básico, aquí el driver es **genérico y reentrante**. Esto significa que podemos usar el mismo código para manejar múltiples botones simultáneamente creando diferentes instancias.

### Estados de la MEF:
1. **BUTTON_UP**: Estado de reposo (botón suelto).
2. **BUTTON_FALLING**: Se detecta un cambio, se inicia el temporizador de validación.
3. **BUTTON_DOWN**: El botón está presionado y la señal es estable.
4. **BUTTON_RISING**: Se detecta que se soltó el botón, se inicia el temporizador de validación.

## 💻 Implementación de Alto Nivel
La potencia de este driver es que utiliza nuestra `API_Delay` internamente. El `main.c` se mantiene limpio, delegando toda la lógica de filtrado al módulo:

```c
/* Estructura que define nuestro botón */
button_t botonUsuario = {GPIOC, GPIO_PIN_13, true}; // Botón azul (Active Low)

int main(void) {
    // ... Inicialización ...
    debounceFSM_Init(); // Configura el timer de 40ms interno

    while (1) {
        /* Actualizamos la máquina de estados del botón */
        debounceFSM_Update(&botonUsuario);

        /* Consultamos si hubo una pulsación válida (limpia de rebotes) */
        if (readKey(&botonUsuario)) {
            API_LED_Toggle(&usr_ledRojo);
            Debug_Log("Pulsación confirmada y filtrada!\r\n");
        }
    }
}
```

## 🔍 ¿Qué estamos demostrando aquí?

1. Jerarquía de Drivers: Un driver de alto nivel (API_Debounce) utiliza un driver de bajo nivel (API_Delay).
2. Robustez: El sistema ignora ruidos eléctricos y mecánicos de corta duración.
3. Multitarea: Mientras la FSM espera los 40ms para validar, el CPU sigue ejecutando otras tareas en el while(1).

---
*En este proyecto finalizamos el nivel básico integrando el uso de etiquetas en el .ioc, abstracción por estructuras y lógica de estados. Hemos resuelto el problema del rebote mecánico de forma profesional. Con estas herramientas, estamos listos para el Nivel Intermedio, donde dejaremos de "preguntar" por el estado (Polling) y pasaremos a usar Interrupciones (IT) para una respuesta en tiempo real.*