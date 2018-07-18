#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <stdbool.h>
#include <math.h>

#include <graphics.h>
#include <macros.h>
#include <lcd.h>
#include "lcd_model.h"
#include <sprite.h>
#include "cab202_adc.h"
#include "usb_serial.h"

// Define constants
#define DELAY (10) /* Millisecond delay */
#define CAR_WIDTH (6)
#define CAR_HEIGHT (8)
#define TREE_WIDTH (8)
#define TREE_HEIGHT (8)
#define HOUSE_WIDTH (8)
#define HOUSE_HEIGHT (8)
#define PERSON_WIDTH (6)
#define PERSON_HEIGHT (8)
#define ROADBLOCK_WIDTH (8)
#define ROADBLOCK_HEIGHT (3)
#define SMALLCAR_WIDTH (5)
#define SMALLCAR_HEIGHT (5)
#define FINISHLINE_WIDTH (24)
#define FINISHLINE_HEIGHT (3)
#define FUEL_WIDTH (6)
#define FUEL_HEIGHT (8)

#define MAX_SPEED (0.5)
#define MAX_HORIZONTAL_SPEED (0.5)
#define MAX_DISTANCE (100)
#define MAX_FUEL (100)

#define THRESHOLD (512)
#define BIT(x) (1 << (x))
#define OVERFLOW_TOP (1023)

// Define sprites
Sprite car;
Sprite tree;
Sprite tree2; 
Sprite house;
Sprite house2;
Sprite person;
Sprite person2;
Sprite roadBlock;
Sprite roadBlock2;
Sprite smallCar;
Sprite smallCar2;
Sprite fuel;
Sprite finishLine;

// Define global variables

double elapsed_time = 0;
int condition = 3;
double current_speed = 0;
double fuel_remaining = 100.0;
double distance_travelled = 0;
int crash = 0;
int left_adc = 0;

/* Refuel variables */
float begin_refuel_amount = 0;
int refuel_stages = 7;
float stage_fuel = 0;
double refuel_start_time = 0;

char dashboard_string[20];
char time_string[20];
char distance_string[20];

enum game_states {
	SPLASH_SCREEN,
	PAUSE_SCREEN,
	PLAYING_GAME,
	GAME_OVER,
	END_GAME
} game_state = SPLASH_SCREEN;

// Define functions
void process_game_state (void);
void update_game_state (void);
void splash_screen (void);
void setup (void);
void new_lcd_init(uint8_t contrast);
void setup_timers (void);
void setup_buttons (void);
void setup_PWM (void);
void set_duty_cycle(int duty_cycle);

void usb_serial_send (char * message);
void usb_serial_send_int (int value);
void setup_usb_serial (void);
void usb_serial_save (void);

void draw_game (void);
void draw_dashboard (void);
void draw_road (void);
void process (void);

int rand_offroad_x (void);
int rand_onroad_x (void);
int rand_fuel_x (void);
int rand_fuel_y (void);

double update_speed (Sprite sprite);
void setup_car (void);
void update_car (void);
void accelerate_car (void);
void accelerate_car2 (void);
void decelerate_car (void);
void decelerate_car2 (void);
void update_steering (void);
void check_offroad (void);

void setup_tree (void);
void update_tree (void);
void setup_tree2 (void);
void update_tree2 (void);
void setup_house (void);
void update_house (void);
void setup_house2 (void);
void update_house2 (void);
void setup_person (void);
void update_person (void);
void setup_person2 (void);
void update_person2 (void);
void setup_roadBlock (void);
void update_roadBlock (void);
void setup_roadBlock2 (void);
void update_roadBlock2 (void);
void setup_smallCar (void);
void update_smallCar (void);
void setup_smallCar2 (void);
void update_smallCar2 (void);
void setup_fuel (void);
void update_fuel (void);
void setup_finishLine (void);
void update_finishLine (void);

void update_distance (void);
void update_fuel_remaining (void);
void begin_refuel (double time);
void refuel (double time);

bool sprites_collided (Sprite sprite1, Sprite sprite2, int sprite1_height, 
						int sprite1_width, int sprite2_height, int sprite2_width);
void update_collision (void);						
bool fuel_collided (void);
void finishLine_collision (void);
bool car_safe_spot (void);
void crash_collision (void);

void pause (void);
void do_game_over (void);
void reset_variables (void);
void do_end_game (void);

// Game state
bool game_over = false; /* Set true when game is over */
bool update_screen = true; /* Set false to prevent screen updates */
bool new_game = true;
bool game_paused = true;
bool not_refuelling = true;

// Define bitmaps
uint8_t car_image[] = {
	0b01111000,
	0b11001100,
	0b11001100,
	0b01001000,
	0b01001000,
	0b11001100,
	0b11001100,
	0b01111000,
}; // 6x8

uint8_t tree_image[] = {
	0b00011000,
	0b00111100,
	0b01111110,
	0b11111111,
	0b11111111,
	0b00011000,
	0b00011000,
	0b00011000,
}; 

uint8_t house_image[] = {
	0b01111110,
	0b01100110,
	0b11000011,
	0b10000001,
	0b10011001,
	0b10100101,
	0b10100101,
	0b11111111,
}; 

uint8_t person_image[] = {
	0b01111000,
	0b01001000,
	0b01111000,
	0b11111100,
	0b10110100,
	0b00110000,
	0b01001000,
	0b01001000,
};

uint8_t roadBlock_image[] = {
	0b11111111,
	0b01011010,
	0b01011010,
};

uint8_t smallCar_image[] = {
	0b01110000,
	0b11011000,
	0b01010000,
	0b11011000,
	0b01110000,
};

uint8_t fuel_image[] = {
	0b11110000,
	0b10010000,
	0b10011100,
	0b10010100,
	0b10010100,
	0b10010100,
	0b10010000,
	0b11110000,
};

uint8_t finishLine_image[] = {
	0b10001000, 0b00000000, 0b1000100,
	0b01010101, 0b11111111, 0b0101010,
	0b00100010, 0b00000000, 0b0010001,
};

/**------------------------------------------------
*** CREATE TIMERS
***---------------------------------------------**/

#define FREQ (8000000.0)
#define PRESCALE (64.0)

void setup_timers (void) {
	// Initialise Timer 1
	TCCR1A = 0;
	TCCR1B = 3;
	TIMSK1 = 1;
	
	// Initialise Timer 0
	TCCR0A = 0;
	TCCR0B = 4;
	TIMSK0 = 1;
	
	//Turn on interrupts
	sei();
} // end setup_timers

/**------------------------------------------------
*** GAME TIMER
***---------------------------------------------**/

