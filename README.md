# IBM122keyPico
IBM 122 Key converter for Raspberry Pi Pico  
This is a quick weekend project to make a converter for the IBM 122 key keyboard using a raspberry pi pico to translate the IBM set 3 keycodes to USB keyboard inputs.  
Because if you have one of these and some basic skills, you shouldn't need to shell out $50 or so for a Soarer's converter.  
It's pretty simple, the codes are send using PS2 protocols, just with different scancodes. And the option to set keys to make/break is off by default but this code enables it on startup.  

## About this converter  
The code included here by me remaps the keys to fit the normal 83 key model M layout I'm used to.  
This means that the '< >' key next to left shift is also left shift (turns out thats the side of the key I was hitting usually)  
The '! cent symbol' and '\ |' keys are now '[ {' and '] }' and so on.   
The center arrow key is down (down is down too)  
and the keys above the arrow keys and keys surrounding the numbers on the numpad are also as they appear on an 83 key keyboard.  

Some extra keys I added are:  
an OS key on 'Recrd',  
Escape on right alt and on 'Clear'  
Printscrn on 'attn'  
And I made enable and disable 'bunny-hopping' (Spamming spacebar) on 'Erase EOF' and 'ExSel' respectively.  
The function keys 13-24 are for macros, holding shift and pressing one of these keys will begin recording, and pressing the same button again without shift will end the recording. Pressing the key from here on will replay the sequece you typed while recording.   
If you hook up the LED's, there is one to indicate that this recording is happening.  
If you want to change these, look up the scan codes here https://github.com/tmk/tmk_keyboard/wiki/IBM-PC-AT-Keyboard-Protocol#scan-code-set-3 and change them in the code. This is also a pretty good source of info about your keyboard  

## How to build
To build one, You need:  
A 240 degree 5 pin DIN connector (Be careful, the more common 180 degree 5 pin DIN is not compatible)  
2x NPN Transistor  
2x 1k-10k pull up resistors  
2x Diodes  
3x LED  
3x Current limiting resistors for LED, choose based on desired brightness, mine are 1.2KOhm.  
  
Build this   
![Schematic](https://www.burtonsys.com/ps2_chapweske_files/ps2.JPG)   
found at https://www.burtonsys.com/ps2_chapweske.htm  
If this image is lost, as it is an old looking site, you will need to imagine +5V, with the 1-10k pull up resistors connecting from 5V to Data and clock,  
each with a diode that connects (downstream from the pullups) from those to 'A' and 'B' on your pico,  
and with 'C' and 'D' driving the bases of NPN transistors that will short those aforementioned Data and Clock pins to ground.  
Change the code's pin definitions to match:  
A: DATA_IN  
B: CLOCK_IN  
C: CLOCK_OUT  
D: DATA_OUT  
I've also included LEDs to indicate Caps lock, Num lock, and macro recording.  
Set the pins you use in the definitions for:  
CAPS_LOCK_LIGHT  
NUM_LOCK_LIGHT  
REC_MACRO_LIGHT  
  
The pins for the 5 pin DIN are defined here ![Pinouts](http://i.imgur.com/NNquq.jpg)  
If this image is lost in time, they are like this:  
./....3....\\  
|.2.      .4.|  
.\\.1..n..5./  
That is, with the notch on the bottom, counterclockwise around the socket.  
 with n representing the notch in the shield.  
 1: +5V  
 2: Gnd  
 3: Sheath, if your DIN connector has a pin that connects to the sheath around the pins, connect to that.  
 4: Data  
 5: Clock  
This diagram is shown looking "into the socket", so this is looking at your socket, not at your keyboard connector, and not at the back side of the socket where you'll be soldering wires. This is important, you don't wanna fry your expensive new keyboard.  

## Homework (Issues you may want to fix yourself cause you're a smart little hacker)
### Macros are not stored between power cycles. 
 This is likely easy to do with LittleFS, but I am pretty lazy and this weekend project has overrun its time allowance.  
### Volume Keys
 I really wanted to make "Cursr Sel and Erlnp" on the side grouping of keys do volume up and down, but this code is based on Keyboard.h which does not include the "HID Consumer Control" options.   
I did not have the time/patience to learn how to move this project to use tinyUSB and implement them myself and it seems that no one has made a nice raspberry pi pico-USB keyboard library that allows for keyboard presses/releases with media controls as well.   
Well for Arduino anyways. Adafruit makes it available in MicroPython I think but I discovered that much too late to consider rewriting everything in another language.   
I examined bluemicro_hid but it does not allow for key-releases one at a time, only releasing all keys. If you knew what my bunnyhopping key was for, then you understand why this is unacceptable for a keyboard.  
### Capslock/Numlock lights
Another thing missing from Keyboard.h and other hid libraries I browsed is the ability to query the OS for what the capslock and numlock states are.  
In the absence of this, I put some extra code in so that if your capslock/numlock lights are not aligned with what your computer is doing, holding control and pressing those keys will toggle the light to synchronize it with the proper states.  

So if you're a ~~nerd~~ technically inclined hero, you might take this as a call to action and make that useful raspberry pi pico HID keyboard library, and maybe give me a MR for other ~~nerds~~ vintage computing enthusiasts to make full use of their skateboard-sized keyboard.
.  
