# OPRcontrolFOC

Placa de control FOC (Field-Oriented Control) para motores brushless basada en
STM32F103C8. Implementa conmutación sinusoidal para 2 BLDC con lazo PI de
velocidad y realimentación por encoders magnéticos.

> **Documentación completa**: [docs.oprobots.org/boards/OPRcontrolFOC](https://docs.oprobots.org/boards/OPRcontrolFOC)

---

## ⚙️ Hardware

| Componente | Detalle |
|-----------|---------|
| **Microcontrolador** | STM32F103C8T6 (BluePill) @ 72 MHz |
| **Motores** | 2× BLDC con driver MOSFET trifásico |
| **Control** | Conmutación sinusoidal (LUT 360 entradas, PWM ~28 kHz) |
| **Encoders** | 2× encoder magnético cuadratura (4096 CPR) + índice Z |
| **Comunicación** | USART3 @ 115200 (comandos serie) |
| **PCB** | Integrada en [UltiBot](https://github.com/OPRobots/UltiBot) |

---

## 💻 Software

| Componente | Detalle |
|-----------|---------|
| **Framework** | libopencm3 (bare-metal) |
| **Entorno** | PlatformIO |
| **Frecuencia control** | ~1 kHz (PID), 10 kHz (lectura encoders) |
| **Control** | PI velocidad (Kp=8.0, Ki=0.1) |
| **Debug** | LEDs onboard (PC13/PC14) + USART3 printf |

---

## 📚 Documentación

- [Hardware](docs/01-hardware.md) — MCU, pinout, timers, periféricos
- [Arquitectura Software](docs/02-software-architecture.md) — main loop, ISRs, flujo de ejecución
- [Comunicaciones](docs/03-communications.md) — protocolo serie de comandos
- [Movimiento](docs/04-movement.md) — conmutación sinusoidal y modulación PWM
- [Control PID](docs/05-control-system.md) — lazo PI de velocidad y anti-windup
- [Encoders](docs/06-encoders-gyro.md) — encoders de cuadratura y cálculo de velocidad
- [Debug](docs/07-debug-system.md) — LEDs, printf, open-loop mode
- [Problemas Conocidos](docs/08-known-issues.md) — ~12 issues documentados

---

## 🔧 Stack Tecnológico

| Capa | Tecnología |
|------|-----------|
| **MCU** | STM32F103C8T6 (ARM Cortex-M3) |
| **Framework** | libopencm3 |
| **Build** | PlatformIO |
| **Lenguaje** | C11 |
| **Docs** | MkDocs Material |

---

*Documentación generada con `/doc-init`. Estilo OPRobots.*