uint32_t overflow_counter = 0;
ISR(TIMER1_OVF_vect) {
	if (!game_paused) {
		overflow_counter++;
	}
} // end ISR Timer 1

double current_time (void) {
	double time = (overflow_counter * 65536.0 + TCNT1) * PRESCALE / FREQ;
	return time;
} // end current_time

/**------------------------------------------------
*** DEBOUNCING TIMER
***---------------------------------------------**/

volatile uint8_t centre_counter = 0;
volatile uint8_t rightSwitch_counter = 0;
volatile uint8_t start_counter = 0;
volatile uint8_t up_counter = 0;
volatile uint8_t down_counter = 0;
volatile uint8_t left_counter = 0;
volatile uint8_t right_counter = 0;

volatile uint8_t centre_closed = 0;
volatile uint8_t rightSwitch_closed = 0;
volatile uint8_t start_closed = 0;
volatile uint8_t up_closed = 0;
volatile uint8_t down_closed = 0;
volatile uint8_t left_closed = 0;
volatile uint8_t right_closed = 0;

ISR(TIMER0_OVF_vect) {
	centre_counter = centre_counter << 1;
	rightSwitch_counter = rightSwitch_counter << 1;
	start_counter = start_counter << 1;
	up_counter = up_counter << 1;
	down_counter = down_counter << 1;
	left_counter = left_counter << 1;
	right_counter = right_counter << 1;
	
	volatile uint8_t centre_mask = 0b00000111;
	centre_counter &= centre_mask;
	volatile uint8_t rightSwitch_mask = 0b00000111;
	rightSwitch_counter &= rightSwitch_mask;
	volatile uint8_t start_mask = 0b00000111;
	start_counter &= start_mask;
	volatile uint8_t up_mask = 0b00000111;
	up_counter &= up_mask;	
	volatile uint8_t down_mask = 0b00000111;
	down_counter &= down_mask;
	volatile uint8_t left_mask = 0b00000111;
	left_counter &= left_mask;
	volatile uint8_t right_mask = 0b00000111;
	right_counter &= right_mask;
	
	centre_counter |= BIT_VALUE(PINB, 0);
	rightSwitch_counter |= BIT_VALUE(PINF, 5);
	start_counter |= ((BIT_VALUE(PINF, 5)) || (BIT_VALUE(PINF, 6)));
	up_counter |= BIT_VALUE(PIND, 1);
	down_counter |= BIT_VALUE(PINB, 7);
	left_counter |= BIT_VALUE(PINB, 1);
	right_counter |= BIT_VALUE(PIND, 0);
	
	if (centre_counter == centre_mask) {
		centre_closed = 1;
	}
	if (rightSwitch_counter == rightSwitch_mask) {
		rightSwitch_closed = 1;
	}
	if (start_counter == start_mask) {
		start_closed = 1;
	}
	if (up_counter == up_mask) {
		up_closed = 1;
	}
	if (down_counter == down_mask) {
		down_closed = 1;
	}	
	if (left_counter == left_mask) {
		left_closed = 1;
	}
	if (right_counter == right_mask) {
		right_closed = 1;
	}
	
	
	if (centre_counter == 0) {
		centre_closed = 0;
	}
	if (rightSwitch_counter == 0) {
		rightSwitch_closed = 0;
	}
	if (start_counter == 0) {
		start_closed = 0;
	}
	if (up_counter == 0) {
		up_closed = 0;
	}
	if (down_counter == 0) {
		down_closed = 0;
	}	
	if (left_counter == 0) {
		left_closed = 0;
	}
	if (right_counter == 0) {
		right_closed = 0;
	}
	
} // end ISR Timer 0

/**------------------------------------------------
*** DEBOUNCING FUNCTIONS
***---------------------------------------------**/

bool centre_debounce (void) {
	bool debounced = false;
	
	static uint8_t prevState = 0;
	if (centre_closed != prevState) {
		debounced = true;
	}
	
	return debounced;
} // end centre_debounce

bool rightSwitch_debounce (void) {
	bool debounced = false;
	
	static uint8_t prevState = 0;
	if (rightSwitch_closed != prevState) {
		debounced = true;
	}
	
	return debounced;
} // end rightSwitch_debounce

bool start_debounce (void) {
	bool debounced = false;
	
	static uint8_t prevState = 0;
	if (start_closed != prevState) {
		debounced = true;
	}
	
	return debounced;
} // end start_debounce

bool up_debounce (void) {
	bool debounced = false;
	
	static uint8_t prevState = 0;
	if (up_closed != prevState) {
		debounced = true;
	}
	
	return debounced;
} // end up_debounce

bool down_debounce (void) {
	bool debounced = false;
	
	static uint8_t prevState = 0;
	if (down_closed != prevState) {
		debounced = true;
	}
	
	return debounced;
} // end down_debounce

bool left_debounce (void) {
	bool debounced = false;
	
	static uint8_t prevState = 0;
	if (left_closed != prevState) {
		debounced = true;
	}
	
	return debounced;
} // end left_debounce

bool right_debounce (void) {
	bool debounced = false;
	
	static uint8_t prevState = 0;
	if (right_closed != prevState) {
		debounced = true;
	}
	
	return debounced;
} // end right_debounce

/**------------------------------------------------
*** GAME STATE
***---------------------------------------------**/

void process_game_state (void) {
	switch (game_state) {
		default:
		case SPLASH_SCREEN:
			reset_variables();		
			if (start_debounce()) {
				game_state = PLAYING_GAME;
				game_paused = false;
				setup();
			}
			break;
		case PLAYING_GAME:
			set_duty_cycle(0);
			if (centre_debounce()) {
				game_paused = true;
				game_state = PAUSE_SCREEN;
			}
			process();
			break;
		case PAUSE_SCREEN:
			set_duty_cycle(700);
			if (rightSwitch_debounce()) {
				game_state = PLAYING_GAME;
				game_paused = false;
			}
			if (up_debounce()) {
				usb_serial_save();
			}
			break;
		case GAME_OVER:
			if (centre_debounce()) {
				game_state = SPLASH_SCREEN;		
			} else if (rightSwitch_debounce()) {
				game_state = END_GAME;
			}
			break;
		case END_GAME:
			new_game = false;
			break;
	}
} // end process_game_state 

void update_game_state() {
	switch (game_state) {
		default:
		case SPLASH_SCREEN:
			splash_screen();
			break;
		case PLAYING_GAME:
			draw_game();
			break;
		case PAUSE_SCREEN:
			pause();
			break;
		case GAME_OVER:
			do_game_over();
			break;
		case END_GAME:
			do_end_game();
			break;
	}
} // end update_game_state

