# Problemas Conocidos

> **Fecha de análisis**: 2026-06-30
> **Última actualización**: 2026-06-30
> **Total**: 13 issues — 1 crítico, 4 moderados, 8 leves

---

## Resumen por Prioridades

| Prioridad | Cantidad | Issues |
|-----------|:--------:|--------|
| 🔴 Críticos | 1 | SW-01 |
| 🟡 Moderados | 4 | HW-01, HW-03, CT-01, CT-02, EG-02 |
| 🟢 Leves | 8 | HW-02, SW-02, SW-03, EG-01, MV-01, MV-02, MV-03 |
| **TOTAL** | **13** | |

---

## Correcciones Recientes ✅

| ID | Fecha | Descripción |
|----|-------|-------------|
| — | — | — |

---

## 🔴 Críticos

### Software / Arquitectura

<a id="sw-01"></a>
#### SW-01 — Buffer overflow en ISR de USART

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/setup.c#L98"><code>setup.c:98</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>El buffer de comandos <code>static char command[8]</code> se llena sin comprobación de límites en <code>command[i++]</code>. Si se envían más de 7 caracteres antes del <code>\n</code>, el buffer desborda y corrompe la pila (variables locales, dirección de retorno de la ISR).</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Alto</strong> — Puede causar crashes, comportamiento errático, o vulnerabilidad de seguridad.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna. Se asume que el host no envía comandos largos.</td></tr>
<tr><td><strong>Solución</strong></td><td>Añadir <code>if (i &lt; sizeof(command) - 1)</code> antes de almacenar cada carácter, o implementar un timeout que descarte comandos incompletos.</td></tr>
</table>

---

## 🟡 Moderados

### Hardware

<a id="hw-01"></a>
#### HW-01 — Discrepancia SYSCLK: 84 MHz vs 72 MHz real

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/include/config.h#L7"><code>config.h:7</code></a>, <a href="../source_code/src/setup.c#L13"><code>setup.c:13</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td><code>config.h</code> declara <code>SYSCLK_FREQUENCY_HZ 84000000</code> y <code>CYCLE_COUNTER_PER_MICROSECONDS 84</code>, pero <code>setup_clock()</code> llama a <code>rcc_clock_setup_in_hse_8mhz_out_72mhz()</code> que configura el PLL a 72 MHz. Como resultado, <code>delay_us()</code> calcula mal los ciclos de espera: usa 84 ciclos/µs cuando debería usar 72. Los delays en microsegundos son un ~14% más cortos de lo esperado.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Medio</strong> — Timing incorrecto en <code>delay_us()</code>. Las frecuencias de PWM y encoder se derivan del reloj real (72 MHz), por lo que los valores documentados en código (basados en 84 MHz) no coinciden.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna. El código funciona porque 72 MHz es la frecuencia real.</td></tr>
<tr><td><strong>Solución</strong></td><td>Cambiar <code>SYSCLK_FREQUENCY_HZ</code> a <code>72000000</code> y <code>CYCLE_COUNTER_PER_MICROSECONDS</code> a <code>72</code>.</td></tr>
</table>

<a id="hw-03"></a>
#### HW-03 — Timer prescaler documentado para frecuencia incorrecta

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/setup.c#L152"><code>setup.c:152</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>El prescaler de TIM1/TIM2 es 4 (división por 5). Con 72 MHz reales, la frecuencia PWM es 72 MHz / 5 / 512 = 28.125 kHz. Con 84 MHz (como declara <code>config.h</code>) sería 32.8 kHz. La documentación del código no aclara qué frecuencia se pretendía. 28 kHz es adecuado para motores BLDC pequeños (por encima del rango audible).</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Medio</strong> — La frecuencia real de PWM es correcta, pero la discrepancia con la documentación puede confundir.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna necesaria.</td></tr>
<tr><td><strong>Solución</strong></td><td>Documentar la frecuencia real de PWM y ajustar las constantes de <code>config.h</code> para que coincidan.</td></tr>
</table>

### Control PID

<a id="ct-01"></a>
#### CT-01 — Término derivativo del PID comentado (solo PI)

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/motors.c#L65"><code>motors.c:65</code></a>, <a href="../source_code/src/motors.c#L86"><code>motors.c:86</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td><code>MOTOR_SPEED_KD</code> está definido como 0.0 y el cálculo del término derivativo <code>(left_error - motor_left_last_error) * MOTOR_SPEED_KD</code> está comentado. El controlador opera como PI en lugar de PID. En control de velocidad de motores, el término D puede ayudar a amortiguar oscilaciones y mejorar la respuesta a cambios bruscos de carga.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Medio</strong> — Respuesta transitoria subóptima. Posibles overshoots en cambios bruscos de consigna.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>El término integral proporciona cierta capacidad de reacción, pero sin amortiguamiento.</td></tr>
<tr><td><strong>Solución</strong></td><td>Experimentar con KD bajo (0.5-2.0) y evaluar si mejora la respuesta sin amplificar ruido del encoder.</td></tr>
</table>

