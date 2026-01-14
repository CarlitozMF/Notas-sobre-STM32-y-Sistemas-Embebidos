# 01_Blinky_GPIO - El Hola Mundo Embebido

Este proyecto implementa una secuencia de parpadeo (desplazamiento de luz) utilizando los tres LEDs integrados en la placa Nucleo-F439ZI.

## 📍 Configuración de Hardware
Los LEDs de usuario en esta placa están conectados al **GPIOB**:
- **LD1 (Verde):** Pin `PB0`
- **LD2 (Azul):** Pin `PB7`
- **LD3 (Rojo):** Pin `PB14`

## ⚙️ Conceptos Aprendidos

### 1. Uso de HAL_GPIO_WritePin
Inicialmente, la secuencia se realizó forzando el estado de cada pin. Esto es útil para asegurar estados específicos, pero requiere más líneas de código.
- `GPIO_PIN_SET`: Enciende el LED.
- `GPIO_PIN_RESET`: Apaga el LED.

## 🚀 Código Principal
```c
  while (1)
  {
	  /* 1. Encender LED Verde (LD1) y esperar 500ms */
	      HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET);
	      HAL_Delay(500);

	      /* 2. Encender LED Azul (LD2) y apagar el Verde */
	      HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_RESET);
	      HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET);
	      HAL_Delay(500);

	      /* 3. Encender LED Rojo (LD3) y apagar el Azul */
	      HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
	      HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);
	      HAL_Delay(500);

	      /* 4. Apagar todo y reiniciar ciclo */
	      HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
  }
```

### 2. Optimización con HAL_GPIO_TogglePin
Posteriormente, se refactorizó el código para utilizar **Toggle**, que invierte el estado lógico actual del pin.

**Lógica de desplazamiento:**
Para que la luz "salte" de un LED a otro usando Toggle, se debe:
1. Inicializar un LED en estado `SET` antes del bucle principal.
2. Dentro del `while(1)`, aplicar Toggle simultáneamente al LED encendido y al siguiente en la secuencia. Esto apaga el actual y enciende el próximo en una sola operación lógica.

## 🚀 Código Final (Secuencia de Desplazamiento)

```c
/* Inicialización: Encendemos el primer LED */
HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET); 

while (1)
{
    HAL_Delay(500);
    // Paso 1: Apaga Verde y enciende Azul
    HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
    HAL_GPIO_TogglePin(GPIOB, LD2_Pin);
    
    HAL_Delay(500);
    // Paso 2: Apaga Azul y enciende Rojo
    HAL_GPIO_TogglePin(GPIOB, LD2_Pin);
    HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
    
    HAL_Delay(500);
    // Paso 3: Apaga Rojo y enciende Verde para reiniciar el ciclo
    HAL_GPIO_TogglePin(GPIOB, LD3_Pin);
    HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
}
```
## ⚠️ Nota importante sobre HAL_Delay()

La función `HAL_Delay(ms)` es la forma más sencilla de generar pausas, pero tiene una característica crítica: **es una función bloqueante**.

- **¿Cómo funciona?**: El procesador entra en un bucle cerrado comparando el tiempo actual con el valor del `Systick` (un contador interno del microcontrolador) hasta que transcurren los milisegundos indicados.
- **Limitación**: Mientras el microcontrolador está ejecutando el "delay", **no puede realizar ninguna otra tarea** en el bucle principal (`while(1)`). Solo las interrupciones con mayor prioridad pueden interrumpir este tiempo.
- **Uso recomendado**: Se debe usar solo para inicializaciones simples o aplicaciones donde no importe que el micro se "congele" momentáneamente. En aplicaciones profesionales de tiempo real, se suelen utilizar **Timers** o **Interrupciones** para manejar el tiempo sin bloquear el sistema.