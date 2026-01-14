# 01_Blinky_GPIO - El Hola Mundo Embebido

Este proyecto implementa una secuencia de parpadeo utilizando los tres LEDs integrados en la placa Nucleo-F439ZI.

## 📍 Configuración de Hardware
Los LEDs de usuario en esta placa están conectados a los siguientes pines del **GPIOB**:
- **LD1 (Verde):** Pin `PB0`
- **LD2 (Azul):** Pin `PB7`
- **LD3 (Rojo):** Pin `PB14`

## ⚙️ Conceptos Aprendidos
- Configuración de pines como **Salida Digital (Output)**.
- Uso de `HAL_GPIO_WritePin` para encender/apagar.
- Uso de `HAL_Delay` para generar pausas temporales.

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