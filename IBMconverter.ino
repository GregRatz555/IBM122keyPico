// IBM Model M 122 key Driver

#include <Keyboard.h>

// Pins
#define DATA_IN 	29
#define DATA_OUT 	28
#define CLOCK_IN    27
#define CLOCK_OUT   26

// LED pins
#define CAPS_LOCK_LIGHT 2
#define NUM_LOCK_LIGHT  3
#define REC_MACRO_LIGHT 4

#define MAX_MACRO_LEN 1024

//TODO 
// Figure out HID for media controls, reading Capslock/numlock from computer
// Save macros to file and load on start

// The keycode most recently read
int code;
// Boolean for bunnyhopping
int bhop_enabled;
// Track status for macros
int ctrl_pressed;
int shift_pressed;
int release_key;
// Macros
char macros[12][MAX_MACRO_LEN]; // 12 macros of max length
int  macro_len[12];   // lengths of each macro
int  recording_macro; // Which macro is recording, -1 is none
int  macro_index; 	  // What index in current macro to write
// Track status for lights
int capslock_on;
int numlock_on;

void pull_clock_low(){
	// pulls clock line low using NPN transistor
	digitalWrite(CLOCK_OUT, HIGH);
}

void release_clock(){
	// undoes pull_clock_low
	digitalWrite(CLOCK_OUT, LOW);
}

void pull_data_low(){
	// pulls data line low using NPN transistor
	digitalWrite(DATA_OUT, HIGH);
}

void release_data(){
	// undoes pull_data_low
	digitalWrite(DATA_OUT, LOW);
}

void write_data(int val){
	// Invert output to drive NPN
	if (val) digitalWrite(DATA_OUT, LOW);
	else if (!val) digitalWrite(DATA_OUT, HIGH);
}

void write_clock(int val){
	// Invert output to drive NPN
	if (val) digitalWrite(CLOCK_OUT, LOW);
	else if (!val) digitalWrite(CLOCK_OUT, HIGH);
}

int KB_read(){
	// Receive a byte from the Keyboard

	// Disable interrupts
	gpio_set_irq_enabled(CLOCK_IN, GPIO_IRQ_EDGE_FALL, false);

	int val = 0;
	for (int i=0; i<11; i++){
		while(digitalRead(CLOCK_IN));
		val |= digitalRead(DATA_IN)<<i;
		while(!digitalRead(CLOCK_IN));
	}
	val = ((val >> 1) & 255);

	// Enable interrupts
	gpio_set_irq_enabled(CLOCK_IN, GPIO_IRQ_EDGE_FALL, true);
	return val;
}

void KB_write(int val){
	// Send a byte to the Keyboard

	// Disable interrupts
	gpio_set_irq_enabled(CLOCK_IN, GPIO_IRQ_EDGE_FALL, false);

	pull_clock_low();
	sleep_us(110);
	pull_data_low();
	release_clock();
	sleep_us(10);
	while (digitalRead(CLOCK_IN));
	int parity = 1;
	for(int i=0; i<8; i++){
		// Remember to invert for NPN pullup/down
		write_data(val & 1);
		parity ^= (val & 1);
		val = val >> 1;
		while (!digitalRead(CLOCK_IN)); while (digitalRead(CLOCK_IN));
	}
	// Send parity bit
	write_data(parity);
	while (!digitalRead(CLOCK_IN)); while (digitalRead(CLOCK_IN));
	// Send STOP bit
	release_data();
	while (!digitalRead(CLOCK_IN)); while (digitalRead(CLOCK_IN));
	// Catch ACK
	int ack = 0;
	while (!digitalRead(CLOCK_IN)){
		if (!digitalRead(DATA_IN)){
			ack = 1;
		}
	}
	// Enable interrupts
	gpio_set_irq_enabled(CLOCK_IN, GPIO_IRQ_EDGE_FALL, true);
}