/**------------------------------------------------
*** SPLASH AND PAUSE SCREEN
***---------------------------------------------**/

void splash_screen (void) {

	char *title1 = "Race to";
	char *title2 = "Zombie Mountain";
	char *name = "Davina Tan";
	char *student_no = "n9741127";
	char *instructions = "L or R to play";
	
	draw_string(24, 2, title1, FG_COLOUR);
	draw_string(6, 10, title2, FG_COLOUR);
	draw_string(18, 18, name, FG_COLOUR);
	draw_string(22, 26, student_no, FG_COLOUR);
	draw_string(6, 34, instructions, FG_COLOUR);
		
	new_game = false;
} // end splash_screen

void pause (void) {

	char *pause = "PAUSED";
	snprintf(time_string, 11, "Time: %02f", elapsed_time);
	sprintf(distance_string, "Distance: %0.01f", distance_travelled);
	draw_string(24, 2, pause, FG_COLOUR);
	draw_string(1, 12, time_string, FG_COLOUR);
	draw_string(1, 22, distance_string, FG_COLOUR);

} // end pause 

/**------------------------------------------------
*** PROCESS AND MAIN
***---------------------------------------------**/

void process (void) {
	
	if (!game_paused) {
		elapsed_time = current_time();
		update_distance();
		update_fuel_remaining();
	}
	
	left_adc = adc_read(0);
	
	update_car();
	update_tree();
	update_tree2();
	update_house();
	update_house2();
	update_person();
	update_person2();
	update_roadBlock();
	update_roadBlock2();
	update_smallCar();
	update_smallCar2();
	update_fuel();
	update_finishLine();	
	
	if (sprites_collided(car, tree, CAR_HEIGHT, CAR_WIDTH, TREE_HEIGHT, TREE_WIDTH)) {
		update_collision();
	}
	
	if (sprites_collided(car, house, CAR_HEIGHT, CAR_WIDTH, HOUSE_HEIGHT, HOUSE_WIDTH)) {
		update_collision();
	}
	
	if (sprites_collided(car, house2, CAR_HEIGHT, CAR_WIDTH, HOUSE_HEIGHT, HOUSE_WIDTH)) {
		update_collision();
	}
	
	if (sprites_collided(car, person, CAR_HEIGHT, CAR_WIDTH, PERSON_HEIGHT, PERSON_WIDTH)) {
		update_collision();
	}
	
	if (sprites_collided(car, person2, CAR_HEIGHT, CAR_WIDTH, PERSON_HEIGHT, PERSON_WIDTH)) {
		update_collision();
	}
	
	if (sprites_collided(car, roadBlock, CAR_HEIGHT, CAR_WIDTH, ROADBLOCK_HEIGHT, ROADBLOCK_WIDTH)) {
		update_collision();
	}
	
/*	if (sprites_collided(car, roadBlock2, CAR_HEIGHT, CAR_WIDTH, ROADBLOCK_HEIGHT, ROADBLOCK_WIDTH)) {
		update_collision();
	}*/
	
	if (sprites_collided(car, smallCar, CAR_HEIGHT, CAR_WIDTH, SMALLCAR_HEIGHT, SMALLCAR_WIDTH)) {
		update_collision();
	}
	
	if (sprites_collided(car, smallCar2, CAR_HEIGHT, CAR_WIDTH, SMALLCAR_HEIGHT, SMALLCAR_WIDTH)) {
		update_collision();
	} 
	
	if (sprites_collided(car, fuel, CAR_HEIGHT, CAR_WIDTH, FUEL_HEIGHT, FUEL_WIDTH)) {
		condition = 0;
		game_state = GAME_OVER;
	}	
	
	if (fuel_collided()) {
		if (not_refuelling) {
			if ((current_speed >= -0.5) && (current_speed <= 0)) {
				not_refuelling = false;
				begin_refuel(current_time());
			} else {
				not_refuelling = true;
			}
		} else {
			refuel(current_time());
		}
	} else {
		not_refuelling = true;
	}

	crash_collision();
	finishLine_collision();	
	
} // end process

// Main program
int main (void) {
	set_clock_speed(CPU_8MHz); // set clock speed
	new_lcd_init(LCD_DEFAULT_CONTRAST); // initialise LCD screen
	setup_usb_serial(); // setup USB serial
	adc_init(); // initialise ADC
	
	setup_buttons();
	
	setup_timers();
	
	setup_PWM();
	
	if (new_game == true) {
		
		// while the game isn't over, play the game
		while (!game_over) {
			clear_screen();

			process_game_state();	
			
			if (game_state == PLAYING_GAME) {
				process();
			}
			
			update_game_state();
			show_screen();
		}
		return 0;				
	}
} // end main

/**------------------------------------------------
*** SETUP FUNCTIONS
***---------------------------------------------**/

void usb_serial_send(char * message) {
	// Cast to avoid "error: pointer targets in passing argument 1 
	//	of 'usb_serial_write' differ in signedness"
	usb_serial_write((uint8_t *) message, strlen(message));
} // end usb_serial_send

void usb_serial_send_int(int value) {
	static char buffer[8];
	snprintf(buffer, sizeof(buffer), "%d", value);
	usb_serial_send( buffer );
} // end usb_serial_send_int

void setup_usb_serial(void) {
	// Set up LCD and display message
	lcd_init(LCD_DEFAULT_CONTRAST);
	draw_string(10, 10, "Connect USB...", FG_COLOUR);
	show_screen();

	usb_init();

	while ( !usb_configured() ) {
		// Block until USB is ready.
	}
	
	// Send connection success message
	usb_serial_send( "Connection success!");
	usb_serial_send( "\r\n" );

} // end setup_usb_serial

void usb_serial_save (void) {
	usb_serial_send("Save attempt");
	usb_serial_send("\r\n");
	
//	 FILE *fp;
//
//   fp = fopen("/Assignment2_New/save.txt", "w+");
//   fputs("This is testing for fputs...\n", fp);
//   fclose(fp);	
   
} // end usb_serial_save

// Uses Timer 4 to drive the Output Compare Register 4A, on pin C7 (backlight).
void setup_PWM (void) {
	TC4H = OVERFLOW_TOP >> 8;
	OCR4C = OVERFLOW_TOP & 0xff;

	TCCR4A = BIT(COM4A1) | BIT(PWM4A);
	SET_BIT(DDRC, 7);

	TCCR4B = BIT(CS42) | BIT(CS41) | BIT(CS40);	
	TCCR4D = 0;	
} // end setup_PWM

