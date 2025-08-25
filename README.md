# Aeroq

<img width="2268" height="1181" alt="image" src="https://github.com/user-attachments/assets/4e70900d-991c-4c67-8099-969e4073f7d7" />

### A comprehensive indoor air quality monitor, capturing Temperature, Relative Humidity, CO2 (ppm), VOC (Index), and PM 1.0, 2.5, 4.0, 10 (ug/m3)

## Contents
- [Project Overview](#project-overview)
- [Required Hardware](#required-hardware)
- [Firmware Setup](#firmware-setup)
- [Assembly](#assembly)
- [Using Aeroq](#using-aeroq)

---
## Project Overview

# Hardware
Schematic and PCB designed using KiCad. Schematic, PCB design, and manufacturing files can be found under [Aeroq Hardware/v3/](Aeroq Hardware/v3/)

# Firmware
Source code for ESP32 firmware, developed with PlatformIO. Can be found under [Aeroq Firmware/](Aeroq Firmware/)

# Enclosure
Two 3D printed components are required for the assembly. Model files can be found under [Aeroq Enclosure/](Aeroq Enclosure/)

---

## Required Hardware  

| Item | Link |
|------|------|
| Custom PCB Order | [PCBWay](https://www.pcbway.com/project/shareproject/Air_Quality_Monitor_a6b051c5.html) |
| SEN54 (VOC/PM) Sensor | [Sensirion](https://sensirion.com/products/catalog/SEN54) |
| Enclosure 3D Print Files | [Printables](https://www.printables.com/model/1394330-indoor-air-quality-monitor) |
| 3.7" 240x416 E-Paper Display | [Elecrow](https://www.elecrow.com/3-7-inch-240-416-e-paper-display-black-white-e-ink-display-with-spi-communication.html?srsltid=AfmBOorSii7sAGDAezYqj44_16daoAiQB-C5RoZuXMNfL8XauDDESF2X) |
| JST GH Connector Kit | [Amazon](https://amzn.to/4lGmWBa) |

---

## Firmware Setup

This project uses PlatformIO, a plugin for VSCode that allows for a streamlined ESP32 development process. Outlined are the steps for flashing firmware to the ESP32-C3-MINI...

---

## Assembly

### 1. ...
### 2. ...
### 3. ...

---

## Using Aeroq

Simply plug the device in using any USB-C cable/power supply (minimum 5V 1A), and wait for the initialization to complete.  
**For optimal measurement accuracy, keep it away from heat sources (electronics, lights, etc) and in an area with minimal clutter.**

## License

This project (hardware, design files, and firmware) is licensed under the 
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-nc-sa/4.0/).

[![License: CC BY-NC-SA 4.0](https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png)](http://creativecommons.org/licenses/by-nc-sa/4.0/)