<a id="ct-02"></a>
#### CT-02 — Anti-windup asimétrico: integrador sin límite superior

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/motors.c#L75-L80"><code>motors.c:75-80</code></a>, <a href="../source_code/src/motors.c#L96-L101"><code>motors.c:96-101</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>La condición <code>if (motor_left_speed_factor &lt; 100 || left_error &lt; 0)</code> permite acumular error integral cuando <code>speed_factor == 100</code> y <code>error &lt; 0</code> (el motor va más rápido de lo pedido). Esto es correcto para reducir la integral cuando se necesita frenar. Pero cuando <code>speed_factor == 100</code> y <code>error &gt; 0</code> se deja de acumular — correcto. Sin embargo, el integrador no tiene límite superior (solo inferior en 0) ni back-calculation, así que si se acumula un valor muy alto durante una aceleración, tarda en bajar cuando se reduce la consigna.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Medio</strong> — Posible overshoot al reducir bruscamente la velocidad objetivo desde máxima velocidad.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>El clamping inferior (nunca &lt; 0) evita windup negativo.</td></tr>
<tr><td><strong>Solución</strong></td><td>Añadir clamping superior al integrador (ej. <code>100 / MOTOR_SPEED_KI</code>) correspondiente al máximo speed factor alcanzable.</td></tr>
</table>

### Encoders

<a id="eg-02"></a>
#### EG-02 — Posible overflow en acumulador fixed-point del filtro de velocidad

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/encoders.c#L164-L167"><code>encoders.c:164-167</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>La variable <code>left_speed_smooth_fp</code> (int32_t) acumula valores en formato fixed-point con el filtro IIR: <code>speed_smooth_fp = (speed_smooth_fp &lt;&lt; 4) - speed_smooth_fp + speed</code>. Con 3 bits fraccionales, el rango efectivo es ±2²⁸ ≈ ±268M ticks/s. Aunque improbable en condiciones normales, con ruido extremo o spikes de encoder el acumulador podría desbordar silenciosamente.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Medio</strong> — Overflow silencioso causaría lecturas de velocidad incorrectas. Poco probable en operación normal.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Los valores reales de velocidad están muy por debajo del límite de overflow (MAX_RPM = 900 genera ~150 ticks/diff).</td></tr>
<tr><td><strong>Solución</strong></td><td>Añadir clamping del valor filtrado al rango esperado o usar <code>int64_t</code> para el acumulador fixed-point.</td></tr>
</table>

---

## 🟢 Leves

### Hardware

<a id="hw-02"></a>
#### HW-02 — Includes ADC/DMA no utilizados

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/include/setup.h#L7-L8"><code>setup.h:7-8</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td><code>setup.h</code> incluye <code>&lt;libopencm3/stm32/adc.h&gt;</code> y <code>&lt;libopencm3/stm32/dma.h&gt;</code>, pero no se configura ningún ADC ni DMA en <code>setup.c</code>. Código muerto que probablemente se añadió para una funcionalidad futura (sensor de corriente o tensión de batería).</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Bajo</strong> — Solo afecta al tiempo de compilación y claridad del código.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna necesaria.</td></tr>
<tr><td><strong>Solución</strong></td><td>Eliminar los includes o implementar la lectura de ADC.</td></tr>
</table>

### Software / Arquitectura

<a id="sw-02"></a>
#### SW-02 — `motor_right_inited` no declarado volatile

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/motors.c#L23"><code>motors.c:23</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td><code>motor_right_inited</code> se modifica en <code>set_motor_right_inited()</code>, llamada desde la ISR <code>exti15_10_isr()</code>. Sin embargo, no está declarado <code>volatile</code>, a diferencia de otras variables como <code>motor_left_speed</code>. El compilador podría optimizar lecturas si el flag llegara a usarse en el main loop.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Bajo</strong> — Actualmente el flag no se lee desde el main loop (ver SW-03). Si se implementara su lectura, podría fallar sin <code>volatile</code>.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>El flag no se usa, así que el bug está latente.</td></tr>
<tr><td><strong>Solución</strong></td><td>Declarar <code>static volatile int motor_right_inited</code>.</td></tr>
</table>

<a id="sw-03"></a>
#### SW-03 — Flags de inicialización por motor nunca consultados

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/motors.c#L114-L120"><code>motors.c:114-120</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td><code>set_motor_left_inited()</code> y <code>set_motor_right_inited()</code> son llamadas desde las ISRs de índice Z para marcar cada motor como inicializado. Sin embargo, estos flags (<code>motor_left_inited</code>, <code>motor_right_inited</code>) no se leen en ninguna parte del código. <code>motors_move()</code> solo comprueba el flag global <code>inited</code>.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Bajo</strong> — Funcionalidad incompleta: no se puede saber si un encoder ha recibido su pulso de índice. Permitiría distinguir entre "encoder sin referencia de cero" y "encoder listo".</td></tr>
<tr><td><strong>Mitigación</strong></td><td>El flag global <code>inited</code> es suficiente para la operación actual.</td></tr>
<tr><td><strong>Solución</strong></td><td>Usar los flags para condicionar el modo de operación (open-loop hasta recibir índice, closed-loop después) o eliminar el código muerto.</td></tr>
</table>

