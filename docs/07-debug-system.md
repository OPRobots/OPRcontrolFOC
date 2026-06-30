# Debug

## Visión General

El sistema de debug de OPRcontrolFOC es minimalista y está orientado al
desarrollo y pruebas de bajo nivel. Consta de tres herramientas:

| Herramienta | Propósito |
|-------------|-----------|
| **LEDs onboard (PC13, PC14)** | Indicación visual de estado y saturación del PID |
| **USART3 printf** | Salida de depuración serie a través de `printf()` |
| **Open-loop move** | Función de prueba para verificar hardware sin lazo de control |
| **Comandos serie** | Control manual de motores para pruebas |

---

## LEDs de Estado

La BluePill dispone de dos LEDs en GPIOC que se usan como indicadores de
saturación del PID:

| LED | Pin | Indicación |
|-----|-----|------------|
| **LED1** | PC14 | Speed factor motor izquierdo > 95% |
| **LED2** | PC13 | Speed factor motor derecho > 95% |

```c
if (motor_left_speed_factor > 95) {
    gpio_set(GPIOC, GPIO14);     // LED ON = saturando
} else {
    gpio_clear(GPIOC, GPIO14);   // LED OFF
}
```

### Interpretación

| Estado LED | Significado |
|------------|-------------|
| **Apagado** | Motor dentro de rango normal de operación |
| **Encendido** | PID ha saturado — el motor no alcanza la velocidad objetivo |
| **Parpadeo** | El motor oscila cerca del límite de saturación |
| **Siempre ON** | Velocidad objetivo demasiado alta para la tensión de batería |

---

## Salida Serie (printf)

La salida estándar se redirige a USART3 mediante la reimplementación de
`_write()`. Esto permite usar `printf()` normalmente para depurar:

```c
// Ejemplos de uso (actualmente comentados en el código)
printf("OK> %c: %d\n", command, value);          // Confirmación de comando
printf("ERR> %c: %d\n", command, value);          // Error de comando
printf("%d %d %d\n", sine_lookup[A], sine_lookup[B], sine_lookup[C]);  // Valores LUT
printf("%d - %d %d %d\n", angle, phase_A, phase_B, phase_C);  // Fases
```

La mayoría de las llamadas a `printf()` en
[`motors.c`](../source_code/src/motors.c#L165-L166) y
[`command.c`](../source_code/src/command.c#L4) están **comentadas** para no
interferir con el control en tiempo real. La transmisión serie es bloqueante
(`usart_send_blocking`) y puede afectar al timing del lazo de control.

> **💡 Consejo**: Para depurar, descomenta los `printf()` uno a uno. No actives
> varios a la vez — la latencia de transmisión a 115200 baud puede superar el
> período de control de 1 ms.

---

## Modo Open-Loop

[`debug_motors_move_open_loop()`](../source_code/src/motors.c#L197-L235) hace
avanzar la conmutación sinusoidal incrementalmente **sin leer el encoder** ni
ejecutar el PID. Cada llamada avanza 1 grado eléctrico y aplica un delay de 1 ms.

### Propósito

- Verificar el cableado de las fases del motor
- Confirmar que el motor gira suavemente
- Diagnosticar problemas de hardware sin la influencia del lazo de control
- Probar el correcto funcionamiento de los timers PWM

### Uso

Llamar desde `main()` en lugar de `motors_move()` tras la inicialización:

```c
// En main():
while (1) {
    debug_motors_move_open_loop();
}
```

A 1 grado por ms, el motor da una vuelta eléctrica completa en 360 ms.

---

## Comandos de Depuración por Serie

Los comandos serie (ver [Comunicaciones](03-communications.md)) permiten control
manual para pruebas:

```bash
# Desde un terminal serie (115200 8N1)
E        # Habilitar motores
L500     # Motor izquierdo a 500 RPM
R-300    # Motor derecho a -300 RPM (invertido)
L0       # Parar motor izquierdo
R0       # Parar motor derecho
D        # Deshabilitar motores
```

---

## Puntos de Mejora

- **Telemetría continua**: Implementar un modo de streaming que envíe datos
  (RPM, speed factor, posición) periódicamente sin saturar el lazo de control.
- **Logging de errores**: El `TODO` en [`command.c:19`](../source_code/src/command.c#L19)
  indica que se planea notificar comandos inválidos pero no está implementado.
- **Canales adicionales**: USART1/USART2 están disponibles en la BluePill y
  podrían usarse para un canal de debug independiente, liberando USART3 para
  comandos de control.
- **DMA para printf**: Usar DMA para transmisión serie eliminaría el bloqueo de
  `usart_send_blocking()` durante el debug.

---

*Documento generado el 2026-06-30. Ver también [Comunicaciones](03-communications.md), [Control PID](05-control-system.md), [Arquitectura Software](02-software-architecture.md).*