void set_duty_cycle(int duty_cycle) {
	TC4H = duty_cycle >> 8;
	OCR4A = duty_cycle & 0xff;
} // end set_duty_cycle

void new_lcd_init(uint8_t contrast) {
    // Set up the pins connected to the LCD as outputs
    SET_OUTPUT(DDRD, SCEPIN); // Chip select -- when low, tells LCD we're sending data
    SET_OUTPUT(DDRB, RSTPIN); // Chip Reset
    SET_OUTPUT(DDRB, DCPIN);  // Data / Command selector
    SET_OUTPUT(DDRB, DINPIN); // Data input to LCD
    SET_OUTPUT(DDRF, SCKPIN); // Clock input to LCD

    CLEAR_BIT(PORTB, RSTPIN); // Reset LCD
    SET_BIT(PORTD, SCEPIN);   // Tell LCD we're not sending data.
    SET_BIT(PORTB, RSTPIN);   // Stop resetting LCD

    LCD_CMD(lcd_set_function, lcd_instr_extended);
    LCD_CMD(lcd_set_contrast, contrast);
    LCD_CMD(lcd_set_temp_coeff, 0);
    LCD_CMD(lcd_set_bias, 3);

    LCD_CMD(lcd_set_function, lcd_instr_basic);
    LCD_CMD(lcd_set_display_mode, lcd_display_normal);
    LCD_CMD(lcd_set_x_addr, 0);
    LCD_CMD(lcd_set_y_addr, 0);
} // end new_lcd_init

void setup_buttons (void) {
	// enable joystick input
	CLEAR_BIT(DDRB, 1); // left
	CLEAR_BIT(DDRB, 7); // down
	CLEAR_BIT(DDRD, 0); // right
	CLEAR_BIT(DDRD, 1); // up
	CLEAR_BIT(DDRB, 0); // centre	
	// enable button input
	CLEAR_BIT(DDRF, 5);
	CLEAR_BIT(DDRF, 6);
} // end setup_buttons

void setup (void) {
	// random seed
	srand(TCNT1 * 29);
	// fill with functions to set up the game
	setup_car();
	setup_tree();
	setup_tree2();
	setup_house();
	setup_house2();
	setup_person();
	setup_person2();
	setup_roadBlock();
	setup_roadBlock2();
	setup_smallCar();
	setup_smallCar2();	
	setup_fuel();
	setup_finishLine();	
} // end setup

void setup_car (void) {
	int car_x = (LCD_X - CAR_WIDTH) / 2;
	int car_y = (LCD_Y - CAR_HEIGHT - 2);
	sprite_init(&car, car_x, car_y, CAR_WIDTH, CAR_HEIGHT, car_image);
} // setup_car

void setup_tree (void) {
	int tree_x = rand_offroad_x();
	int tree_y = (rand() % (LCD_Y - TREE_HEIGHT - 1));
	sprite_init(&tree, tree_x, tree_y, TREE_WIDTH, TREE_HEIGHT, tree_image);	
} // end setup_tree

void setup_tree2 (void) {
	int tree2_x = rand_offroad_x();
	int tree2_y = (rand() % (LCD_Y - TREE_HEIGHT - 1));
	sprite_init(&tree2, tree2_x, tree2_y, TREE_WIDTH, TREE_HEIGHT, tree_image);	
} // end setup_tree2

void setup_house (void) {
	int house_x = rand_offroad_x();
	int house_y = (rand() % (LCD_Y - HOUSE_HEIGHT - 1));
	sprite_init(&house, house_x, house_y, HOUSE_WIDTH, HOUSE_HEIGHT, house_image);	
} // end setup_house

void setup_house2 (void) {
	int house2_x = rand_offroad_x();
	int house2_y = (rand() % (LCD_Y - HOUSE_HEIGHT - 1));
	sprite_init(&house2, house2_x, house2_y, HOUSE_WIDTH, HOUSE_HEIGHT, house_image);	
} // end setup_house2

void setup_person (void) {
	int person_x = rand_offroad_x();
	int person_y = (rand() % (LCD_Y - PERSON_HEIGHT - 1));
	sprite_init(&person, person_x, person_y, PERSON_WIDTH, PERSON_HEIGHT, person_image);	
} // end setup_person

void setup_person2 (void) {
	int person2_x = rand_offroad_x();
	int person2_y = (rand() % (LCD_Y - PERSON_HEIGHT - 1));
	sprite_init(&person2, person2_x, person2_y, PERSON_WIDTH, PERSON_HEIGHT, person_image);	
} // end setup_person2

void setup_roadBlock (void) {
	int roadBlock_x = rand_onroad_x();
	int roadBlock_y = (rand() % (LCD_Y - ROADBLOCK_HEIGHT - 1));
	sprite_init(&roadBlock, roadBlock_x, roadBlock_y, ROADBLOCK_WIDTH, ROADBLOCK_HEIGHT, roadBlock_image);	
} // end setup_roadBlock

void setup_roadBlock2 (void) {
	int roadBlock2_x = rand_onroad_x();
	int roadBlock2_y = (rand() % (LCD_Y - ROADBLOCK_HEIGHT - 1));
	sprite_init(&roadBlock2, roadBlock2_x, roadBlock2_y, ROADBLOCK_WIDTH, ROADBLOCK_HEIGHT, roadBlock_image);	
} // end setup_roadBlock2

void setup_smallCar (void) {
	int smallCar_x = rand_onroad_x();
	int smallCar_y = (rand() % (LCD_Y - SMALLCAR_HEIGHT - 1));
	sprite_init(&smallCar, smallCar_x, smallCar_y, SMALLCAR_WIDTH, SMALLCAR_HEIGHT, smallCar_image);	
} // end setup_smallCar

void setup_smallCar2 (void) {
	int smallCar2_x = rand_onroad_x();
	int smallCar2_y = (rand() % (LCD_Y - SMALLCAR_HEIGHT - 1));
	sprite_init(&smallCar2, smallCar2_x, smallCar2_y, SMALLCAR_WIDTH, SMALLCAR_HEIGHT, smallCar_image);	
} // end setup_smallCar2

void setup_fuel (void) {
	int fuel_x = rand_fuel_x();
	int fuel_y = 0;
	sprite_init(&fuel, fuel_x, fuel_y, FUEL_WIDTH, FUEL_HEIGHT, fuel_image);
} // end setup_fuel

void setup_finishLine (void) {
	int finishLine_x = ((LCD_X / 2) - (FINISHLINE_WIDTH / 2));
	int finishLine_y = 0;
	sprite_init(&finishLine, finishLine_x, finishLine_y, FINISHLINE_WIDTH, FINISHLINE_HEIGHT, finishLine_image);
} // end setup_finishLine

