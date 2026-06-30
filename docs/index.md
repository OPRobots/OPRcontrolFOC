# OPRcontrolFOC

Placa de control FOC (Field-Oriented Control) para motores brushless.
Implementa conmutación sinusoidal con realimentación de encoders magnéticos
y lazo PI de velocidad independiente para cada motor, usando un STM32F103C8
con libopencm3.

Diseñado como módulo de control de motores para robots OPRobots. El circuito
FOC está integrado directamente en la PCB de [UltiBot](https://github.com/OPRobots/UltiBot).

---

## ⚙️ Hardware

| Característica | Detalle |
|---------------|---------|
| **Microcontrolador** | STM32F103C8T6 (BluePill) @ 72 MHz |
| **Motores** | 2× BLDC controlados por conmutación sinusoidal |
| **Drivers** | MOSFET trifásico (TIM1 motor derecho, TIM2 motor izquierdo) |
| **Encoders** | 2× encoder magnético por cuadratura (4096 CPR) con índice Z |
| **PWM** | ~28.125 kHz, 3 fases por motor |
| **Comunicación** | USART3 @ 115200 baud 8N1 |
| **PCB** | Integrada en [UltiBot](https://github.com/OPRobots/UltiBot) |

---

## 💻 Software

| Componente | Detalle |
|-----------|---------|
| **Framework** | libopencm3 (bare-metal, sin RTOS) |
| **Entorno** | PlatformIO |
| **Frecuencia de control** | PID a ~1 kHz, lectura de encoders a 10 kHz |
| **Algoritmo** | PI velocidad + conmutación sinusoidal con LUT |
| **Sintonía PID** | Kp=8.0, Ki=0.1, Kd=0.0 |

---

## 📚 Documentación

- [Hardware](01-hardware.md) — MCU, pinout completo, timers, periféricos, diagrama de bloques
- [Arquitectura Software](02-software-architecture.md) — main loop, ISRs, flujo de ejecución
- [Comunicaciones](03-communications.md) — protocolo serie de comandos (USART3)
- [Movimiento](04-movement.md) — conmutación sinusoidal, LUT de seno, modulación PWM
- [Control PID](05-control-system.md) — lazo PI de velocidad, anti-windup, indicadores LED
- [Encoders](06-encoders-gyro.md) — encoders de cuadratura, cálculo RPM, suavizado
- [Debug](07-debug-system.md) — LEDs de estado, printf serie, modo open-loop
- [Problemas Conocidos](08-known-issues.md) — 12 issues documentados con diagnóstico y soluciones

---

## 🔧 Stack Tecnológico

| Capa | Tecnología |
|------|-----------|
| **MCU** | STM32F103C8T6 (ARM Cortex-M3, 72 MHz) |
| **Framework** | libopencm3 |
| **Build** | PlatformIO |
| **Lenguaje** | C11 |
| **Docs** | MkDocs Material + KaTeX + Mermaid |

---

*Documento generado el 2026-06-30. Ver también [Hardware](01-hardware.md).*
