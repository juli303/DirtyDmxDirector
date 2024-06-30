
# DirtyDmxDirector (WIP)

Ever needed a simple standalone DMX controller that fits in your pocket, doesn't need a network or computer and can be powered over USB?
This is the solution, DirtyDMXDirector FTW!  
The current hardware is the Elecrow ESPTerminal as it combines the ESP32 with a nice screen and case. The only thing you have to add is a MAX485 IC (and optocouplers and resistors if you want to be on the safe side).  

> [!WARNING]
> Keep in mind that this project is WIP! A more detailed documentation is planned and this project is at a very early stage. (More like a proof of concept than a usefull product)

## Features

- [x] Simple User Interface to set the animation (currently only static) and value for each channel
- [x] DMX Output via the serial port of the microcontroller
- [ ] Basic animations (Squarewave, Triangle, Sawtooth), yet to be implemented
- [X] Holds the values after reboot

## Possible future features
- support for other ESP32 displays
- web UI for ESP32s without displays
- Artnet Node mode (to act as an artnet node)
- output to Artnet (as a master)
- fixture definition (would probably use an open fixture file format for compatibility)


## Required Hardware
- Elecrow ESP Terminal
- MAX485

## Libraries
- [LVGL](https://github.com/lvgl/lvgl)
- [esp_dmx](https://github.com/someweisguy/esp_dmx) by someweisguy

## Contributing

Contributions are always welcome!

## Authors

- [@juli303](https://www.github.com/juli303)
