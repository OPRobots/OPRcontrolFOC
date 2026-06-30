# Movimiento

## Principio de Funcionamiento

OPRcontrolFOC implementa **conmutación sinusoidal** para motores BLDC, una
técnica de Field-Oriented Control (FOC) simplificada que genera tres señales
PWM senoidales desfasadas 120° entre sí.

A diferencia del control trapezoidal (6-step), la conmutación sinusoidal produce
un campo magnético giratorio suave, eliminando el ripple de par y reduciendo
el ruido audible.

---

## Modos de Operación

| Modo | Función | Realimentación |
|------|---------|----------------|
| **Closed-loop** | `motors_move()` | Encoder de posición (TIM3/TIM4) |
| **Open-loop (debug)** | `debug_motors_move_open_loop()` | Ninguna |

### Modo Closed-Loop

`motors_move()` ejecuta el lazo completo a ~1 kHz desde el main loop:

1. **PID de velocidad** — calcula `motor_left_speed_factor` y `motor_right_speed_factor` (0-100%).
2. **Lectura de posición** — obtiene la posición absoluta del encoder dentro de la vuelta eléctrica.
3. **Cálculo de fase** — convierte la posición del encoder en un ángulo eléctrico (0-360°).
4. **Offset direccional** — añade +90° o +270° según el signo de la velocidad.
5. **Modulación PWM** — indexa la LUT de seno y escala la amplitud según `speed_factor`.

### Modo Open-Loop

[`debug_motors_move_open_loop()`](../source_code/src/motors.c#L197-L235) hace
avanzar la fase eléctrica incrementalmente (1 paso por llamada) sin leer el
encoder. Útil para verificar el cableado y la respuesta del motor sin lazo de
control.

---

## Lookup Table de Seno

La LUT de seno [`sine_lookup[360]`](../source_code/src/motors.c#L28-L37) se
genera al arrancar en `motors_init()`:

```c
for (uint16_t i = 0; i < 360; i++) {
  value = sin((i / 360.0f) * 2 * M_PI);
  value *= PWM_PERIOD / 2.0f;    // Escalar a amplitud PWM
  value += PWM_PERIOD / 2.0f;    // Offset DC (centrado en mitad de período)
  value = abs(round(value));
  sine_lookup[i] = (uint16_t)value;
}
```

| Parámetro | Valor |
|-----------|-------|
| **Resolución** | 360 entradas (1° por entrada) |
| **Período PWM** | 512 |
| **Amplitud pico** | ±256 (centrado en 256) |
| **Tipo de dato** | `uint16_t` |

Cada entrada representa el duty cycle PWM para ese ángulo eléctrico, con offset
DC para centrar la sinusoide en la mitad del rango PWM.

La fórmula equivale a:

$$ \text{LUT}[i] = \left| \text{round}\left( \frac{\text{PWM\_PERIOD}}{2} \cdot \sin\left(\frac{2\pi i}{360}\right) + \frac{\text{PWM\_PERIOD}}{2} \right) \right| $$

---

## Cálculo de Fase Eléctrica

La posición del encoder se convierte en ángulo eléctrico mediante la función
[`map()`](../source_code/src/utils.c#L3-L5):

```c
// Motor izquierdo, velocidad positiva
motor_left_A = map(get_encoder_left_absolute_position(),
                   0, MAX_ABSOLUTE_POSITION, 0, 360) + 270;
motor_left_A = motor_left_A % 360;
```

| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| **MAX_ABSOLUTE_POSITION** | 585 | Ticks del encoder por vuelta eléctrica |
| **Rango mapeado** | 0–360 | Grados eléctricos |
| **Offset positivo** | +90° o +270° | Adelanto de fase para girar en un sentido |
| **Offset negativo** | +270° o +90° | Adelanto de fase para girar en sentido opuesto |

La relación entre posición mecánica y eléctrica es:

$$ \theta_{elec} = \left( \frac{\text{pos\_abs} \times 360}{\text{MAX\_ABSOLUTE\_POSITION}} + \text{offset} \right) \bmod 360 $$

Donde el offset es +90° o +270° dependiendo del motor (izquierdo/derecho) y del
sentido de giro. Este offset asegura que el campo magnético del estator esté
siempre adelantado 90° respecto al rotor para máximo par.

---

## Asignación de Fases PWM

Cada motor usa un timer de 3 canales para generar las 3 fases senoidales:

### Motor Izquierdo (TIM2)

| Canal | Pin | Fase |
|-------|-----|------|
| **TIM2_CH1** (CCR1) | PA0 | Fase C |
| **TIM2_CH2** (CCR2) | PA1 | Fase B |
| **TIM2_CH3** (CCR3) | PA2 | Fase A |

### Motor Derecho (TIM1)

| Canal | Pin | Fase |
|-------|-----|------|
| **TIM1_CH1** (CCR1) | PA8 | Fase A |
| **TIM1_CH2** (CCR2) | PA9 | Fase C |
| **TIM1_CH3** (CCR3) | PA10 | Fase B |

> **Nota**: El orden de fases es diferente entre el motor izquierdo (C→B→A) y
> el derecho (A→C→B). Esto es intencionado y refleja la orientación física de
> los motores en el robot UltiBot. Ver [MV-02](08-known-issues.md#mv-02).

---

## Duty Cycle y Speed Factor

La amplitud de la sinusoide se escala según el `speed_factor` (0-100) calculado
por el PID:

```c
TIM_CCR1(TIM2) = sine_lookup[motor_left_C] * (abs(motor_left_speed_factor) / 100.0f);
TIM_CCR2(TIM2) = sine_lookup[motor_left_B] * (abs(motor_left_speed_factor) / 100.0f);
TIM_CCR3(TIM2) = sine_lookup[motor_left_A] * (abs(motor_left_speed_factor) / 100.0f);
```

- **speed_factor = 0**: motor parado (PWM con ciclo fijo centrado, sin alternancia)
- **speed_factor = 100**: máxima amplitud (PWM senoidal completa)
- La velocidad objetivo determina la frecuencia de la sinusoide; el speed factor
  determina su amplitud (y por tanto el par disponible).

Cuando `motor_speed == 0`, los canales PWM se mantienen en el valor de la LUT
sin alternancia (campo magnético estático, freno suave).

---

## Frecuencia PWM

| Timer | Prescaler | Período | Frecuencia PWM |
|-------|-----------|---------|----------------|
| **TIM1** | 5 (div 5) | 512 | $$f = \frac{72\text{ MHz}}{5 \times 512} \approx 28.125\text{ kHz}$$ |
| **TIM2** | 5 (div 5) | 512 | $$f = \frac{72\text{ MHz}}{5 \times 512} \approx 28.125\text{ kHz}$$ |

---

*Documento generado el 2026-06-30. Ver también [Control PID](05-control-system.md), [Encoders](06-encoders-gyro.md), [Hardware](01-hardware.md).*
