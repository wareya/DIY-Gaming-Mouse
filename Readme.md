# DIY Gaming Mouse

This repository contains the code, schematic, PCB layout, and 3d model for a completely DIY gaming mouse, all designed around a Pixart PMW3360 sensor.

None of the companies that make gaming mouse sensors sell them, including Pixart, so the sensor needs to be salvaged (or bought from a scalper, or a small business that specializes in reselling them).

## Features

- True zero-latency button debouncing
- Low-latency motion tracking (no smoothing)
- Completely open source (Apache 2.0 license)
- Even the hardware is open source! (except the processor and sensor)
- Ultra light-weight shell and assembly (55g in total)
- Fingertip-grip shell shape
- Extra middle mouse button
- - the button that normally controls DPI on other mice is mapped to M3
- PCB exposes extra unused pins from the rp2040 as test pads for hardware/firmware modding fun; attach your own sensors! or LEDs! whatever you want!

TODO:

- Finish the debouncing algorithm; right now, it doesn't reject rare forms of bounce (ON-to-OFF bounce, as opposed to ON-to-MIDDLE bounce, which it DOES reject)
- Configurable DPI (currently hardcoded at 1200)
- - To change it in the firmware code, edit the `spi_write(REG_CONFIG1, 11); // 1200 dpi` line, replacing 11 with your dpi divided by 100 minus 1, e.g. 7 for 800 dpi, 3 for 400, 14 for 1600, etc
- "fun" optional features like WMO-style diagonal jaggy removal or optional in-formware acceleration, angle snapping, cinematic smoothing, etc

## Required skills

You need to be familiar with, or be able to learn quickly:

- 3d printing
- how to use a "slicer" (e.g. Cura) to convert stl models to gcode (what your 3d printer uses)
- repeatedly making test prints
- how to clean up 3d prints and remove support material from fragile prints
- basic firmware code debugging skills
- the ability to read embedded C/C++ code
- the Arduino IDE (I use 2.x, not 1.x)
- desoldering (with a hot air rework station)
- soldering (with a soldering iron)
- soldering bodge wires to existing pieces of hardware without creating solder bridges (for SROM extraction)
- comfort using a hot air rework station to soften and fix misshapen parts on 3d printed objects if they print improperly
- the ability to do all of the above without wasting tons of money or burning your home down
- familiarity with PCB printing companies and PCB assembly manufacturers (JLCPCB, PCBWay, etc)
- patience

If you cannot do all of the above, you should have someone else build this project for you, rather than building it for yourself. Or, at the very least, I can't give you support if you don't have the skills needed to make this; I'm just some random guy.

You also need:

- access to a 3d printer that you can repeatedly print test prints on until they come out right
- a hot air rework station with temperature and air flow rate control
- a soldering iron with temperature control
- various small tools like screwdrivers, tweezers, cleanup blades / deburring tools, etc. for various parts of the building process

## Instructions

### 3D printing the shell

This assumes you're using a filament printer. Resin printers might work too, but I have never used one.

I print with PLA filament.

Print each of the STL files in the "Case Model" directory in their natural (not default!) orientations.

You need to convert the stl files to gcode yourself and run various test prints because each 3d printer and filament has different requirements for layer thickness, support cutoff angle, printing speed, z seam alignment placement, etc.

Cura will complain about certain pieces (like the shell bottom) not being "water tight", but it's wrong; the watertightness check doesn't handle multiple overlapping solids properly. The models will still slice correctly and without glitches.

The thumb pieces must be rotated into the orientation where they do not need any supports. This means the small part of the thumb piece is facing up, ans the long part of the switch clamp section is on the bottom.

The scroll wheel must be rotated so that the thick cylinder part is on the bottom and the thin pin part is on the top. It must be printed with supports; any type of support is fine. It should be printed at 100% speed or lower, not higher, until it gets to the pin, at which point you MUST reduce the printing speed as low as you can tolerate. On my printer, a Sovol SV06, I had to turn it down to 25% speed. If your printer doesn't allow you to change the speed mid-print, you have to print at the low speed for the entire print. This is important; you have to print it slowly, because otherwise, the pin will be misaligned with itself and the scroll wheel will be off-balance and hit the middle mouse button switch with different pressures depending on where it is in its spin cycle.

Print the scroll wheel brace as-is.

Print the shell bottom as-is.

Print the shell top with tree supports and a support cutoff angle between 60 and 70 degrees. Do not use "standard"/traditional/grid supports; they will be too difficult to remove, and you will probably damage the print trying to remove them. On my printer, a Sovol SV06, 67 degrees was just right. If you have poor print quality, you might need to reduce the cutoff angle down to something silly like 50 degrees. If you have to go below 50 degrees, get a better printer, or make sure you're using the right filament at the right temperature, etc.

Once the shell top is printed, you need to use a deburring tool (or other hard, sharp metal implement) to remove a small amount of material from a certain curved edge along the seam where it meets the shell bottom, on the right side of the shell (with the button pieces away from you and the "butt" part towards you). The shell will fit together without removing a small amount of material from this part of the shell top, but it'll be misaligned by around a quarter of a millimeter and the right click won't quite work right. This material has to be removed because it sags during printing, and it sags during printing because of how support bridging works; the 3d model itself is correct.

