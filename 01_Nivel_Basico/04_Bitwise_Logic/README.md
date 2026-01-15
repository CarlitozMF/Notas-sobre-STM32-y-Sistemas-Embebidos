# 04_Bitwise_Logic - Operaciones de Bits y Caso I2C 🔢

Este módulo explica cómo manipular bits individuales dentro de un byte o registro. En sistemas embebidos, esta técnica es esencial para configurar periféricos, leer sensores y optimizar el uso de la memoria.

## 📍 Objetivos
- Dominar los operadores lógicos de bits: `AND`, `OR`, `XOR`, `NOT`.
- Entender el desplazamiento de bits (`Shift`) y su aplicación en protocolos como **I2C**.
- Aprender a utilizar máscaras de bits para leer y modificar registros.

## 🛠️ Los Operadores de "Cirujano"

| Operador | Nombre | Acción | Uso Típico |
| :---: | :--- | :--- | :--- |
| `\|` | **OR** | Pone un bit en 1. | **SET:** Activar una función en un registro. |
| `&` | **AND** | Filtra bits específicos. | **READ:** Verificar si una bandera está activa. |
| `~` | **NOT** | Invierte los bits. | **MASK:** Invertir una máscara para apagar bits. |
| `^` | **XOR** | Conmuta el estado. | **TOGGLE:** Cambiar de 0 a 1 y viceversa. |
| `<<` | **L-Shift** | Desplaza a la izquierda.| Crear máscaras (ej: `1 << 3`). |

## 🛰️ Caso de Uso Real: Direccionamiento I2C

Un uso fundamental del desplazamiento de bits ocurre en el protocolo **I2C**. Las direcciones de los dispositivos suelen ser de **7 bits**, pero el bus requiere un byte (8 bits) para incluir el bit de **Lectura/Escritura (R/W)**.

Para esto, desplazamos la dirección un lugar a la izquierda:
1. **Dirección Original:** `0x3C` (binario: `0011 1100`)
2. **Shift Left (`<< 1`):** La dirección se convierte en `0x78` (`0111 1000`). El último bit queda libre.
3. **Bit R/W:** - Si queremos **Escribir**, dejamos el último bit en `0`.
   - Si queremos **Leer**, aplicamos un `OR 0x01` para poner el último bit en `1`.



## 💻 Ejemplo de Código

```c
// Definición de una máscara para un bit de error
#define BIT_ERROR (1 << 3) 

// Activar el bit de error (SET)
registro |= BIT_ERROR;

// Limpiar el bit de error (CLEAR)
registro &= ~BIT_ERROR;

// Preparar dirección I2C para lectura
uint8_t read_addr = (DEVICE_ADDR << 1) | 0x01;