/**------------------------------------------------
*** DRAW FUNCTIONS
***---------------------------------------------**/

void draw_game (void) {
	// clear the screen before drawing everything on it
	clear_screen();
	draw_road();
	
	sprite_draw(&car);
	sprite_draw(&tree);
//	sprite_draw(&tree2); // made tree2 invisible 
	sprite_draw(&house);
	sprite_draw(&house2);
	sprite_draw(&person);
	sprite_draw(&person2);
	sprite_draw(&roadBlock);
//	sprite_draw(&roadBlock2);
	sprite_draw(&smallCar);
	sprite_draw(&smallCar2);
	sprite_draw(&fuel);
	sprite_draw(&finishLine);	
	
	draw_dashboard();
	
} // end draw_game

void draw_dashboard (void) {
	int fuelRemaining = round(fuel_remaining);
	int currentSpeed = round(current_speed);
	sprintf(dashboard_string, "H:%i  S:%2i  F:%i  ", condition, currentSpeed, fuelRemaining);
	draw_string(1, 0, dashboard_string, FG_COLOUR);
	draw_line(0, 8, 84, 8, FG_COLOUR);
} // end draw_dashboard

void draw_road (void) {
	int top = 0;
	int bot = (LCD_Y - 1);
	int left = (LCD_X / 3);
	int right = ((LCD_X / 3) * 2 - 1);
	
	draw_line(left, top, left, bot, FG_COLOUR);
	draw_line(right, top, right, bot, FG_COLOUR);
} // end draw_road

/**------------------------------------------------
*** UPDATE FUNCTIONS
***---------------------------------------------**/

void update_distance (void) {
	if ((current_speed >= 0.5) && (current_speed < 2)) {
		distance_travelled += 0.01;
	} else if ((current_speed >= 2) && (current_speed < 3)) {
		distance_travelled += 0.02;
	} else if ((current_speed >= 3) && (current_speed < 4)) {
		distance_travelled += 0.03;
	} else if ((current_speed >= 4) && (current_speed < 5)) {
		distance_travelled += 0.04;
	} else if ((current_speed >= 5) && (current_speed < 6)) {
		distance_travelled += 0.05;
	} else if ((current_speed >= 6) && (current_speed < 7)) {
		distance_travelled += 0.06;
	} else if ((current_speed >= 7) && (current_speed < 8)) {
		distance_travelled += 0.07;
	} else if ((current_speed >= 8) && (current_speed < 9)) {
		distance_travelled += 0.08;
	} else if ((current_speed >= 9) && (current_speed < 10)) {
		distance_travelled += 0.09;
	} else if (current_speed >= 10) {
		distance_travelled += 0.1;
	}
} // end update_distance

double update_speed (Sprite sprite) {
	
	if ((current_speed >= 0.5) && (current_speed < 2)) {
		sprite.dy = (MAX_SPEED/10);
	} else if ((current_speed >= 2) && (current_speed < 3)) {
		sprite.dy = ((MAX_SPEED/10) * 2);
	} else if ((current_speed >= 3) && (current_speed < 4)) {
		sprite.dy = ((MAX_SPEED/10) * 3);
	} else if ((current_speed >= 4) && (current_speed < 5)) {
		sprite.dy = ((MAX_SPEED/10) * 4);
	} else if ((current_speed >= 5) && (current_speed < 6)) {
		sprite.dy = ((MAX_SPEED/10) * 5);
	} else if ((current_speed >= 6) && (current_speed < 7)) {
		sprite.dy = ((MAX_SPEED/10) * 6);
	} else if ((current_speed >= 7) && (current_speed < 8)) {
		sprite.dy = ((MAX_SPEED/10) * 7);
	} else if ((current_speed >= 8) && (current_speed < 9)) {
		sprite.dy = ((MAX_SPEED/10) * 8);
	} else if ((current_speed >= 9) && (current_speed < 10)) {
		sprite.dy = ((MAX_SPEED/10) * 9);
	} else if (current_speed >= 10) {
		sprite.dy = MAX_SPEED;
	} else {
		sprite.dy = 0;
	}	
	return sprite.dy;
} // end update_speed

void update_fuel_remaining (void) {
	
	if (fuel_remaining > 0) {
		if ((current_speed >= 0.5) && (current_speed < 2)){
			fuel_remaining -= 0.0075;
		} else if ((current_speed >= 2) && (current_speed < 3)) {
			fuel_remaining -= 0.015;
		} else if ((current_speed >= 3) && (current_speed < 4)) {
			fuel_remaining -= 0.0225;
		} else if ((current_speed >= 4) && (current_speed < 5)) {
			fuel_remaining -= 0.03;
		} else if ((current_speed >= 5) && (current_speed < 6)) {
			fuel_remaining -= 0.0375;
		} else if ((current_speed >= 6) && (current_speed < 7)) {
			fuel_remaining -= 0.045;
		} else if ((current_speed >= 7) && (current_speed < 8)) {
			fuel_remaining -= 0.0525;
		} else if ((current_speed >= 8) && (current_speed < 9)) {
			fuel_remaining -= 0.06;
		} else if ((current_speed >= 9) && (current_speed < 10)) {
			fuel_remaining -= 0.0675;
		} else if (current_speed >= 10) {
			fuel_remaining -= 0.075;
		} 
	} else {
		condition = 0;
		game_state = GAME_OVER;
	}
} // end update_fuel_remaining

/**------------------------------------------------
*** RESPAWN POSITIONS
***---------------------------------------------**/

int rand_offroad_x (void) {
	// choose a random position off the road on both sides
	int offroad1_max = (LCD_X / 3 - 8);
	int offroad1_min = 1;
	int offroad1_num = rand() % (offroad1_max - offroad1_min + 1) + offroad1_min;
	
	int offroad2_max = (LCD_X - 8);
	int offroad2_min = ((LCD_X / 3) * 2 + 1);
	int offroad2_num = rand() % (offroad2_max - offroad2_min + 1) + offroad2_min;
	
	// randomly choose between the two random numbers
	int offroad_array[] = {offroad1_num, offroad2_num};
	
	int random_choice = rand() % 2;
	
	int random_pos = offroad_array[random_choice];

	return random_pos;
} // end rand_offroad_x 

int rand_onroad_x (void) {
	int onroad_max = ((LCD_X / 3) * 2 - 8);
	int onroad_min = (LCD_X / 3 + 1);
	int onroad_num = rand() % (onroad_max - onroad_min + 1) + onroad_min;

	return onroad_num;
} // end rand_onroad_x

