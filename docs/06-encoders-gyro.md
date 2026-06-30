# Encoders

## Hardware

OPRcontrolFOC usa **encoders magnéticos de cuadratura** (AS5600 o compatibles)
para medir la posición y velocidad de cada motor. El encoder va montado en el
eje del motor y detecta la orientación de un imán diametralmente magnetizado.

| Característica | Detalle |
|---------------|---------|
| **Modelo** | AS5600 o compatible |
| **Resolución** | 4096 CPR (cuadratura ×4) |
| **Posición absoluta por vuelta eléctrica** | 586 ticks (`MAX_ABSOLUTE_POSITION`) |
| **Interfaz** | Timer en modo encoder (cuadratura A/B) + pulso índice Z |
| **Timer izquierda** | TIM3 (PB4=CH1, PB5=CH2) con partial remap |
| **Timer derecha** | TIM4 (PB6=CH1, PB7=CH2) |
| **Índice Z izquierda** | PB3 → EXTI3 (rising edge) |
| **Índice Z derecha** | PA15 → EXTI15 (rising edge) |

> **Nota**: `MAX_ABSOLUTE_POSITION = 585` no coincide con los 4096 CPR del
> encoder. Esto es porque el motor tiene varios pares de polos magnéticos,
> por lo que una vuelta mecánica completa equivale a múltiples vueltas eléctricas.
> El valor 586 representa los ticks por vuelta **eléctrica**, no mecánica.

---

## Configuración de Timers

Los timers de encoder se configuran en modo **encoder EM3** (cuadratura en ambos
flancos de ambos canales):

```c
timer_set_period(TIM3, 0xFFFF);           // Contador 16-bit
timer_slave_set_mode(TIM3, TIM_SMCR_SMS_EM3);  // Encoder mode 3 (×4)
timer_ic_set_input(TIM3, TIM_IC1, TIM_IC_IN_TI1);
timer_ic_set_input(TIM3, TIM_IC2, TIM_IC_IN_TI2);
timer_enable_counter(TIM3);
```

| Timer | Modo | Período | Canales |
|-------|------|---------|---------|
| **TIM3** | Encoder EM3 (×4) | 65535 | CH1=PB4, CH2=PB5 |
| **TIM4** | Encoder EM3 (×4) | 65535 | CH1=PB6, CH2=PB7 |

---

## Lectura y Acumulación de Ticks