You should print each part one-by-one and check for errors each time.

The final result will be somewhat rough and rugged, but it's a functional mouse shell.

### SROM preparation

The SROM needs to be captured from an existing mouse because it's copyrighted. Give it the name `srom_3360_0x03.h`, `srom_3360_0x04.h`, or `srom_3360_0x05.h`, depending on what version it is. I poked around github and found other projects using the 3360 SROM, and this seems to be the way they decided to name them their header files, so I'm copying the filename from them.

Capture the SROM from a commercial 3360 mouse with this code, and a standalone raspberry pi pico (not this mouse's custom PCB): https://gist.github.com/wareya/0dc3498bd80be93c47b6f79aa9164321

Note: the above SROM capturing code requires you to use the "arduino-pico" board type in the Arduino IDE. But the firmware in this repostory requires you to use the "mbed OS" rp2040 board type.

Use a breadboard to make a ripping setup equivalent to that depicted in the following image:

![EspAlvz](https://github.com/wareya/DIY-Gaming-Mouse/assets/585488/19571f49-33f4-4286-96d8-effffb80b110)

Be very careful not to create any solder bridges between any of the pins on the 3360.

Reconnect the mouse so that it reboots and sends the SROM to the sensor again.

The output should be exactly 4094 bytes long, and the second byte should be 0x03, 0x04, or 0x05 (different versions).

Repeat the capturing process until you get the same SROM several times, just to make sure that it captured properly. To capture the SROM again, you need to reboot the mouse. This can be plugging it in to your computer again, or disconnecting and reconnecting its USB power wire, or any safe method you prefer.

The MD5 checksums of the versions of the SROM that I know about are:

```
0x03 - C64F2BDA742DD1623D6A5DC869FC5E60
0x04 - 0BCB410858FF21D091DDC497D19BC943
0x05 - 337254C54AD82C689D4B9A58D06A3CF6
```

If you have an SROM with a different version number, it might not work with this firmware. It might, but it also might not. I can't provide support for other SROM versions.

After capturing the SROM data, save it in a formatted .h file formatted as such:

```c

#define SROM_LENGTH (4094)

const uint8_t PROGMEM srom[SROM_LENGTH] =
{
0x01,
0x<version number>,
// ...
// etc
};
```

I cannot distribute the SROM in this repository, because it's copyrighted proprietary code.

Disclaimer: The above steps do not bypass any copy protection, no encryption is being broken, and copyright law is not being violated by making your own backup copy of the sensor's SROM and using it on your own device. This is fully legal in reasonable countries (like the US).

### PMW 3360 sensor salvage

The Pixart PMW3360 is not sold commercially to hobbyists or small repair shops; to get one, you have to either buy an expensive breakout board, buy it from scalpers on aliexpress (or similar), or salvage it from an existing commercial mouse. This project assumes that you're salvaging it from an existing commercial mouse, after capturing the SROM from that mouse.

You need a hot air rework station, NOT a soldering iron, to desolder and salvage the 3360 safely. If you try to use a soldering iron, even with desoldering wick and/or a desoldering pump, you are VERY likely to fail to salvage it, damaging the pins or sensor internals in the process. The 3360 is somewhat sensitive to high temperatures, so you can only heat it up for a fairly short amount of time, and desoldering the ground pin with a soldering iron will get the sensor hot enough that it might start to damage it.

The 3360 sensor's lens is usually directly mounted to the sensor by melting its clear mounting pins down onto the top of the sensor's IC package.

Take the commercial mouse's PCB, scrape off (don't break off) the top melted part of the lens's mounting posts. Then, being careful not to damage anything, remove the lens from the sensor and PCB and put it somewhere safe.

Now, put the commercial mouse's PCB upside down on a non-flammable surface, in a workspace free of any flammable material. Solid wood surfaces are USUALLY safe for this kind of work as long as they don't have any fluff or sawdust or rough patches, and the wood finish (if any) isn't flammable itself, but it's still technically dangerous, and I cannot be held responsible if you start a fire in such a workspace. You have been warned!

Find the bottom of the sensor's pins. To each of these pins, apply a small amount of either leaded solder (if it's all you have) or low-temperature non-leaded solder paste/gel/etc (preferred) (NOT the same thing as flux!). You want to mix your solder into the existing solder to make it easier to melt the existing solder without using dangerously high temperatures. Use the hot air tool at around 600F/300C the two types of solder mix together. Do not physically touch the PCB or pins with the hot air tool; rely only on the air itself.

Once the solder for each pin is mixed together, pass the hot air tool over each section of pins in a circular motion until the sensor eventually, finally comes loose. At this point, use any metal tool (like metal tweezers or a screw driver) to make sure the sensor fully comes out of the PCB.

Now, use a soldering iron to clean any excess remaining solder off of the pins. If the soldering iron ends up with too much solder on it, clean it off with brass/bronze wool or similar. Warning: at high enough temperatures, brass wool is flammable. Do not use steel wool as it is very flammable.

### PCB building and assembly

