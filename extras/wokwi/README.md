# Wokwi diagrams for the examples

[Wokwi](https://wokwi.com) simulates an ESP32 together with HC-SR04 modules in the
browser, so the examples of this library can be tried out without any hardware. The two
diagrams here cover all five examples, because they share their wiring.

| Diagram | Wiring | Examples |
| --- | --- | --- |
| `diagram-1-sensor.json` | trigger on GPIO 21, echo on GPIO 12 | `HC-SR04_Simple`, `HC-SR04_Temperature` |
| `diagram-2-sensoren.json` | trigger on GPIO 21, echos on GPIO 12 and 13 | `HC-SR04_Multiple`, `HC-SR04_Time`, `HC-SR04_Units` |

## How to use them

1. Start a new ESP32 project on [wokwi.com](https://wokwi.com/projects/new/esp32).
2. Paste the example into `sketch.ino` and the matching diagram into the `diagram.json`
   tab.
3. Add this library through the library manager of the project, which writes the
   `libraries.txt` next to these diagrams.
4. Start the simulation and open the serial monitor. The distance of a module can be
   changed while it runs by clicking it.

## Notes

The ESP32 reads its boot configuration from GPIO 12, so a module that holds its echo
line high during a reset keeps the board from starting. In the simulation that never
happens, on real hardware a different echo pin is the safer choice.

A real HC-SR04 answers with 5 V, which an ESP32 input does not tolerate. The simulation
ignores that, a voltage divider on the echo line does not hurt in reality.