La lectura de encoders se realiza en [`update_encoder_readings()`](../source_code/src/encoders.c#L143-L179),
ejecutada desde la ISR del SysTick a **10 kHz**.

### Algoritmo de Diferencia con Wrap-Around

La función [`max_likelihood_counter_diff()`](../source_code/src/encoders.c#L124-L133)
calcula la diferencia entre dos lecturas del contador de 16 bits, manejando
correctamente el overflow:

```c
int32_t max_likelihood_counter_diff(uint16_t now, uint16_t before) {
  uint16_t forward_diff  = (uint16_t)(now - before);   // Diferencia "hacia delante"
  uint16_t backward_diff = (uint16_t)(before - now);    // Diferencia "hacia atrás"
  if (forward_diff > backward_diff)
    return -(int32_t)backward_diff;   // El contador decreció
  return (int32_t)forward_diff;       // El contador creció
}
```

**Principio**: si las lecturas son mucho más rápidas que el overflow del
contador, la diferencia más pequeña en valor absoluto es la correcta. Esto
funciona siempre que:

$$ |\Delta ticks| < 32768 \text{ (mitad del rango de 16 bits)} $$

A 10 kHz de muestreo, esto permite hasta ~48 millones de ticks/s, muy por
encima de lo que los motores pueden generar.

### Sentido de Giro

El signo del diff se invierte para el encoder izquierdo:

```c
left_diff_ticks  = -max_likelihood_counter_diff(left_ticks, last_left_ticks);
right_diff_ticks =  max_likelihood_counter_diff(right_ticks, last_right_ticks);
```

Esto significa que los encoders izquierdo y derecho cuentan en direcciones
físicas opuestas, probablemente por la orientación de montaje de los motores en UltiBot.

---

## Cálculo de Velocidad (RPM)

La velocidad instantánea se calcula a partir de los ticks por período de sampleo:

$$ \text{RPM} = \frac{\Delta\text{ticks} \times f_{\text{sampleo}} \times 60}{\text{CPR}} $$

Donde:
- $\Delta\text{ticks}$ = diferencia de ticks en un período de sampleo
- $f_{\text{sampleo}}$ = 10 kHz (frecuencia del SysTick)
- $\text{CPR}$ = 4096 (counts per revolution)

```c
left_speed = (left_diff_ticks * SYSTICK_FREQUENCY_HZ * 60.0) / 4096;
```

---

## Suavizado de Velocidad (Fixed-Point)

La velocidad se suaviza con un **filtro exponencial de punto fijo** para
reducir el ruido de cuantización del encoder:

```c
left_speed <<= 3;                                                    // Shift a fixed-point (×8)
left_speed_smooth_fp = (left_speed_smooth_fp << 4) - left_speed_smooth_fp;  // ×15
left_speed_smooth_fp += left_speed;                                 // + nueva muestra
left_speed_smooth_fp >>= 4;                                         // ÷16
left_speed_smooth_int = left_speed_smooth_fp >> 3;                  // Volver a entero
```

| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| **SPEED_SMOOTHING_FACTOR** | 4 | Factor de suavizado ($2^4 = 16$) |
| **Fixed-point shift** | 3 bits | 3 bits fraccionales (precisión 0.125 RPM) |
| **Filtro equivalente** | $\alpha = 1/16$ | Respuesta paso: 63.2% en 16 muestras (1.6 ms) |

La fórmula del filtro es la media móvil exponencial estándar:

$$ y[n] = \frac{15}{16}y[n-1] + \frac{1}{16}x[n] $$

---

## Posición Absoluta y Pulso de Índice

### Posición Absoluta

[`get_encoder_left_absolute_position()`](../source_code/src/encoders.c#L54-L59)
devuelve la posición dentro de una vuelta eléctrica (0 a `MAX_ABSOLUTE_POSITION`):

```c
int32_t get_encoder_left_absolute_position(void) {
  if (get_encoder_left_total_ticks() >= 0) {
    return get_encoder_left_total_ticks() % (MAX_ABSOLUTE_POSITION + 1);
  } else {
    return (4096 + get_encoder_left_total_ticks()) % (MAX_ABSOLUTE_POSITION + 1);
  }
}
```

> **⚠️ Advertencia**: El offset de 4096 para ticks negativos es cuestionable.
> Si `get_encoder_left_total_ticks()` es menor que -4096, el resultado de
> `(4096 + total)` seguiría siendo negativo, produciendo un módulo negativo.
> En la práctica esto no ocurre porque los totales nunca son tan negativos,
> pero es frágil.

### Pulso de Índice Z

Cada encoder emite un pulso por vuelta mecánica (índice Z). Las ISRs de EXTI
resetean los contadores totales al valor de calibración:

```c
void exti3_isr(void) {
  reset_encoder_left_total_ticks();   // left_total_ticks = 22
  exti_reset_request(EXTI3);
}
void exti15_10_isr(void) {
  reset_encoder_right_total_ticks();  // right_total_ticks = -61
  exti_reset_request(EXTI15);
}
```

| Motor | Offset de calibración |
|-------|----------------------|
| **Izquierdo** | +22 ticks |
| **Derecho** | -61 ticks |

Estos offsets corrigen el desfase entre el pulso de índice físico y el cero
eléctrico del motor. Son valores determinados experimentalmente durante la
calibración. Ver [EG-01](08-known-issues.md#eg-01).

---

*Documento generado el 2026-06-30. Ver también [Hardware](01-hardware.md), [Movimiento](04-movement.md), [Control PID](05-control-system.md).*
