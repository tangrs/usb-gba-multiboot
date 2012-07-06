# The USB-GBA multiboot project

This was a project to build a set of software and microcontroller firmware to upload code to the GameBoy Advance via a USB serial interface. It includes software to do the transfer and firmware for the Teensy AVR (available from [PJRC](http://www.pjrc.com/teensy/)).

## Introduction

The GameBoy Advance has a function where it can download code from other GameBoys carrying supporting games. That way, its possible to play multiplayer games with only one console with the game pak.

This project will allow you to upload your own GBA code (up to 256KB) via this method and makes a cheap way to get homebrew working without a flashcart (which is quite difficult to find nowadays).

There are several different interfaces that can be used (more info can be found on [gbatek](http://nocash.emubase.de/gbatek.htm#gbacommunicationports)). The interfaces that can be used include the Joybus protocol, SIO normal mode and the multiplayer mode.

## Implementation details

SIO normal mode is used because it's a 32bit interface, provides the fastest transfer speed since it recieves at the same time as sending and is easiest to implement. Unfortunately, this means that only one GBA can be multibooted at one time.

The firmware for SIO multiplayer mode is also included but this is much slower because it's a 16bit protocol and it operates at half-duplex.

The Teensy acts as a USB serial device and takes 4 bytes from the PC, transmits it to the Gameboy Advance and sends 4 bytes back to the PC.

Data are sent high byte first and lower byte last.

## Second stage loader

The second stage loader is supposed to speed up transfer but the speed difference is negligent when using SIO normal mode.

Basically, it's useless.

## How to use

Turn on your GBA without a game pak, plug everything in and run the gbaxfer program.

If you need to have a game pak installed for whatever reason, hold down START+SELECT on startup to cancel booting the game pak.

## Teensy Pinout

```
GBA             Teensy
SD ---------->  PB0
SC ---------->  PB1
SI ---------->  PB2
SO ---------->  PB3
```

Probably should also add some voltage regulators to step the voltage down to 3.3v. I ran mine at 5v and there wasn't too many dramas.

## License

GPL v3