# Aeroq

<img width="2268" height="1181" alt="image" src="https://github.com/user-attachments/assets/a664637c-8e82-404b-834a-4a762beba134" />

### A comprehensive indoor air quality monitor, capturing Temperature, Relative Humidity, CO2 (ppm), VOC (Index), and PM 1.0, 2.5, 4.0, 10 (ug/m3)

## Contents
- [Project Overview](#project-overview)
- [Required Hardware](#required-hardware)
- [Firmware Setup](#firmware-setup)
- [Assembly](#assembly)
- [Using Aeroq](#using-aeroq)

---
## Project Overview

### Hardware
Schematic and PCB designed using KiCad. Schematic, PCB design, and manufacturing files can be found under [Aeroq Hardware/](./Aeroq%20Hardware/v3)
<img width="500" height="1011" alt="image" src="https://github.com/user-attachments/assets/9c807429-ff14-4dc1-8fe5-cacd1b4f265d" />

### Enclosure
Two 3D printed components are required for the assembly. Model files can be found under [Aeroq Enclosure/](./Aeroq%20Enclosure)
<img width="500" alt="image" src="https://github.com/user-attachments/assets/4743dbd0-92f9-4496-bc7a-a594504eaffe" />

### Firmware
Source code for ESP32 firmware, developed with PlatformIO. Can be found under [Aeroq Firmware/](./Aeroq%20Firmware)

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

This project uses [PlatformIO](https://platformio.org/), a plugin for VSCode that streamlines ESP32 development. The firmware is written in C++ and targets the **ESP32-C3** microcontroller.  

### Steps

1. **Install VSCode**  
   Download and install [Visual Studio Code](https://code.visualstudio.com/).

2. **Install PlatformIO**  
   In VSCode, open the Extensions tab (`Ctrl+Shift+X`), search for **PlatformIO IDE**, and install it.

3. **Clone this Repository**  
   ```bash
   git clone https://github.com/EnriqueNeyra/Aeroq.git
   cd "Aeroq/Aeroq Firmware"

4. **Open Project in PlatformIO**  
   - In VSCode, go to *File > Open Folder*  
   - Select the `Aeroq Firmware/` directory.  

5. **Connect the ESP32-S3**  
   - Use a USB-C cable to connect the board to your computer.  
   - Ensure the correct COM port/device is available  
   - The *monitor_port* configuration value may need to be changed in the platformio.ini file  

6. **Build and Upload**  
   In PlatformIO, use the toolbar at the bottom:  
   - **Build**: Compile the firmware  
   - **Upload**: Flash firmware to the ESP32-C3  

7. **Monitor Serial Output (Optional)**  
   - Use the PlatformIO **Serial Monitor** to view sensor readings  
   - Default baud rate: `115200`

---

## Assembly

### 1. Components should connect as pictured below
<p align="center"><img src="https://github.com/user-attachments/assets/5740bab2-e6b4-4e19-9f66-7cb31afb2b5f" width="700"></p>

### 2. Insert the SEN54 into the slot in the enclosure, ensuring it is pressed in all the way
<p align="center"><img src="https://github.com/user-attachments/assets/f272257f-dbc3-4e41-ade8-5990a57cf071" width="700"></p>

### 3. Place display into frame
<p align="center"><img src="https://github.com/user-attachments/assets/dd597183-dd0e-4cfe-8b03-1174462b1bcc" width="700"></p>

### 4. Orient the PCB so that it sits against the back of the display. This CAD assembly image illustrates the correct component orientation
<p align="center"><img src="https://github.com/user-attachments/assets/d5072620-c4a7-4da5-b4a5-5fcc545736fb" width="700"></p>

### 5. Press the enclosure shut. The USB-C port should be visible and accessible through the slot at the rear
<p align="center"><img src="https://github.com/user-attachments/assets/f5b62217-b6d0-4359-aae4-ba4664226a09" width="700"></p>

---

## Using Aeroq

Simply plug the device in using any USB-C cable/power supply (minimum 5V 1A), and wait for the initialization to complete.  
**For optimal measurement accuracy, keep it away from heat sources (electronics, lights, etc) and in an area with minimal clutter.**
<img width="2268" height="1181" alt="image" src="https://github.com/user-attachments/assets/763cf77f-df35-4627-929d-93a298740765" />


## License

This project (hardware, design files, and firmware) is licensed under the 
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-nc-sa/4.0/).

[![License: CC BY-NC-SA 4.0](https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png)](http://creativecommons.org/licenses/by-nc-sa/4.0/)