int rand_fuel_x (void) {
	// choose a random position on each side of road 
	int left_x = (LCD_X / 3 - FUEL_WIDTH);
	int right_x = ((LCD_X / 3) * 2);
	
	// randomly choose between the two random numbers
	int fuel_array[] = {right_x, left_x};
	
	int random_choice = rand() % 2;
	
	int random_pos = fuel_array[random_choice];

	return random_pos;
} // end rand_fuel_x 

int rand_fuel_y (void) {
	int max = 0;
	int min = 0;

	if ((fuel_remaining > 75) && (fuel_remaining < 100)) {
		max = 100;
		min = 75;
	} else if ((fuel_remaining > 50) && (fuel_remaining < 75)) {
		max = 75;
		min = 50;
	} else if ((fuel_remaining > 25) && (fuel_remaining < 50)) {
		max = 50;
		min = 25;
	} else if ((fuel_remaining > 0) && (fuel_remaining < 25)) {
		max = 25;
		min = 1;
	}
	
	int num = rand() % (max - min + 1) + min;
	
	return num;
} // end rand_fuel_y

/**------------------------------------------------
*** UPDATE SPRITES
***---------------------------------------------**/

void update_tree (void) {
	int tree_y = round(tree.y);
	
	if (tree_y > (LCD_Y + TREE_HEIGHT)) {
		tree.x = rand_offroad_x();
		tree.y = 0;
	}
	
	tree.y += update_speed(tree);
} // end update_tree

/** For some unknown reason, tree2 keeps respawning on the road...
*** even though the x value is set to rand_offroad_x... 
**/
void update_tree2 (void) {
	int tree2_y = round(tree2.y);
	
	if (tree2_y > (LCD_Y + TREE_HEIGHT)) {
		tree2.x = rand_offroad_x();
		tree2.y = 0;
	}
	
	tree2.y += update_speed(tree2);
} // end update_tree

void update_house (void) {
	int house_y = round(house.y);
	
	if (house_y > (LCD_Y + HOUSE_HEIGHT)) {
		house.x = rand_offroad_x();
		house.y = 0;
	}
	
	house.y += update_speed(house);
} // end update_house

void update_house2 (void) {
	int house2_y = round(house2.y);
	
	if (house2_y > (LCD_Y + HOUSE_HEIGHT)) {
		house2.x = rand_offroad_x();
		house2.y = 0;
	}
	
	house2.y += update_speed(house2);
} // end update_house2

void update_person (void) {
	int person_y = round(person.y);
	
	if (person_y > (LCD_Y + PERSON_HEIGHT)) {
		person.x = rand_offroad_x();
		person.y = 0;
	}
	
	person.y += update_speed(person);
} // end update_person

void update_person2 (void) {
	int person2_y = round(person2.y);
	
	if (person2_y > (LCD_Y + PERSON_HEIGHT)) {
		person2.x = rand_offroad_x();
		person2.y = 0;
	}
	
	person2.y += update_speed(person2);
} // end update_person2

void update_roadBlock (void) {
	int roadBlock_y = round(roadBlock.y);
	
	if (roadBlock_y > (LCD_Y + ROADBLOCK_HEIGHT)) {
		roadBlock.x = rand_onroad_x();
		roadBlock.y = 0;
	}
	
	roadBlock.y += update_speed(roadBlock);
} // end update_roadBlock

void update_roadBlock2 (void) {
	int roadBlock2_y = round(roadBlock2.y);
	
	if (roadBlock2_y > (LCD_Y + ROADBLOCK_HEIGHT)) {
		roadBlock2.x = rand_onroad_x();
		roadBlock2.y = 0;
	}
	
	roadBlock2.y += update_speed(roadBlock2);
} // end update_roadBlock2

void update_smallCar (void) {
	int smallCar_y = round(smallCar.y);
	
	if (smallCar_y > (LCD_Y + SMALLCAR_HEIGHT)) {
		smallCar.x = rand_onroad_x();
		smallCar.y = 0;
	}
	
	smallCar.y += update_speed(smallCar);
} // end update_smallCar

void update_smallCar2 (void) {
	int smallCar2_y = round(smallCar2.y);
	
	if (smallCar2_y > (LCD_Y + SMALLCAR_HEIGHT)) {
		smallCar2.x = rand_onroad_x();
		smallCar2.y = 0;
	}
	
	smallCar2.y += update_speed(smallCar2);
} // end update_smallCar2

void update_fuel (void) {
	int fuel_y = round(fuel.y);
	
	if (fuel_y >= 0) {
		fuel.y += update_speed(fuel);
	} 
	
	if (fuel_y > (LCD_Y + FUEL_HEIGHT + rand_fuel_y())) {
		fuel.x = rand_fuel_x();
		fuel.y = 0;
	}
} // end update_fuel

void update_finishLine (void) {
	if (distance_travelled >= MAX_DISTANCE) {
		finishLine.y += update_speed(finishLine);
	}
} // end update_finishLine

/**------------------------------------------------
*** CAR FUNCTIONS 
***---------------------------------------------**/

void update_car (void) {
	// If the speed > 0 then the car can move
	// If car on road then speed max 10
	// If car off road then speed max 3	
	
	if (up_debounce()) {
		accelerate_car();
	} else if (down_debounce() && (current_speed > 0)) {
		decelerate_car();
	} else if (current_speed > 1) {
		decelerate_car2();
	} else if (current_speed < 1) {
		accelerate_car2();
	}
	
	if (current_speed > 0.5) {
		update_steering();
	}
	
	// If the car is off the road but speed is > 3, change speed to 3
	check_offroad();
	
} // end update_car
	
void check_offroad (void) {
	int car_x = round(car.x);
	if ((car_x > 1) && (car_x < (LCD_X / 3 - 1)) &&  (current_speed > 3)) {
		current_speed = 3;
	}
	
	if ((car_x > ((LCD_X / 3) * 2 - 1)) && (current_speed > 3)) { 
		current_speed = 3;
	}	
} // end check_offroad

void accelerate_car (void ) {
	int car_x = round(car.x);
	if ((car_x > 0) && (car_x < (LCD_X / 3 - 1)) &&  (current_speed < 3)) {
		current_speed += 0.016;
	}
	
	if ((car_x > ((LCD_X / 3) * 2 - 1)) && (current_speed < 3)) { 
		current_speed += 0.016;
	}
	
	if ((car_x > (LCD_X / 3 - 1)) && ((car_x < (LCD_X / 3) * 2 - 1)) 
		&& (current_speed < 10)) {
			current_speed += 0.05;
	}	
} // end accelerate_car

