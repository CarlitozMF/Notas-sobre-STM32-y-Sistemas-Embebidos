# 📂 Documentación del Sistema Embebido (STM32F439ZI)

Bienvenido al directorio central de documentación de este proyecto. En este espacio se detalla el análisis, configuración y desarrollo del firmware diseñado para la placa de desarrollo **NUCLEO-F439ZI**, estructurado bajo una **Arquitectura de Software Modular de 3 Capas** y explotando la **Capa HAL** nativa mediante el entorno **STM32CubeIDE**.

El objetivo de esta documentación es registrar de forma rigurosa la progresión técnica del proyecto, desde el análisis del silicio y la hoja de datos (*Datasheet*) hasta la consolidación de Máquinas de Estados Finitos (MFS) no bloqueantes.

---

## 🧭 Mapa de Ruta y Estructura de Guías

La documentación está organizada de forma secuencial y evolutiva. Cada carpeta contiene su propio archivo `README.md` detallado con explicaciones conceptuales, diagramas arquitectónicos y buenas prácticas de codificación:

### ⚙️ Fundamentos de Hardware y Arquitectura
* **[00. Overview del Hardware (Guía 00)](./00_Hardware_Overview/README.md):** Introducción a la placa NUCLEO-F439ZI, topología eléctrica, pines accesibles de laboratorio y periféricos integrados de usuario (LEDs y pulsador).
* **[01. El Núcleo de Silicio: STM32F439ZI (Guía 01)](./01_STM32F439ZI/README.md):** Análisis en profundidad de la arquitectura Cortex-M4, mapa de memoria, organización de buses de datos internos (AHB/APB) y árbol de relojes del sistema.
* **[02. Filosofía y Arquitectura de Software (Guía 02)](./02_Arquitectura_De_Software/README.md):** Desglose conceptual de nuestro modelo de desarrollo basado en 3 Capas independientes (Hardware Mapping, Abstraction Drivers, Application/FSM) para garantizar la modularidad y portabilidad.

### 🛠️ Configuración de Entorno e Infraestructura
* **[03. Entorno de Desarrollo: STM32CubeIDE (Guía 03)](./03_STM32CubeIDE/README.md):** Justificación y configuración del IDE del fabricante, gestión del workspace, y flujo de compilación basado en el compilador GCC de ARM.
* **[04. Creación Estructurada de Proyectos (Guía 04)](./04_Creacion_Proyectos/README.md):** Paso a paso técnico para dar de alta un proyecto limpio en el entorno, inicialización de estructuras esenciales y configuraciones de depuración (*Debug*).
* **[05. Configuración del Periférico desde el IOC (Guía 05)](./05_Configuracion_IOC/README.md):** Uso profesional del configurador gráfico. Configuración de frecuencias para poner el **SYSCLK a 180 MHz** y asignación semántica de **User Labels** en los pines analizados.

### 💻 Orquestación de Firmware y Abstracción Primaria
* **[06. Anatomía del main.c y Secciones de Usuario (Guía 06)](./06_Main.c/README.md):** Inspección minuciosa del archivo raíz generado por el IDE, flujo de ejecución tras salir de Reset (`HAL_Init()`, `SystemClock_Config()`) y la regla de oro para la convivencia de código mediante los bloques `USER CODE`.
* **[07. Hardware Mapping e Infraestructura de la HAL (Guía 07)](./07_Hardware_Mapping/README.md):** La consolidación de la **Capa 1**. Comparativa entre la programación con pines fijos (*hardcoded*) frente al uso de *User Labels*, y cómo el archivo `main.h` actúa como el mapa de hardware oficial del proyecto.

---

## 🧠 Pilares de Ingeniería Aplicados

A lo largo de todo el desarrollo documentado en estas guías, se aplican de forma estricta los siguientes criterios de robustez en sistemas embebidos:

1.  **Bloqueo Cero (Zero-Blocking):** Prohibición terminante del uso de funciones de retraso por software refractarias (como `HAL_Delay()`) dentro del lazo principal de ejecución, priorizando bases de tiempo por hardware y contadores no bloqueantes.
2.  **Abstracción Semántica:** Aislamiento absoluto de los números físicos de los pines en la lógica de control superior; toda llamada al hardware debe realizarse mediante alias significativos.
3.  **Higiene del main:** El archivo `main.c` opera únicamente como un inicializador de hardware y orquestador de alto nivel, derivando la lógica algorítmica compleja a módulos independientes.

---

🛠️ **Carlos** | Estudiante de Ing. Electrónica @UTN_FRT.  
🚀 Apasionado Autodidacta por los Sistemas Embebidos.