Build gerber files and drill files and all the other necessary files (BOM, pick and place, etc) that your PCB manufacturer needs to print **and assemble** the PCB. Some of the components on the PCB are very very delicate, so unless you're a genuine expert, trying to hand-solder them is doomed.

Once you have your PCBs, you need the following parts to solder to the board:

- 6 Omron D2F-01F switches (or compatible, e.g. D2FC)
- 11mm Alps-compatible scroll wheel encoder
- salvaged PMW 3360 sensor, undamaged, pins not bent
- - 3360 sensor's lens, with mounting posts still intact
- Four dupont-compatible header pins for connecting to USB cable (optional)
- spare discrete resistors in case you messed up any of the resistor values when ordering the PCB like I did

And the following additional parts for assembly:

- USB cable, ideally with dupont-compatible connectors
- 3d printed shell (2 parts)
- 3d printed side buttons (2 parts)
- 3d printed scroll wheel brace (1 part)
- 3d printed scroll wheel OR salvaged scroll wheel for 11mm Alps-compatible scroll wheel encoder
- mouse feet (salvaged or custom) with approximately 0.6mm (that's zero-point-six millimeters) thickness

Note: The mouse feet thickness is very important, within half a millimeter! Using any other thickness of mouse feet is likely to result in poor, glitchy tracking. The PMW3360 is VERY sensitive to VERY small changes in tracking distance.

Make sure you have screws the right size to screw everything together. Do a test-fit with screws if you need to. I used salvaged screws with a major diameter (outside edge of threads) of 2mm and a minor diameter of around 1.65~1.7mm and the holes on the case were designed around this, but slightly larger screws will still work.

![PPRKHyR](https://github.com/wareya/DIY-Gaming-Mouse/assets/585488/2df3f1eb-3b81-4f22-a2cc-522cd9e09db8)


Solder the dupont-compatible header pins to the PCB, or if you have a different plan to connect the PCB to USB, do that instead. Then, connect it to your computer and make sure you can program it with the Arduino IDE. If it doesn't work, try a different copy of the PCB. Don't continue until you know that the PCB and its rp2040 processor are working properly. You should also do a test-fit of the PCB inside the 3d printed shell bottom to make sure nothing went wrong in 3d printing, and that the edge cuts and drill holes on the PCB are accurate to how they were specified in the design files.

Solder each Omron switch to the board with the plungers aligned with the silkscreen illustration. Check and double check that the switches are fully flush with the board before and while you solder them; double check that they're flush after soldering the first pin.

Test fit the PCB into the shell bottom, and without screwing anything into anything else, attach the top half of the shell and make sure that the buttons can be clicked properly and aren't clicked just by attaching the top half of the shell. If they're hard to click or click themselves, your 3d printer has poor tolerances, and you may need to adjust the button pieces by softening them with the hot air tool and bending them slightly until they fit just right. Now that you're dong with that, remove the PCB from the shell.

Solder the scroll wheel encoder into to the RotaryEncoder footprint on the PCB. Be very sure that it's all the way down before and while soldering it. You may need to twist the pins very slightly with a pair of needle-nose pliers to get them to fit into their holes; the holes are very narrow, on purpose, to reduce the chance of solder bridges forming between them.

Solder the PMW3360 sensor to the top center of the PCB, making VERY, VERY sure that it's in ALL THE WAY before and while soldering it. If it's not in all the way, the PCB may not fit properly in the shell, and/or the sensor may fail to track properly.

Attach the lens to the sensor on the underside of the PCB. Then, slot the PCB into the bottom shell and make sure it fully fits all the way down.

Remove the PCB and fit the side button thumb pieces to their switches. The long piece of the switch brace should wrap around the "inside" of the switches (towards the other side button switch), not the "outside". To get the thumb pieces around the switches, first fit the "hook" to the bottom around the edge of the PCB, slightly offset from the switch, and rotate the thumb piece down so that the far end of its brace hits the top of the switch. Move it as far from the switch as you comfortable can, so that it doesn't have to bend too much to slide down past the top and around the other side of the switch. Once that happens, slide it towards the switch so that the small hook piece at the end of the brace fully wraps around the other corner of the switch. Repeat this for the other switch's thumb piece.

Fit scroll wheel brace around the middle mouse button (near the right mouse button), with the spikes towards the "inside" of the mouse and away from the middle mouse button. Now fit the scroll wheel and make sure it's snug and properly balanced. If it's not, you'll need to reprint it.

Put the PCB back into the shell bottom and make sure it's still snug. Route the USB cable behind the support post in the back of the case (it has a screw hole in it) and around the right side of the PCB (NOT the left side), hugging the right side of the right mouse button, and then feed it out through the slot in the front. Use hot glue to secure the USB cable to that front slot. Make sure that it points relatively straight out the front when you hot glue it down.

Plug it in, compile and upload the firmware from the Arduino IDE, and make sure everything works. If it does, you're done!

## License

3360_Mouse_pico.ino, the main file of this project, is released under the Apache License, version 2.0.

relmouse_16.h is based on files from mbed OS, also licensed under the Apache License, version 2.0. https://github.com/arduino/ArduinoCore-mbed
