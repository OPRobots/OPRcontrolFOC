# OPRcontrolFOC

Placa de control FOC (Field-Oriented Control) para motores brushless basada en
STM32F103C8 (BluePill). Implementa conmutación sinusoidal con realimentación de
encoders y lazo PI de velocidad. Diseñado como módulo de control de motores
para robots OPRobots, integrado en la PCB de UltiBot.

## Stack tecnológico
- **Microcontrolador**: STM32F103C8T6 (BluePill) @ 72 MHz
- **Framework**: libopencm3
- **Entorno**: PlatformIO
- **Lenguaje**: C11
- **Sensores**: 2× encoder magnético por cuadratura (AS5600 o compatible)
- **Actuadores**: 2× BLDC con driver MOSFET trifásico

## Estructura del repositorio
- `source_code/` — firmware (PlatformIO + libopencm3)
- `pcb_files/` — placeholder (el circuito FOC está integrado en la PCB de UltiBot)
- `3d_model/` — modelos mecánicos (FreeCAD, SketchUp, STL)
- `docs/` — documentación técnica (MkDocs Material)
- `images/` — imágenes del proyecto

## Convenciones de código
- C11 con libopencm3 (bare-metal, sin HAL)
- Comentarios en español e inglés (estilo mixto)
- ISRs en `setup.c` (handlers de interrupción junto a la inicialización)
- Variables compartidas ISR↔main loop declaradas `volatile`
- Formateo con clang-format (`.clang-format` en `source_code/`)

## Documentación
- Usa `/doc-init` para regenerar la documentación desde cero
- Usa `/doc-review` para revisar cambios incrementales y actualizar known issues
- La documentación sigue el estándar OPRobots (MkDocs Material + estilo ZoroBot3)
- Despliegue en GitHub Pages (docs.oprobots.org) vía monorepo OPRobots/docs

## Notas
- No existe PCB independiente — el circuito FOC está integrado en la PCB de
  [UltiBot](https://github.com/OPRobots/UltiBot). Los esquemáticos KiCad están
  en ese repositorio.
- La frecuencia de SYSCLK real es 72 MHz aunque `config.h` declare 84 MHz
  (issue documentado en [HW-01](docs/08-known-issues.md#hw-01)).
- El control es solo PI (término D comentado). Ver [CT-01](docs/08-known-issues.md#ct-01).
- SimpleFOC está listado como dependencia en `.pio/libdeps/` pero el código no
  usa sus clases directamente — la implementación de FOC es propia.
- Los offsets de encoder (+22, -61) son específicos del montaje en UltiBot.
  Para usar esta placa en otro robot, recalibrar según [EG-01](docs/08-known-issues.md#eg-01).