char scancode_to_char(int scancode){
	switch (scancode){
		// Top row `12345667890-=BS
		case 0x0E:
			return '`';
		case 0x16:
			return '1';
		case 0x1E:
			return '2';
		case 0x26:
			return '3';
		case 0x25:
			return '4';
		case 0x2E:
			return '5';
		case 0x36:
			return '6';
		case 0x3D:
			return '7';
		case 0x3E:
			return '8';
		case 0x46:
			return '9';
		case 0x45:
			return '0';
		case 0x4E:
			return '-';
		case 0x55:
			return '=';
		case 0x66:
			return KEY_BACKSPACE;
		// Second row \tqwertyuiop[]
		case 0x0d:
			return KEY_TAB;
		case 0x15:
			return 'q';
		case 0x1D:
			return 'w';
		case 0x24:
			return 'e';
		case 0x2D:
			return 'r';
		case 0x2C:
			return 't';
		case 0x35:
			return 'y';
		case 0x3C:
			return 'u';
		case 0x43:
			return 'i';
		case 0x44:
			return 'o';
		case 0x4D:
			return 'p';
		case 0x54:
			return '[';
		case 0x5B:
			return ']';
		// Third row CAPSasdfghjkl;'ENTER
		// Caps lock taken to extras
		//case 0x14:
		//	return KEY_CAPS_LOCK;
		case 0x1C:
			return 'a';
		case 0x1B:
			return 's';
		case 0x23:
			return 'd';
		case 0x2B:
			return 'f';
		case 0x34:
			return 'g';
		case 0x33:
			return 'h';
		case 0x3B:
			return 'j';
		case 0x42:
			return 'k';
		case 0x4B:
			return 'l';
		case 0x4C:
			return ';';
		case 0x52:
			return '\'';
		case 0x53:
			return '\\';
		case 0x5A:
			return KEY_RETURN;
		// Fourth Row SHIFTzxcvbnm,./SHIFT
		case 0x12:
			return KEY_LEFT_SHIFT;
		case 0x13:
			return KEY_LEFT_SHIFT;
		case 0x1A:
			return 'z';
		case 0x22:
			return 'x';
		case 0x21:
			return 'c';
		case 0x2A:
			return 'v';
		case 0x32:
			return 'b';
		case 0x31:
			return 'n';
		case 0x3A:
			return 'm';
		case 0x41:
			return ',';
		case 0x49:
			return '.';
		case 0x4A:
			return '/';
		case 0x59:
			return KEY_RIGHT_SHIFT;
		// Space Row Ctrl Alt Space Alt Esc
		case 0x11:
			return KEY_LEFT_CTRL;
		case 0x19:
			return KEY_LEFT_ALT;
		case 0x29:
			return ' ';
		case 0x39:
			return KEY_ESC;
		case 0x58:
			return KEY_RIGHT_CTRL;
		// NUMPAD
		// Num lock moved to extras
		//case 0x76:
			//return KEY_NUM_LOCK;
		case 0x77:
			return KEY_KP_SLASH;
		case 0x7E:
			return KEY_KP_ASTERISK;
		case 0x84:
			return KEY_KP_MINUS;
		case 0x6C:
			return KEY_KP_7;
		case 0x75:
			return KEY_KP_8;
		case 0x7D:
			return KEY_KP_9;
		case 0x7C:
			return KEY_KP_PLUS;
		case 0x6B:
			return KEY_KP_4;
		case 0x73:
			return KEY_KP_5;
		case 0x74:
			return KEY_KP_6;
		case 0x7B:
			return KEY_KP_PLUS;
		case 0x69:
			return KEY_KP_1;
		case 0x72:
			return KEY_KP_2;
		case 0x7A:
			return KEY_KP_3;
		case 0x70:
			return KEY_KP_0;
		case 0x71:
			return KEY_KP_DOT;
		case 0x79:
			return KEY_KP_ENTER;
		// Above arrow keys INs Home PgUp Del End PgDn
		case 0x67:
			return KEY_INSERT;
		case 0x6E:
			return KEY_HOME;
		case 0x6F:
			return KEY_PAGE_UP;
		case 0x64:
			return KEY_DELETE;
		case 0x65:
			return KEY_END;
		case 0x6D:
			return KEY_PAGE_DOWN;
		// Arrows 
		case 0x60:
			return KEY_DOWN_ARROW;
		case 0x61:
			return KEY_LEFT_ARROW;
		case 0x62:
			return KEY_DOWN_ARROW;
		case 0x63:
			return KEY_UP_ARROW;
		case 0x6A:
			return KEY_RIGHT_ARROW;
		// F Keys lower row F1-F12
		case 0x07:
			return KEY_F1;
		case 0x0F:
			return KEY_F2;
		case 0x17:
			return KEY_F3;
		case 0x1F:
			return KEY_F4;
		case 0x27:
			return KEY_F5;
		case 0x2F:
			return KEY_F6;
		case 0x37:
			return KEY_F7;
		case 0x3F:
			return KEY_F8;
		case 0x47:
			return KEY_F9;
		case 0x4F:
			return KEY_F10;
		case 0x56:
			return KEY_F11;
		case 0x5E:
			return KEY_F12;
		// Left of keyboard select keys
		case 0x06:
			// Clear
			return KEY_ESC;
		case 0x05:
			// Attn
			return KEY_PRINT_SCREEN;
		case 0x09:
			// Recrd
			return KEY_LEFT_GUI;
		default:
			return 0;
	}
}