void accelerate_car2 (void ) {
	int car_x = round(car.x);
	if ((car_x > 0) && (car_x < (LCD_X / 3 - 1))) {
		current_speed += 0.007;
	}
	
	if (car_x > ((LCD_X / 3) * 2 - 1)) { 
		current_speed += 0.007;
	}
	
	if ((car_x > (LCD_X / 3 - 1)) && ((car_x < (LCD_X / 3) * 2 - 1))) {
		current_speed += 0.01;
	}	
} // end accelerate_car2

void decelerate_car (void) {
	current_speed -= 0.12;
} // end decelerate_car

void decelerate_car2 (void) {
	int car_x = round(car.x);	
	if ((car_x > 0) && (car_x < (LCD_X / 3 - 1)) &&  (current_speed <= 3)) {
	current_speed -= 0.01;
	}
	
	if ((car_x > ((LCD_X / 3) * 2 - 1)) && (current_speed <= 3)) { 
		current_speed -= 0.01 ;
	}
	
	if ((car_x > (LCD_X / 3 - 1)) && ((car_x < (LCD_X / 3) * 2 - 1)) 
		&& (current_speed < 11)) {
			current_speed -= 0.11;
	}		
} // end decelerate_car2


void update_steering (void) {
	int car_x = round(car.x);

	if ((left_adc < (THRESHOLD - 100)) && (car_x > 1)) {
		car.dx = -0.5;
		if ((current_speed >= 0.5) && (current_speed < 2)) {
			car.dx = -(MAX_HORIZONTAL_SPEED/10);
		} else if ((current_speed >= 2) && (current_speed < 3)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 2);
		} else if ((current_speed >= 3) && (current_speed < 4)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 3);
		} else if ((current_speed >= 4) && (current_speed < 5)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 4);
		} else if ((current_speed >= 5) && (current_speed < 6)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 5);
		} else if ((current_speed >= 6) && (current_speed < 7)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 6);
		} else if ((current_speed >= 7) && (current_speed < 8)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 7);
		} else if ((current_speed >= 8) && (current_speed < 9)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 8);
		} else if ((current_speed >= 9) && (current_speed < 10)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 9);
		} else if (current_speed >= 10) {
			car.dx = -MAX_HORIZONTAL_SPEED;
		} else {
			car.dx = 0;
		}			
	} else if ((left_adc > (THRESHOLD + 100)) && (car_x < LCD_X - CAR_WIDTH - 1)) {
		car.dx = 0.5;
		if ((current_speed >= 0.5) && (current_speed < 2)) {
			car.dx = (MAX_HORIZONTAL_SPEED/10);
		} else if ((current_speed >= 2) && (current_speed < 3)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 2);
		} else if ((current_speed >= 3) && (current_speed < 4)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 3);
		} else if ((current_speed >= 4) && (current_speed < 5)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 4);
		} else if ((current_speed >= 5) && (current_speed < 6)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 5);
		} else if ((current_speed >= 6) && (current_speed < 7)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 6);
		} else if ((current_speed >= 7) && (current_speed < 8)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 7);
		} else if ((current_speed >= 8) && (current_speed < 9)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 8);
		} else if ((current_speed >= 9) && (current_speed < 10)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 9);
		} else if (current_speed >= 10) {
			car.dx = MAX_HORIZONTAL_SPEED;
		} else {
			car.dx = 0;
		}			
		
	} else if (left_debounce() && (car_x > 1)) {
		car.dx = -0.5;
		if ((current_speed >= 0.5) && (current_speed < 2)) {
			car.dx = -(MAX_HORIZONTAL_SPEED/10);
		} else if ((current_speed >= 2) && (current_speed < 3)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 2);
		} else if ((current_speed >= 3) && (current_speed < 4)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 3);
		} else if ((current_speed >= 4) && (current_speed < 5)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 4);
		} else if ((current_speed >= 5) && (current_speed < 6)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 5);
		} else if ((current_speed >= 6) && (current_speed < 7)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 6);
		} else if ((current_speed >= 7) && (current_speed < 8)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 7);
		} else if ((current_speed >= 8) && (current_speed < 9)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 8);
		} else if ((current_speed >= 9) && (current_speed < 10)) {
			car.dx = -((MAX_HORIZONTAL_SPEED/10) * 9);
		} else if (current_speed >= 10) {
			car.dx = -MAX_HORIZONTAL_SPEED;
		} else {
			car.dx = 0;
		}	
			
	} else if (right_debounce() && (car_x < LCD_X - CAR_WIDTH - 1)) {
		car.dx = 0.5;
		if ((current_speed >= 0.5) && (current_speed < 2)) {
			car.dx = (MAX_HORIZONTAL_SPEED/10);
		} else if ((current_speed >= 2) && (current_speed < 3)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 2);
		} else if ((current_speed >= 3) && (current_speed < 4)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 3);
		} else if ((current_speed >= 4) && (current_speed < 5)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 4);
		} else if ((current_speed >= 5) && (current_speed < 6)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 5);
		} else if ((current_speed >= 6) && (current_speed < 7)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 6);
		} else if ((current_speed >= 7) && (current_speed < 8)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 7);
		} else if ((current_speed >= 8) && (current_speed < 9)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 8);
		} else if ((current_speed >= 9) && (current_speed < 10)) {
			car.dx = ((MAX_HORIZONTAL_SPEED/10) * 9);
		} else if (current_speed >= 10) {
			car.dx = MAX_HORIZONTAL_SPEED;
		} else {
			car.dx = 0;
		}		
		
	} else {
		car.dx = 0;
	}
	
	car.x += car.dx;
} // end update_steering		

/**------------------------------------------------
*** COLLISION FUNCTIONS
***---------------------------------------------**/

bool sprites_collided (Sprite sprite1, Sprite sprite2, int sprite1_height, 
						int sprite1_width, int sprite2_height, int sprite2_width) {
	bool collided = true;
	
	int sprite1_top = round(sprite1.y),
		sprite1_bot = sprite1_top + sprite1_height - 1,
		sprite1_left = round(sprite1.x),
		sprite1_right = sprite1_left + sprite1_width - 1;
		
	int sprite2_top = round(sprite2.y),
		sprite2_bot = sprite2_top + sprite2_height - 1,
		sprite2_left = round(sprite2.x),
		sprite2_right = sprite2_left + sprite2_width - 1;
		
	if (sprite1_bot < sprite2_top) {
		collided = false;
	}
	
	if (sprite1_top > sprite2_bot) {
		collided = false;
	}
	
	if (sprite1_right < sprite2_left) {
		collided = false;
	}
	
	if (sprite1_left > sprite2_right) {
		collided = false;
	}
	
	return collided;
} // end sprites_collided

