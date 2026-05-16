# 05. ⚙️ Configuración de Periféricos e Infraestructura (.ioc)

En esta guía abordamos la configuración lógica del microcontrolador utilizando la interfaz gráfica. Estableceremos el corazón del sistema (Clock) y la interfaz de entrada/salida (GPIO), sentando las bases para que nuestros drivers de **Capa 2** operen de forma eficiente.

---

## 1. ⏱️ Configuración del System Clock (180 MHz)
Para que el STM32F439ZI rinda al máximo de su capacidad según el **Reference Manual (RM0090)**, debemos configurar correctamente el **RCC (Reset and Clock Control)**.
1.  **Pestaña System Core:**
    * Sobre el Item RCC y en la ventana *RCC Mode and Configuration* habilitar la opción **Crystal/Ceramic Resonator** para poder usar el cristal externo y poder llegar a la maxima frecuencia *HCLK* (180 MHz).

    <p align="center">
      <img src="./assets/01.png" alt="Habilitar RCC" width="500">
      <br>
      <em>Figura 1: Habilitar cristal externo para RCC.</em>
    </p>

2.  **Pestaña Clock Configuration:**
    * **Input Frequency:** Aseguramos que la entrada desde el ST-LINK (HSE) esté en **8 MHz**.
    * **PLL Source Mux:** Seleccionamos **HSE** (High Speed External).
    * **System Clock Mux:** Seleccionamos **PLLCLK**.
    * **HCLK (MHz):** Escribimos **180** y presionamos Enter. El IDE calculará automáticamente los divisores $M, N, P, Q$.

    <p align="center">
      <img src="./assets/02.png" alt="Habilitar Max Frecuencia HCLK" width="500">
      <br>
      <em>Figura 2: Configurar HCLK a 180 MHz.</em>
    </p

> [!CAUTION]
> **Recomendación:** Las placas que tienen la conexión ethernet y otg inician por defecto y su pre configuración altera el calculo de la frecuencia máxima de la placa por ello, para llegar a los 180 MHz se recomienda desactivarlos.

> [!CAUTION]
> **Límites de Bus:** Al configurar 180 MHz, es vital verificar que el bus **APB1** no supere los $45\text{ MHz}$ y el **APB2** los $90\text{ MHz}$. El configurador gráfico resaltará en rojo si los periféricos están fuera de rango, lo que podría causar fallos en comunicaciones I2C o UART.


<p align="center">
    <img src="./assets/03.png" alt="Deshabilitar Módulo ETH integrado" width="500">
      <br>
      <em>Figura 3: Deshabilitar el modo RMII del módulo ETHERNET.</em>
</p

<p align="center">
    <img src="./assets/04.png" alt="Deshabilitar Módulo OTG integrado" width="500">
      <br>
      <em>Figura 4: Deshabilitar el modo Device_Only del módulo USB-OTG.</em>
</p

---

## 2. 📍 Configuración de GPIO (I/O)
En el **Pinout View**, podemos definir la función de cada pin. Para el desarrollo del firmware y drivers propios, nos enfocamos en:

* **GPIO_Output:** Utilizado para los LEDs de usuario (`LD1-PB0`, `LD2-PB7`, `LD3-PB14`).
* **GPIO_Input:** Utilizado para el botón de usuario (`USR_BTN-PC13`).

<p align="center">
    <img src="./assets/05.png" alt="Pinout View" width="500">
      <br>
      <em>Figura 5: Pinout por defecto.</em>
</p

> [!IMPORTANT]
> Para configurar cualquier pin de la placa basta con posarse sobre el cuadro gris asociado al pin que queremos configurar y clickear con el boton izquierdo para desplegar todas las posibles configuraciones del mismo.

<p align="center">
    <img src="./assets/06.png" alt="Pin Config" width="500">
      <br>
      <em>Figura 6: Ejemplo de posibles configuraciones del pin PB11.</em>
</p

### Opciones de Configuración Técnica:
En la pestaña **System Core -> GPIO**, ajustamos los parámetros que definen el comportamiento eléctrico del pin:
* **GPIO Mode:** *Push-Pull* (estándar) o *Open Drain* (para buses como I2C).
* **GPIO Pull-up/Pull-down:** Activar resistencias internas para evitar estados flotantes en entradas.
* **Maximum Output Speed:** Determina el *Slew Rate*. Para LEDs, se recomienda **Low** para minimizar el ruido electromagnético y el consumo.

<p align="center">
    <img src="./assets/07.png" alt="GPIO MODE AND CONFIG="300">
      <br>
      <em>Figura 7: Ventana GPIO Mode and Configuration.</em>
</p

---

## 3. 🏷️ Uso de Etiquetas (User Labels)
Para garantizar la independencia de nuestra **Capa 3 (Aplicación)**, utilizaremos **User Labels**. Esto evita "hardcodear" números de pines en el código principal.

1.  Hacer clic derecho sobre el pin (ej. `PB11`).
2.  Seleccionar **Enter User Label**.
3.  Asignar un nombre semántico (ej. `LED_VERDE`).

<p align="center">
    <img src="./assets/08.png" alt="GPIO PB11 OUTPUT="300">
      <br>
      <em>Figura 8: Configuración PB11 como Salida (OUTPUT).</em>
</p

<p align="center">
    <img src="./assets/09.png" alt="USER LABEL="300">
      <br>
      <em>Figura 9: Cargar Label asociado a PB11.</em>
</p

<p align="center">
    <img src="./assets/10.png" alt="USER LABEL II="300">
      <br>
      <em>Figura 10: Etiqueta PIN_SALIDA asociada al Pin PB11.</em>
</p

**Ventaja Arquitectónica:**

El IDE generará macros en `main.h` del tipo:
`#define PIN_SALIDA_GPIO_PIN_11`
`#define PIN_SALIDA_GPIO_Port GPIOB`

Esto permite que nuestra aplicación use el nombre `PIN_SALIDA` sin importar si en una revisión futura de hardware el LED se mueve a otro puerto.

---

## 🏗️ Preparación para la Generación de Código
Antes de generar el código fuente (Ctrl + S), debemos asegurar la **Modularidad** en la pestaña **Project Manager -> Code Generator**:

* **Marcado:** *"Generate peripheral initialization as a pair of '.c/.h' files per peripheral"*. 
    * Esto evita tener un `main.c` gigante y facilita la integración con nuestra **Capa 2**.

---

## 🧭 Mapa de Ruta
1.  **Guía 06:** Anatomía del archivo `main.c` y gestión de Secciones de Usuario.

---
**Siguiente paso:** [06_Estructura_Main](../06_Estructura_Main/README.md)

🛠️ **Carlos** | Estudiante de Ing. Electrónica @UTN_FRT.  
🚀 Apasionado Autodidacta por los Sistemas Embebidos.