### Encoders

<a id="eg-01"></a>
#### EG-01 — Offsets de calibración hardcodeados sin documentación

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/encoders.c#L71"><code>encoders.c:71</code></a>, <a href="../source_code/src/encoders.c#L77"><code>encoders.c:77</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>Los valores <code>left_total_ticks = 22</code> y <code>right_total_ticks = -61</code> son offsets de calibración que corrigen la diferencia angular entre el pulso de índice Z y el cero eléctrico del motor. Estos valores son específicos del montaje mecánico de cada motor en UltiBot y no son reutilizables. No hay documentación sobre cómo se obtuvieron ni cómo recalibrar para otro robot.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Bajo</strong> — La funcionalidad es correcta para el hardware actual, pero la mantenibilidad es deficiente.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna.</td></tr>
<tr><td><strong>Solución</strong></td><td>Mover los offsets a <code>#define</code>s con nombres descriptivos en <code>config.h</code> (ej. <code>ENCODER_LEFT_INDEX_OFFSET</code>, <code>ENCODER_RIGHT_INDEX_OFFSET</code>) y documentar el procedimiento de calibración.</td></tr>
</table>

### Movimiento

<a id="mv-01"></a>
#### MV-01 — Uso de `round()` (doble precisión) en lugar de `roundf()`

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/motors.c#L34"><code>motors.c:34</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>En <code>fill_lookup()</code>, se usa <code>abs(round(value))</code> donde <code>round()</code> opera en <code>double</code>. En un Cortex-M3 sin FPU de doble precisión, esto implica emulación por software, añadiendo latencia significativa durante la inicialización. La LUT se genera una sola vez al arrancar.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Bajo</strong> — Solo afecta al tiempo de inicialización (~360 llamadas a <code>round()</code>).</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna. La inicialización ocurre una sola vez.</td></tr>
<tr><td><strong>Solución</strong></td><td>Cambiar a <code>roundf()</code> (float) o precalcular la LUT offline e incluirla como array constante en el código.</td></tr>
</table>

<a id="mv-02"></a>
#### MV-02 — Orden de fases asimétrico entre motores sin documentar

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/motors.c#L161-L163"><code>motors.c:161-163</code></a>, <a href="../source_code/src/motors.c#L184-L186"><code>motors.c:184-186</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td>El motor izquierdo asigna fases en orden C→B→A a los canales PWM (CCR1/CCR2/CCR3), mientras que el motor derecho usa A→C→B. Además, los offsets de fase para velocidad positiva son +270° (izquierdo) y +90° (derecho). Esta asimetría es intencionada por la orientación física de los motores en UltiBot, pero no está documentada ni comentada en el código.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Bajo</strong> — Si se reutiliza este código con otra orientación de motores, el sentido de giro será incorrecto o el motor no arrancará.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna.</td></tr>
<tr><td><strong>Solución</strong></td><td>Añadir comentarios explicando la orientación esperada y la relación entre orden de fases y sentido de giro.</td></tr>
</table>

<a id="mv-03"></a>
#### MV-03 — `motor_right_speed` inconsistente en tipo vs `motor_left_speed`

<table>
<tr><td><strong>Archivos</strong></td><td><a href="../source_code/src/motors.c#L8"><code>motors.c:8</code></a>, <a href="../source_code/src/motors.c#L19"><code>motors.c:19</code></a></td></tr>
<tr><td><strong>Descripción</strong></td><td><code>motor_left_speed</code> está declarado como <code>static volatile int</code>, mientras que <code>motor_right_speed</code> es <code>static volatile int</code>. Ambos son correctos, pero <code>motor_right_inited</code> es <code>static int</code> (no volatile, ver SW-02). La inconsistencia sugiere que los calificadores se aplicaron sin criterio uniforme.</td></tr>
<tr><td><strong>Impacto</strong></td><td><strong>Bajo</strong> — No causa bugs actualmente, pero indica falta de revisión sistemática.</td></tr>
<tr><td><strong>Mitigación</strong></td><td>Ninguna.</td></tr>
<tr><td><strong>Solución</strong></td><td>Auditar y unificar los calificadores de todas las variables compartidas con ISRs.</td></tr>
</table>

---

*Documento generado el 2026-06-30 tras auditoría exhaustiva del código fuente. Ver también el resto de secciones de la documentación para contexto de cada subsistema.*
