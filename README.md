# DrawBot — Low-Cost 3D-Printed CNC Plotter (Open-Source)

This repository is an open-source recreation of a simple **CNC plotting machine** aligned with a training project theme:
a low-profile, economical plotter made with 3D-printed parts and Arduino-based control.

> ⚠️ Disclaimer: These files are community/open-source friendly stand-ins so you can showcase your internship work.
They are compatible with hobby hardware and meant for learning and portfolio use, not production.

## Features
- X/Y/Z motion using **28BYJ-48** stepper motors + **ULN2003** driver boards
- Simple **G-code** serial interpreter (supports `G0/G1` with `X Y Z F`, `M3/M5` pen up/down)
- **Pen plotter** style output; easy to adapt for small extruder/laser (do so at your own risk)
- Minimal **STL** mechanical placeholders for mounting and testing

## Repo Structure
```
cad/        -> simple placeholder STLs (base_plate, bracket, pen_holder)
firmware/   -> Arduino sketch: DrawBot_28BYJ_GCode.ino
gcode/      -> sample G-code files (square_40mm.gcode, HelloPath.gcode)
images/     -> preview render / schematic
docs/       -> notes and wiring diagram text
README.md
LICENSE
```

## Hardware (example)
- 3 × 28BYJ-48 5V stepper motors
- 3 × ULN2003 driver boards
- 1 × Arduino UNO
- 1 × Micro servo (SG90) for pen up/down
- 5V 2A power supply
- GT2 belts/rod guides (or simple printed sliders for testing)

## Wiring (summary)
- X motor coils → ULN2003 #1 → Arduino pins 2–5
- Y motor coils → ULN2003 #2 → Arduino pins 6–9
- Z motor coils → ULN2003 #3 → Arduino pins 10–13
- Servo (pen) signal → Arduino pin 3, +5V, GND
- Common GND between motor PSU and Arduino

See `docs/wiring.txt` for exact mapping.

## Build & Flash
1. Open `firmware/DrawBot_28BYJ_GCode.ino` in Arduino IDE.
2. Board: **Arduino UNO**, Port: your COM port.
3. Install **Servo** (built-in) library if needed.
4. Upload.
5. Open Serial Monitor @ **115200 baud**, newline line-ending.

## Talk G-code
Example (paste line-by-line into Serial Monitor or use a sender app):
```
G0 X0 Y0 Z0 F1000
M3      ; pen down
G1 X40 Y0 F800
G1 X40 Y40
G1 X0  Y40
G1 X0  Y0
M5      ; pen up
```

## Notes
- Steps/mm are rough defaults; tune `STEPS_PER_MM_X/Y/Z` in the sketch.
- 28BYJ-48 have internal gearing; motion is slow but precise enough for plotting.
- Add endstops & homing for safety if you extend this project.

## License
MIT — see `LICENSE`.