void update_collision (void) {
	if (condition > 0) {
		condition -= 1;
	}

	if (condition == 0) {
		game_state = GAME_OVER;
		return;
	}
	
	car_safe_spot();
	current_speed = 0;
	fuel_remaining = 100;
	crash = 25;
} // end update_collision

// AMS Topic08 Exercise4
void crash_collision (void) {
	if (crash > 0) {
		crash--;
		
		set_duty_cycle(750);
		
		LCD_CMD(lcd_set_x_addr, 0);
		LCD_CMD(lcd_set_y_addr, 0);

		// Visit each column of output bitmap		
		for (int bank = 0; bank < LCD_Y / 8; bank++) {
			// Visit each row of output bitmap
			for (int x = 0; x < LCD_X; x++) {
				uint8_t byte_to_write = 0;
				
				LCD_DATA(byte_to_write);
				
				screen_buffer[bank * LCD_X] = byte_to_write;
			}
		}
	}
} // end crash_collision

bool car_safe_spot (void) {
	bool safe = false;
	
	while ( !safe ) {
		car.x = rand_onroad_x();
		car.y = (LCD_Y - CAR_HEIGHT - 2);
		
		if (sprites_collided(car, tree, CAR_HEIGHT, CAR_WIDTH, TREE_HEIGHT, TREE_WIDTH)) {
			safe = false;
//		} else if (sprites_collided(car, tree2)) {
//			safe = false;
		} else if (sprites_collided(car, house, CAR_HEIGHT, CAR_WIDTH, HOUSE_HEIGHT, HOUSE_WIDTH)) {
			safe = false;
		} else if (sprites_collided(car, house2, CAR_HEIGHT, CAR_WIDTH, HOUSE_HEIGHT, HOUSE_WIDTH)) {
			safe = false;
		} else if (sprites_collided(car, person, CAR_HEIGHT, CAR_WIDTH, PERSON_HEIGHT, PERSON_WIDTH)) {
			safe = false;
		} else if (sprites_collided(car, person2, CAR_HEIGHT, CAR_WIDTH, PERSON_HEIGHT, PERSON_WIDTH)) {
			safe = false;
		} else if (sprites_collided(car, roadBlock, CAR_HEIGHT, CAR_WIDTH, ROADBLOCK_HEIGHT, ROADBLOCK_WIDTH)) {
			safe = false;
		} else if (sprites_collided(car, roadBlock2, CAR_HEIGHT, CAR_WIDTH, ROADBLOCK_HEIGHT, ROADBLOCK_WIDTH)) {
			safe = false;
		} else if (sprites_collided(car, smallCar, CAR_HEIGHT, CAR_WIDTH, SMALLCAR_HEIGHT, SMALLCAR_WIDTH)) {
			safe = false;
		} else if (sprites_collided(car, smallCar2, CAR_HEIGHT, CAR_WIDTH, SMALLCAR_HEIGHT, SMALLCAR_WIDTH)) {
			safe = false;
		} else if (sprites_collided(car, fuel, CAR_HEIGHT, CAR_WIDTH, FUEL_HEIGHT, FUEL_WIDTH)) {
			safe = false;
		} else {
			safe = true;
		}
	}
	return safe;
} // end car_safe_spot

void finishLine_collision (void) {
	int car_y = round(car.y);	
	int finishLine_y = round(finishLine.y);
	
	if (car_y == (finishLine_y - CAR_HEIGHT)) {
		game_state = GAME_OVER;
		return;
	}
} // end finishLine_collision

/**------------------------------------------------
*** REFUEL FUNCTIONS
***---------------------------------------------**/

bool fuel_collided (void) {
	bool collided = true;
	
	int left_car = round(car.x);
	int right_car = left_car + CAR_WIDTH - 1;
	int top_car = round(car.y);
	int bottom_car = top_car + CAR_HEIGHT - 1;
	
	int left_fuel = round(fuel.x);
	int right_fuel = left_fuel + FUEL_WIDTH - 1;
	int top_fuel = round(fuel.y);
	int bottom_fuel = top_fuel + FUEL_HEIGHT - 1;
	
	if (top_car < top_fuel - 2) {
		collided = false;
	}
	if (bottom_car > bottom_fuel + 2) {
		collided = false;
	}
	if (left_car > right_fuel + 2) {
		collided = false;
	}
	if (right_car < left_fuel - 2) {
		collided = false;
	}
	
	return collided;		
} // end fuel_collided

void begin_refuel (double time) {
	if (fuel_remaining >= MAX_FUEL) return;
	
	begin_refuel_amount = fuel_remaining;
	refuel_start_time = time;
	stage_fuel = (MAX_FUEL - begin_refuel_amount) / refuel_stages;
} // end begin_refuel

void refuel (double time) {
	double time_amount = (time - refuel_start_time) / 3;
	
	if (time_amount > 1) {
		time_amount = 1;
	}
	
	double fuel_amount = MAX_FUEL - begin_refuel_amount;
	fuel_remaining = begin_refuel_amount + fuel_amount * time_amount;
	
	if (fuel_remaining >= MAX_FUEL) {
		fuel_remaining = MAX_FUEL;
	}
} // end refuel

/**------------------------------------------------
*** GAME OVER FUNCTIONS
***---------------------------------------------**/

void do_game_over (void) {

	char *game_over_text = "GAME OVER";
	char *you_won_text = "YOU WON";
	char* play_again_text = "Centre to Play";
	char* exit_text = "Right Switch Exit";
	
	snprintf(time_string, 11, "Time: %02f", elapsed_time);
	sprintf(distance_string, "Distance: %0.01f", distance_travelled);
	draw_string(1, 10, time_string, FG_COLOUR);
	draw_string(1, 20, distance_string, FG_COLOUR);
	
	if (condition == 0) {
		draw_string(22, 2, game_over_text, FG_COLOUR);
	} else {
		draw_string(22, 2, you_won_text, FG_COLOUR);
	}

	draw_string(1, 30, play_again_text, FG_COLOUR);
	draw_string(1, 40, exit_text, FG_COLOUR);
	
	show_screen();
} // end do_game_over

void reset_variables (void) {
	new_game = true;

	overflow_counter = 0;
	condition = 3;
	current_speed = 0;
	fuel_remaining = 100.0;
	distance_travelled = 0;
} // end reset_variables

void do_end_game (void) {
	char *end_game_text1 = "Thanks for";
	char *end_game_text2 = "Playing!";
	draw_string(16, 17, end_game_text1, FG_COLOUR);
	draw_string(22, 27, end_game_text2, FG_COLOUR);
	
	show_screen();
} // end do_end_game