void enable_record_macro(int macro_num){
	// Begins recording the macro at the given index
	if ((macro_num < 0) || (macro_num > 11)) return;

	macro_index = 0;
	recording_macro = macro_num;
	macros[recording_macro][macro_index] = 0;
	macro_len[recording_macro] = 0;
}

void execute_macro(int macro_num){
	// Runs the stored macro 
	int macro_keys[12] = {0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x57, 0x5F};
	// Catch macro key out of range
	if ((macro_num < 0) || (macro_num > 11)) return;

	int i = 0;
	while (i < macro_len[macro_num]){
		int next_code = macros[macro_num][i];
		if (i < 2) {
			// First two code cannot be a release
			if (next_code == 240){
				i += 2;
				continue;
			}
		}
		if (next_code == 0) break;
		// Cannot hit same macro key in macro
		if (next_code == macro_keys[macro_num]){
			if (release_key) release_key = false;
			i++;
			continue;
		}
		apply_keypress(next_code);
		i++;
	}
}

void macro_keypress_action(int macro_number, int released){
	// Handles the functions for setting up or running each macro
	// This is to shorten repeated macro key functions in scancode_extra
	if (shift_pressed){
		if (released) {
			enable_record_macro(macro_number);
			release_key = false;
		}
	} else if (recording_macro == macro_number){
		if (released) {
			macros[recording_macro][macro_index] = 0;
			macro_index = 0;
			recording_macro = -1;
			release_key = false;
		}
	} else {
		if (released) {
			execute_macro(macro_number);
			release_key = false;
		}
	}

}

char scancode_extra(int code, int released){
	// Extra scancodes for custom objects returns 1 on success
	switch (code){
		/* Media control keys not supported in Keyboard.h, someone smarter will have to do this if desired
		case 0x0C:
			// ErLnp
			if (released) Keyboard.releaseRaw(0x80); // Volume up
			else Keyboard.pressRaw(0x80);
			return 1;
		*/
		case 0x14:
			// Caps lock
			// if ctrl+capslock pressed, just toggles light without setting capslock
			// Used because I'm too lazy to learn HID
			if (released){
				if (!ctrl_pressed){
					Keyboard.release(KEY_CAPS_LOCK);
				}
			} else {
				if (!ctrl_pressed){
					Keyboard.press(KEY_CAPS_LOCK);
				}
				capslock_on = !capslock_on;
			}
			return 1;
		case 0x76:
			// num lock
			// if ctrl+numlock pressed, just toggles light without setting numlock
			// Used because I'm too lazy to learn HID
			if (released){
				if (!ctrl_pressed){
					Keyboard.release(KEY_NUM_LOCK);
					// Toggle light
					numlock_on = !numlock_on;
				}
			} else {
				if (ctrl_pressed){
					numlock_on = !numlock_on;
				} else {
					Keyboard.press(KEY_NUM_LOCK);
				}
			}
			return 1;
		case 0x0B:
			bhop_enabled = true;
			return 1;
		case 0x03:
			bhop_enabled = false;
			return 1;
		case 0x08:
			// PF13 macro 0
			macro_keypress_action(0, released);
			return 1;
		case 0x10:
			// PF14 macro 1
			macro_keypress_action(1, released);
			return 1;
		case 0x18:
			// PF15 macro 2
			macro_keypress_action(2, released);
			return 1;
		case 0x20:
			// PF16 macro 3
			macro_keypress_action(3, released);
			return 1;
		case 0x28:
			// PF17 macro 4
			macro_keypress_action(4, released);
			return 1;
		case 0x30:
			// PF18 macro 5
			macro_keypress_action(5, released);
			return 1;
		case 0x38:
			// PF19 macro 6
			macro_keypress_action(6, released);
			return 1;
		case 0x40:
			// PF20 macro 7
			macro_keypress_action(7, released);
			return 1;
		case 0x48:
			// PF21 macro 8
			macro_keypress_action(8, released);
			return 1;
		case 0x50:
			// PF22 macro 9
			macro_keypress_action(9, released);
			return 1;
		case 0x57:
			// PF23 macro 10
			macro_keypress_action(10, released);
			return 1;
		case 0x5F:
			// PF24 macro 11
			macro_keypress_action(11, released);
			return 1;
		default:
			return 0;
	}

}

