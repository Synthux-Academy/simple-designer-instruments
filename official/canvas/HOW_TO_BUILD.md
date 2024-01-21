# Instructions on how to build the Canvas

## Before you start
In this document, you'll find the instructions to assemble the Canvas. The first step is to go check the [Canvas wiring cheat sheet](simple_canvas_wiring_cheat_sheet.pdf) print it, keep it by your side the whole building time, and add a little checkmark when you did put a wire. In this document, I'll refer to it as the "cheat sheet".

On this cheat sheet, you have the main table with all the wiring and two side tables showing the pinout of the daisy and the MCP23017 gpio expander pinout.
The main table top pin and bottom pin refer to the simple board pins (top and bottom rows). The "Daisy Pin" collum gives the real pin name. Note : there is also MCP23017 pin name in this collum, all the \_A0 to \_A7 and \_B0 to \_B7.

## Test Code

A test code to help you debug your hardware can be found in the software folder and is named [simple_canvas_hardware_test.ino](software/simple_canvas_hardware_test/simple_canvas_hardware_test.ino). On top of the code l12 to l17, you can comment and uncomment the define to enable of disable tests, I'll refer to this code as the "test code".

### Final result
Here is the final result: 
![Main board Top](resources/pictures/Instructions%20(1).jpg)

### General tips
As you can see there are a lot of wires everywhere. And those wires are going under some 3D prints part used to hold the LEDs in place. To make it simple for you to solder try to use color wires, for example, use different colors for analog, digital, and data signals. Also, use the thinner wires you have to not create a chunky wire bloc that would not let you put the front panel. Also as you saw in the picture above, a lot of wires are under the 3D-printed parts. To avoid this problem (I didn't) you can solder as many wires as possible on the bottom of the PCB.

## Time to solder

### Power and output jack
After putting all the components in their correct slot according to the following table (TODO TABLE), you can start to wire the power of the daisy seed. They correspond to the last three lines of the cheat sheet.
You can also solder the two output jack, you can find them in the cheat sheet by searching for jack in the component type column.

### Linear Potentiometer

Lin pot and 5x lin pots on the cheat sheet.

The 5 linear potentiometers use only one analog gpio and 5 digital gpios. The firmware will select one potentiometer at a time with a digital gpio and then read its value. To avoid potentiometers that are not selected, we need to add diodes to their output before combining them.
You'll need to solder each output of the potentiometers to a diode anode (**top** pin 5, 10, 15, 20, 25). Then solder all those diodes cathodes together and wire them to the **bottom** pin 30. (cheat sheet 5x lin pots)
![Diodes Linear potentiometers](resources/pictures/Instructions%20(3).jpg)

You can now wire the 5 digital gpios to the inputs of the 5 potentiometers. (cheat sheet lin pot). I first did a little wrap on the bottom to route the linear potentiometer pin to pins that are connected to the bottom row. 
![Potentiometers digital gpios](resources/pictures/Instructions%20(7).jpg)
Finish by wiring the 5 top pins to the 5 bottom pins by following the cheat sheet.

You can test all the linear potentiometers with the test code by uncommenting l13 ```#define TEST_SEQUENCER_STEPS_READ```.

### Rotary potentiometers

The next steps are the rotary potentiometers. Simply search for _pot_ in the Component type column of the cheat sheet and solder their corresponding top to the bottom row.
You can test all the linear potentiometers with the test code by uncommenting l14 ```#define TEST_ANALOGS_READ```.

### Digital pins connected to the daisy

Only 5 digital pins are connected from the board to the daisy. You can find them in the cheat sheet by their Signal Name _Step0 to Step4_. You can solder their corresponding top to the bottom row.

You can test all the linear potentiometers with the test code by uncommenting l17```#define TEST_DIGITAL_PINS_READ```.

### Digital pins connected to the MCP23017 

The rest of the digital pins are connected to the gpio expander. The 1st step is to connect all the 3 positions and switch the second position to an accessible top-row gpio.

You can find them by searching _switch 3 pos 0_ and _switch 3 pos 1_ in the Component type column of the cheat sheet. The _switch 3 pos 0_ are already connected by the footprint, you'll need to connect all the _switch 3 pos 1_  wires from the footprint to their corresponding Pin top. You can refer to the following pictures, it's the 6 little yellow wires next to the switches in the middle of the board. 

![3 position switches](resources/pictures/Instructions%20(8).jpg)

After this step, you can wire the MCP23017. Search for _gpioExpander_ in the Component type column. You'll have two interrupts signal and i2c pins. Don't forget to also put 3V3 and GND into the MCP23017!

You can now wire all the digital Gpios from the top row to the MCP23071 Gpios.  Search for _\_A0 to \_A7 and \_B0 to \_B7_ in the cheat sheet.


You can test all the MCP23017 gpios with the test code by uncommenting l16 ```#define TEST_MCP_PINS_READ```.

### Capacitive touch sensor MPR121

In the cheat sheet search for the touch sensor in the Component type column. You'll find the i2c pins to also solder to the MPR121 and an interrupt pin. You also need to solder its VCC and GND.

The last step is to wire the capacitive electrode from the front panel to the MPR121 as shown on this table:

| Front panel connector pin number | MPR121 capacitive input number |
|----------------------------------|--------------------------------|
| 1                                | 4                              |
| 2                                | 0                              |
| 3                                | 1                              |
| 4                                | 2                              |
| 5                                | 3                              |
| 0                                | Not connected                  |
| 6                                | 8                              |
| 7                                | 7                              |
| 8                                | 6                              |
| 9                                | 5                              |


You can test capacitive touch pads with the test code by uncommenting l15 ```#define TEST_CAPACITIVE_TOUCH_READ```.

### Leds

To solder the LEDs, we first need to 3d print the led holder part.

#### 3D Printing interludes

You can find all the parts to print inside the [hardware](hardware) folder. All the parts are in 3mf and stl. You'll find:
1. Led_holder_main : main leds holder
2. Led_holder_source : holder of the 4 sources leds
3. Blocker_source : piece used to block light diffusion for the source leds
4. Blocker_destination : piece used to block light diffusion for the destination leds

You can find those pieces numbered in the following picture:
![numbered 3d print](resources/pictures/Instructions%20(13).jpg)

Take your favorite slicer and print those 4 pieces. I recommend using dark grey or black filament/resin for pieces 3 and 4 since their goal is to block light.

#### Add leds in 3d print and solder them

You can now put the led in the led holders. The holder is made so the led can be put and removed without having to desolder them. All the leds are daisy chained following this numbering: 
![numbered leds](resources/pictures/Instructions%20(10).jpg)

This means you can chain their VCC, GND, and the 1st-led DOUT to the 2nd-led DIN, etc...

To solder the leds, I recommend doing the following steps:
- put your two leds in their correct spot in the led holder
-  preparing your wires the correct size
-  strip your wire on a little shorter distance than the led pad
-  put some solder on the stripped ends of the wire and bend those striped ends 90°
-  add some solder on the pads of the leds
-  solder the 3 wires on both leds 

You can already add some long wires on the 1st led to solder it at the end when all the 3D-printed pieces are in place

Here is an example of what it should look like

![close up led](resources/pictures/Instructions%20(5).jpg)

When all your leds are soldered together you can mount the 3D-printed pieces to the PCB. 

The _Led_holder_source_ is a bit tricky to install. The 3D-printed holder is made to clip itself on the bottom row of the daisy header. You'll need to remove your daisy, put the 3D piece and its led, and finally put the daisy back on its header. You can then glue the  as seen on the _Blocker_source_ following pictures:

1. ![clip the Led_holder_source](resources/pictures/Instructions%20(9).jpg)
2. ![put back daisy on the Led_holder_source](resources/pictures/Instructions%20(11).jpg)
3. ![install the led Blocker_source](resources/pictures/Instructions%20(12).jpg)

The _Led_holder_main_ is easier to put, it will clip itself thanks to some switch. The _Blocker_destination_ will clip itself on the _Led_holder_main_ piece.

Finally, you can solder the 1st led to some VCC, GND, and the DIN according to the cheat sheet. Search for neopixel as component type.
You can test all the leds with the test code by uncommenting l112 ```#define TEST_NEOPIXELS```. All leds are turned on in cycle going from all red, all green, all blue.

##### Leds noise

The leds PWM does produce some noise on the audio output. To reduce it, you will have to put some big capacitors in parallel with the power supply. In my case, I put two capacitors, one on the VCC GND pin on the simple board and one directly in parallel to the VCC and GND connected to the first LED.

I tried different capacitor values close to 1000 uF until most of the noise disappeared. Adding too big capacitors can create a problem where the daisy seed doesn't boot correctly when plugging the USB due to the charge of the capacitor. By unplugging and replugging the USB, the voltage will be more stable since the capacitor wouldn't have to charge completely and so the daisy will boot properly. Pressing the reset button also works.

#### One last tip

If you have your daisy on the top of the PCB, it will be complicated to access the boot and reset button to reprogram it. To avoid having to remove all the bolts and potentiometer caps each time you want to reprogram it, you can put some buttons in parallel to the daisy buttons with some wrapping wires as shown in the following pictures:

![wrapping wire on the daisy buttons](resources/pictures/Instructions%20(6).jpg)
![deported buttons](resources/pictures/Instructions%20(3).jpg)
