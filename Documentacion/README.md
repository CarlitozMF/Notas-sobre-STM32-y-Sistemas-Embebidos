# 📘 Guía de Supervivencia: STM32CubeIDE & HAL Layer

Este directorio contiene la base teórica y técnica para el desarrollo de firmware sobre microcontroladores **STM32**. Entender estas particularidades es clave para evitar errores comunes y escribir código profesional.

---

## 1. El Flujo de Trabajo (Workflow)
El desarrollo en STM32 es un ciclo entre la configuración gráfica y la implementación de lógica:

1.  **Configuración (.ioc):** Uso de STM32CubeMX para asignar funciones a los pines y configurar el reloj del sistema (**Clock Tree**).
2.  **Generación de Código:** El IDE traduce la configuración en archivos `.c` y `.h`.
3.  **Implementación de Usuario:** El código DEBE escribirse entre las etiquetas `/* USER CODE BEGIN */` y `/* USER CODE END */`.
    * *Nota:* Si escribes fuera de estas marcas, el IDE borrará tu código al regenerar el proyecto.

---

*Documentación creada por Carlitos MF - Tucumán, Argentina.*