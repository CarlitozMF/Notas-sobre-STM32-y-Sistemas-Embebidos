# 04_Bitwise_Logic - Manipulación de Bits y Protocolos 🔢

Este módulo profundiza en la manipulación quirúrgica de bits dentro de registros y variables. En sistemas embebidos, esta técnica es fundamental para configurar periféricos, optimizar protocolos de comunicación y ahorrar memoria mediante el uso de "bit-fields".

## 📍 Objetivos
- Dominar los operadores lógicos de bajo nivel: `AND`, `OR`, `XOR`, `NOT`.
- Comprender el desplazamiento de bits (`Shifting`) y su rol en protocolos como **I2C**.
- Implementar **Máscaras de Bits** para modificar registros de forma segura.

---

## 🛠️ Los Operadores Lógicos

| Operador | Nombre | Acción Técnica | Aplicación en STM32 |
| :---: | :--- | :--- | :--- |
| **`\|`** | **OR** | Fuerza un bit a `1`. | **SET:** Activar un canal de Timer o un pin. |
| **`&`** | **AND** | Mantiene el bit solo si ambos son `1`. | **READ/CLEAR:** Filtrar estados o apagar bits. |
| **`~`** | **NOT** | Invierte todos los bits. | **MASK:** Invertir una máscara para borrar bits. |
| **`^`** | **XOR** | Invierte el bit si la máscara es `1`. | **TOGGLE:** Conmutar el estado de un LED. |
| **`<<` / `>>`**| **Shift**| Desplaza los bits a los lados. | **OFFSET:** Ubicar un valor en el bit exacto. |



---

## 🛰️ Caso de Uso: El protocolo I2C y el bit R/W

En el protocolo **I2C**, las direcciones de los esclavos suelen ser de **7 bits**. Sin embargo, el hardware del bus requiere un byte completo (8 bits) para funcionar, donde el bit menos significativo (LSB) indica la operación: **Lectura (1)** o **Escritura (0)**.

### El proceso de "Alineación":
1. **Dirección Base:** `0x3C` (Binario: `0011 1100`)
2. **Shift Left (`<< 1`):** Desplazamos para dejar el LSB libre: `0x78` (Binario: `0111 1000`)
3. **Inyección de bit R/W:**
   - Para **Escribir**: `(0x3C << 1) | 0x00` $\rightarrow$ `0x78`
   - Para **Leer**: `(0x3C << 1) | 0x01` $\rightarrow$ `0x79`



---

## 💻 Implementación de Máscaras (Bit Masking)

Una máscara es un valor que nos permite "tapar" los bits que no nos interesan y trabajar solo con el bit objetivo.

```c
// Definimos la posición del bit (ej: Bit 3 de un registro)
#define ERROR_FLAG_BIT    (1 << 3)  // Resultado: 0000 1000

/* 1. ACTIVAR (SET) */
// Usamos OR para poner el bit 3 en '1' sin alterar el resto del registro
registro |= ERROR_FLAG_BIT;

/* 2. VERIFICAR (READ) */
// Usamos AND para aislar el bit 3 y comprobar su estado
if (registro & ERROR_FLAG_BIT) {
    // El bit está activo
}

/* 3. LIMPIAR (CLEAR) */
// Usamos AND con la máscara invertida (NOT) para poner solo el bit 3 en '0'
registro &= ~ERROR_FLAG_BIT;
```

## 🔍 Visualización: ¿Por qué Hexadecimal?

En este laboratorio utilizamos el especificador %02X para telemetría.

* Decimal (%u): No tiene relación directa con la arquitectura de bits.

* Hexadecimal (%X): Cada dígito representa exactamente un Nybble (4 bits).

   * 0xA es 1010.

   * 0xF es 1111. Esto permite identificar instantáneamente qué bit está encendido dentro de un registro de control de la STM32F439ZI simplemente mirando el código hexadecimal.

---
*Dominar la lógica de bits es el primer paso para dejar de usar el microcontrolador como una caja negra y empezar a orquestar el hardware desde sus registros.*