void gpio_callback(uint gpio, uint32_t events){
	// Wrapper for the interrupt to call a read from keyboard
	code = KB_read();
}

void apply_keypress(int keycode){
	// Does the work of a keypress

	// Check for release code, set flag
	if (keycode == 240){
		release_key = true;
		return;
		//keycode = KB_read();
	}
	if (keycode != 0){
		char pressed = scancode_to_char(keycode);
		if ((pressed == KEY_LEFT_SHIFT) || (pressed == KEY_RIGHT_SHIFT)){
			shift_pressed = 1 ^ release_key;
		}
		if ((pressed == KEY_LEFT_CTRL) || (pressed == KEY_RIGHT_CTRL)){
			ctrl_pressed = 1 ^ release_key;
		}
		// If a valid normal keypress, press/release it
		if (pressed != 0){
			if (release_key){
				Keyboard.release(pressed);
				// Reset release flag after key acknowledged
				release_key = false;
			} else {
				Keyboard.press(pressed);
			}
		// Else handle Special keys
		} else {
			pressed = scancode_extra(keycode, release_key);
			if (pressed == 1){
				if (release_key){
					release_key = false;
				}
			}
		}
	}

}

void setup(){
	Serial.begin(115200);
	while (!Serial);
	Keyboard.begin();
	pinMode(DATA_IN,   INPUT);
	pinMode(DATA_OUT,  OUTPUT);
	pinMode(CLOCK_IN,  INPUT);
	pinMode(CLOCK_OUT, OUTPUT);
	pinMode(CAPS_LOCK_LIGHT,  OUTPUT);
	pinMode(NUM_LOCK_LIGHT,  OUTPUT);
	pinMode(REC_MACRO_LIGHT,  OUTPUT);
	release_data();
	release_clock();

	// Set up the interrupt for when KB starts sending a code
	gpio_set_irq_enabled_with_callback(
		CLOCK_IN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);	
	
	//Serial.println("IBM converter");
	KB_write(0xFF); //RESET
	int val = KB_read();
	val = KB_read();
	val = KB_read();
	val = KB_read();
	delay(22);
	KB_write(0xF8); //Set all keys make/break
	delay(22);
	code = 0;
	// Init all status trackers
	bhop_enabled = false;
	ctrl_pressed = false;
	shift_pressed = false;
	release_key = false;
	// These are guesses, hold ctrl and press to toggle
	capslock_on = false;
	numlock_on = false;

	// Clear all macros
	// Could be updated in future to load/store from tinyFS or something
	// But this weekend project is running out of time and I'm lazy
	recording_macro = -1;
	macro_index = 0;
	for (int i = 0; i < 12; i++){
		macro_len[i] = 0;
		for (int j = 0; j < MAX_MACRO_LEN; j++){
			macros[i][j] = 0;
		}
	}
}

void loop(){

	if (bhop_enabled){
		Keyboard.press(' ');
		Keyboard.release(' ');
	}
	if (code != 0){
		// Make the Key work
		apply_keypress(code);

		// Check if recording macro
		if (recording_macro != -1){
			if (macro_index > MAX_MACRO_LEN-1){
				Keyboard.print("  [MACRO OUT OF SPACE!!]  ");
				recording_macro = -1;
			} else {
				// Add code to macro list
				macros[recording_macro][macro_index] = code;
				macros[recording_macro][macro_index+1] = 0;
				macro_len[recording_macro] = macro_index;
				macro_index++;
			}
		}
		code = 0;
	}
	// Set lights
	digitalWrite(CAPS_LOCK_LIGHT, capslock_on);
	digitalWrite(NUM_LOCK_LIGHT, numlock_on);
	digitalWrite(REC_MACRO_LIGHT, (recording_macro != -1));
}


