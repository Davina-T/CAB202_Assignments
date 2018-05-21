#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>
#include <curses.h>

/* THERE MUST BE AT LEAST 5 ITEMS OF SCENERY OR OBSTACLES (ALTOGETHER) 
IN VIEW AT ALL TIMES INC THE INITIAL FRAME */

// Define constants
#define DELAY (10) /* Millisecond delay */
#define CAR_WIDTH (4)
#define CAR_HEIGHT (4)
#define TREE_WIDTH (6)
#define TREE_HEIGHT (5)
#define HOUSE_WIDTH (9)
#define HOUSE_HEIGHT (6)
#define PERSON_WIDTH (5)
#define PERSON_HEIGHT (4)
#define ROADBLOCK_WIDTH (8)
#define ROADBLOCK_HEIGHT (1)
#define BUS_WIDTH (5)
#define BUS_HEIGHT (6)
#define FINISHLINE_WIDTH (21)
#define FINISHLINE_HEIGHT (1)
#define FUEL_WIDTH (6)
#define FUEL_HEIGHT (4)

#define MAX_SPEED (0.0005)
#define MAX_DISTANCE (1000)
#define MAX_FUEL (100)

// Define sprites
sprite_id car;
sprite_id tree;
sprite_id tree2;
sprite_id house;
sprite_id house2;
sprite_id person;
sprite_id person2;
sprite_id roadBlock;
sprite_id roadBlock2;
sprite_id bus;
sprite_id bus2;
sprite_id finishLine;
sprite_id fuel;

// Define global variables
/* Dashboard variables */
timer_id game_timer;
int milliseconds = 0;
int seconds = 0;
int minutes = 0;

int condition = 100;
int current_speed = 0;
double fuel_remaining = 100;
double distance_travelled = 0;
char * current_condition = "Excellent";

timer_id refuel_timer;
float begin_refuel_amount = 0;
int refuel_stages = 7;
float stage_fuel = 0;

// Define functions
void splash_screen (void);
void setup (void);
void draw_game (void);
void draw_borderDashboard (void);
void draw_road (void);
void process (void);
void update_timer (void);
int rand_offroad_x (void);
int rand_onroad_x (void);
int rand_fuel_x (void);
int rand_fuel_y (void);

void update_speed (sprite_id sprite);
void setup_car (void);
void update_car (int key);
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
void setup_bus (void);
void update_bus (void);
void setup_bus2 (void);
void update_bus2 (void);
void setup_fuel (void);
void update_fuel (void);

void move_object (void);

void update_distance (void);
void update_fuel_remaining (void);
void begin_refuel (void);
void refuel (void);

void setup_finishLine (void);
void update_finishLine (void);

bool sprites_collided (sprite_id sprite1, sprite_id sprite2);
void update_collision (void);
bool fuel_collided (sprite_id sprite1, sprite_id sprite2);
void finishLine_collision (void);
bool car_safe_spot (void);

void do_game_over (void);
void time_reset (void);
void pause (void);

// Game state
bool game_over = false; /* Set true when game is over */
bool update_screen = true; /* Set false to prevent screen updates */
bool new_game = true;
bool not_refuelling = true;

// Define character variables

char * splash_screen_image = 
/**/	"                  Race to Zombie Mountain.                        "
/**/	"                                                                  "
/**/	"                   By Davina Tan, N9741127                        "
/**/	"                                                                  "
/**/    " Use the left and right keys to move the car and avoid obstacles. "
/**/    "     Use the up and down keys to change the speed of the car.     "
/**/	"                                                                  "
/**/	"     Collect fuel by driving next to a depot while on the road.   "
/**/	"                     Press 'p' to pause.                          "        
/**/	"                                                                  "
/**/	"                 Press any key to continue...                     ";

char * car_image = 
/**/	"o  o"
/**/	"|--|"
/**/	"|--|"
/**/	"o  o"; // 4x4

/* Scenery drawings */
char * tree_image = 
/**/	"  oo  "
/**/	"oooooo"
/**/	"oooooo"
/**/	"  ||  "
/**/	"  ||  "; // 6x5 

char * house_image =
/**/	"  _____  "
/**/	" |     | "
/**/	"|   O   |"
/**/	"|  __   |"
/**/	"| |  |  |"
/**/	"| | o|  |"; // 9x6 

char * person_image =
/**/	"  O  "
/**/	"--|--"
/**/	"  |  "
/**/	" | | "; // 5x4

/* Obstacle drawings */
char * roadBlock_image = 
/**/	"==T--T=="; // 8x1

char * bus_image = 
/**/	"O   O"
/**/	"|---|"
/**/	"|> <|"
/**/	"|> <|"
/**/	"|___|"
/**/	"O   O"; // 5x6


char * finishLine_image = 
/**/ "~~~~[FINISH LINE]~~~~"; // 21x1 

char * fuel_image = 
/**/	"_____ "
/**/	"|ooo| "
/**/	"|UUU|R"
/**/	"|---| "; // 6x4

// Setup game
void setup (void) {
	int now = get_current_time();
	srand(now);
	
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
	setup_bus();
	setup_bus2();
	setup_finishLine();
	setup_fuel();
	
	game_timer = create_timer(10);
	draw_game();
} // end setup


// Timer function
void update_timer (void) {
		if (timer_expired(game_timer)) {
			timer_reset(game_timer);
			milliseconds++;
		}
		
		if (milliseconds >= 100) {
			seconds++;
			milliseconds = 0;
		}
		
		if (seconds >= 60) {
			minutes++;
			seconds = 0;
		}	
} // end update_timer 

void splash_screen (void) {
	clear_screen();
	
	// find screen_width() and screen_height() of splash screen sprite
	int splash_screen_height = 11;
	int splash_screen_width = strlen(splash_screen_image) / splash_screen_height;
	
	// create splash screen sprite and draw it
	sprite_id splash_box = sprite_create( (screen_width() - splash_screen_width) / 2,
										  (screen_height() - splash_screen_height) / 2,
										   splash_screen_width, splash_screen_height, splash_screen_image);
	sprite_draw(splash_box);
	
	// show it on the screen 
	show_screen();
	
	wait_char();
	new_game = false;
} // end splash_screen

void pause (void) {
	clear_screen();
	draw_formatted(screen_width() / 2 - 6, screen_height() / 2 - 9,
			"PAUSED");
	draw_formatted(screen_width() / 2 - 10, screen_height() / 2 - 5, 
			" Time = %02i:%02i:%02i", minutes, seconds, milliseconds);
	draw_formatted(screen_width() / 2 - 12, screen_height() / 2 - 4,
			"Distance travelled: %0.0lf", distance_travelled);
	draw_formatted(screen_width() / 2 - 14, screen_height() / 2,
			"Press any key to continue...");
	show_screen();
	wait_char();
} // end pause

// Function to play the game
void process (void) {
	
	/* Implement a quit option for testing */
	int key = get_char();
	
	if (key == 'q') {
		game_over = true;
		wait_char();
		return;
	}
	
	/* Implement a pause function */
	if (key == 'p') {
		pause();
	}

	update_car(key); // takes what key is pressed into the function
	update_tree();
	update_tree2();
	update_house();
	update_house2();
	update_person();
	update_person2();
	update_roadBlock();
	update_roadBlock2();
	update_bus();
	update_bus2();
	update_fuel();
	
	update_distance();
	update_fuel_remaining();
	
	update_finishLine();
	
	update_timer();
	
	if (sprites_collided(car, tree)) {
		update_collision();
	}
	
	if (sprites_collided(car, tree2)) {
		update_collision();
	}
	
	if (sprites_collided(car, house)) {
		update_collision();
	}
	
	if (sprites_collided(car, house2)) {
		update_collision();
	}
	
	if (sprites_collided(car, person)) {
		update_collision();
	}
	
	if (sprites_collided(car, person2)) {
		update_collision();
	}
	
	if (sprites_collided(car, roadBlock)) {
		update_collision();
	}
	
	if (sprites_collided(car, roadBlock2)) {
		update_collision();
	}
	
	if (sprites_collided(car, bus)) {
		update_collision();
	}
	
	if (sprites_collided(car, bus2)) {
		update_collision();
	}
	
	if (sprites_collided(car, fuel)) {
		condition = 0;
		do_game_over();
	}
	
	if (fuel_collided(car, fuel)) {
		if (not_refuelling) {
			if (current_speed == 0) {
				not_refuelling = false;
				begin_refuel();
			} else {
				not_refuelling = true;
			}
		} else {
			refuel();
		}
	} else {
		not_refuelling = true;
	}
	
	finishLine_collision();
	
	draw_game();
} // end process

// Main program
int main (void) {
	setup_screen();
	
	if (new_game == true) {
		setup();
		show_screen();
		splash_screen();
		
		// while the game isn't over, play the game
		while (!game_over) {
			process();
			
			if (update_screen) {
				show_screen();
			}
		}

		return 0;
		
	}
} // end main

//
void setup_car (void) {
	int car_x = (screen_width() - CAR_WIDTH) / 2;
	int car_y = (screen_height() - CAR_HEIGHT - 5);
	car = sprite_create(car_x, car_y, CAR_WIDTH, CAR_HEIGHT, car_image);
} // end setup_car

void setup_tree (void) {
	int tree_x = rand_offroad_x();
	int tree_y = (rand() % (screen_height() - TREE_HEIGHT - 1));
	tree = sprite_create(tree_x, tree_y, TREE_WIDTH, TREE_HEIGHT, tree_image);
} // end setup_tree

void setup_tree2 (void) {
	int tree2_x = rand_offroad_x();
	int tree2_y = (rand() % (screen_height() - TREE_HEIGHT - 1));
	tree2 = sprite_create(tree2_x, tree2_y, TREE_WIDTH, TREE_HEIGHT, tree_image);
} // end setup_tree2

void setup_house (void) {
	int house_x = rand_offroad_x();
	int house_y = (rand() % (screen_height() - HOUSE_HEIGHT - 1));
	house = sprite_create(house_x, house_y, HOUSE_WIDTH, HOUSE_HEIGHT, house_image);
} // end setup_house

void setup_house2 (void) {
	int house2_x = rand_offroad_x();
	int house2_y = (rand() % (screen_height() - HOUSE_HEIGHT - 1));
	house2 = sprite_create(house2_x, house2_y, HOUSE_WIDTH, HOUSE_HEIGHT, house_image);
} // end setup_house

void setup_person (void) {
	int person_x = rand_offroad_x();
	int person_y = (rand() % (screen_height() - PERSON_HEIGHT - 1));
	person = sprite_create(person_x, person_y, PERSON_WIDTH, PERSON_HEIGHT, person_image);
} // end setup_person

void setup_person2 (void) {
	int person2_x = rand_offroad_x();
	int person2_y = (rand() % (screen_height() - PERSON_HEIGHT - 1));
	person2 = sprite_create(person2_x, person2_y, PERSON_WIDTH, PERSON_HEIGHT, person_image);
} // end setup_person2

void setup_roadBlock (void) {
	int roadBlock_x = rand_onroad_x();
	int roadBlock_y = (rand() % (screen_height() - ROADBLOCK_HEIGHT - 1));
	roadBlock = sprite_create(roadBlock_x, roadBlock_y, ROADBLOCK_WIDTH, ROADBLOCK_HEIGHT, roadBlock_image);
} // end setup_roadBlock

void setup_roadBlock2 (void) {
	int roadBlock2_x = rand_onroad_x();
	int roadBlock2_y = (rand() % (screen_height() - ROADBLOCK_HEIGHT - 1));
	roadBlock2 = sprite_create(roadBlock2_x, roadBlock2_y, ROADBLOCK_WIDTH, ROADBLOCK_HEIGHT, roadBlock_image);
} // end setup_roadBlock2

void setup_bus (void) {
	int bus_x = rand_onroad_x();
	int bus_y = (rand() % (screen_height() - BUS_HEIGHT - 1));
	bus = sprite_create(bus_x, bus_y, BUS_WIDTH, BUS_HEIGHT, bus_image);
} // end setup_bus

void setup_bus2 (void) {
	int bus2_x = rand_onroad_x();
	int bus2_y = (rand() % (screen_height() - BUS_HEIGHT - 1));
	bus2 = sprite_create(bus2_x, bus2_y, BUS_WIDTH, BUS_HEIGHT, bus_image);
} // end setup_bus2

void setup_finishLine (void) {
	int finishLine_x = ((screen_width() / 2) - (FINISHLINE_WIDTH / 2));
	int finishLine_y = 0;
	finishLine = sprite_create(finishLine_x, finishLine_y, FINISHLINE_WIDTH, FINISHLINE_HEIGHT, finishLine_image);
} // end setup_finishLine

void setup_fuel (void) {
	int fuel_x = rand_fuel_x();
	int fuel_y = 0;
	fuel = sprite_create(fuel_x, fuel_y, FUEL_WIDTH, FUEL_HEIGHT, fuel_image);
} // end setup_fuel

void draw_game (void) {
	// clear the screen before drawing every thing on it
	clear_screen();
	draw_road();
	
	sprite_draw(car);
	sprite_draw(tree);
	sprite_draw(tree2);
	sprite_draw(house);
	sprite_draw(house2);
	sprite_draw(person);
	sprite_draw(person2);
	sprite_draw(roadBlock);
	sprite_draw(roadBlock2);
	sprite_draw(bus);
	sprite_draw(bus2);
	sprite_draw(finishLine);
	sprite_draw(fuel);
	
	draw_borderDashboard();
} // end draw_game

void draw_borderDashboard (void) {
	int left = 0;
	int right = (screen_width() - 1);
	int top = 0;
	int bot = (screen_height() - 1);
	char border_char = '*';

	// draw the border around the whole screen
	draw_line(left, top, right, top, border_char);
	draw_line(right, top, right, bot, border_char);
	draw_line(right, bot, left, bot, border_char);
	draw_line(left, bot, left, top, border_char);

	// draw the dashboard border
	draw_line(left, (top + 1), right, (top + 1), border_char);
	draw_line(left, (top + 2), right, (top + 2), border_char);
	draw_line(left, (top + 3), right, (top + 3), border_char);
	
	// draw the information in the dashboard
	draw_formatted((left+1), (top+1), " Time = %02i:%02i:%02i", minutes, seconds, milliseconds);
	draw_formatted((left+17), (top+1), " * Condition = %2s", current_condition);
	draw_formatted((left+41), (top+1), " * Current Speed = %d", current_speed);
	draw_formatted((left+1), (top+2), " Fuel Remaining = %0.01f", fuel_remaining);
	draw_formatted((left+23), (top+2), " * Distance Travelled = %0.0lf\t", distance_travelled);
} // end draw_borderDashboard

void draw_road (void) {
	int top = 0;
	int bot = (screen_height() - 1);
	int road_left = (screen_width() / 3);
	int road_right = ((screen_width() / 3) * 2 - 1);
	char road_char = '|';
	
	// draw the road
	draw_line(road_left, (top+3), road_left, (bot-1), road_char);
	draw_line(road_right, (top+3), road_right, (bot-1), road_char);
} // end draw_road

void update_car (int key) {
	// Get the current coordinates of the car and check it is in border before moving
	// Only need to get x coordinate because car does not move on the vertical plane
	int car_x = round(sprite_x(car));
	
	// If the car is off the road but speed is > 3, change speed to 3
	if ((car_x > 1) && (car_x < (screen_width() / 3 - 1)) &&  (current_speed > 3)) {
		current_speed = 3;
	}
	
	if ((car_x > ((screen_width() / 3) * 2 - 1)) && (current_speed > 3)) { 
		current_speed = 3;
	}
		
	// If the speed > 0 then the car can move
	// If car on road then speed max 10
	// If car off road then speed max 3	
	
	if (KEY_UP == key) {
		if ((car_x > 0) && (car_x < (screen_width() / 3 - 1)) &&  (current_speed < 3)) {
			current_speed++;
		}
		
		if ((car_x > ((screen_width() / 3) * 2 - 1)) && (current_speed < 3)) { 
			current_speed++;
		}
		
		if ((car_x > (screen_width() / 3 - 1)) && ((car_x < (screen_width() / 3) * 2 - 1)) 
			&& (current_speed < 10)) {
				current_speed++;
		}
		
	}
	
	if ((KEY_DOWN == key) && (current_speed > 0)) {
		current_speed--;
	}
	
	if (current_speed > 0) {
		
		if ((KEY_LEFT == key) && (car_x > 1)) {
			sprite_move(car, -1, 0);
		}
		
		if ((KEY_RIGHT == key) && (car_x < screen_width() - CAR_WIDTH - 1)) {
			sprite_move(car, +1, 0);
		}
	}
} // end update_car

int rand_offroad_x (void) {
	// choose a random position off the road on both sides
	int offroad1_max = (screen_width() / 3 - 10);
	int offroad1_min = 1;
	int offroad1_num = rand() % (offroad1_max - offroad1_min + 1) + offroad1_min;
	
	int offroad2_max = (screen_width() - 10);
	int offroad2_min = ((screen_width() / 3) * 2 + 1);
	int offroad2_num = rand() % (offroad2_max - offroad2_min + 1) + offroad2_min;
	
	// randomly choose between the two random numbers
	int offroad_array[] = {offroad1_num, offroad2_num};
	
	int random_choice = rand() % 2;
	
	int random_pos = offroad_array[random_choice];

	return random_pos;
} // end rand_offroad_x 

int rand_onroad_x (void) {
	int onroad_max = ((screen_width() / 3) * 2 - 8);
	int onroad_min = (screen_width() / 3 + 1);
	int onroad_num = rand() % (onroad_max - onroad_min + 1) + onroad_min;

	return onroad_num;
} // end rand_onroad_x

int rand_fuel_x (void) {
	// choose a random position on each side of road 
	int left_x = (screen_width() / 3 - FUEL_WIDTH);
	int right_x = ((screen_width() / 3) * 2);
	
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

void update_speed (sprite_id sprite) {
	if (current_speed == 1) {
		sprite_move(sprite, 0, (MAX_SPEED/10));
	} else if (current_speed == 2) {
		sprite_move(sprite, 0, (MAX_SPEED/10)*2);
	} else if (current_speed == 3) {
		sprite_move(sprite, 0, (MAX_SPEED/10)*3);
	} else if (current_speed == 4) {
		sprite_move(sprite, 0, (MAX_SPEED/10)*4);
	} else if (current_speed == 5) {
		sprite_move(sprite, 0, (MAX_SPEED/10)*5);
	} else if (current_speed == 6) {
		sprite_move(sprite, 0, (MAX_SPEED/10)*6);
	} else if (current_speed == 7) {
		sprite_move(sprite, 0, (MAX_SPEED/10)*7);
	} else if (current_speed == 8) {
		sprite_move(sprite, 0, (MAX_SPEED/10)*8);
	} else if (current_speed == 9) {
		sprite_move(sprite, 0, (MAX_SPEED/10)*9);
	} else if (current_speed == 10) {
		sprite_move(sprite, 0, MAX_SPEED);
	} 
} // end update_speed

void update_tree (void) {
	int tree_y = round(sprite_y(tree));
	
	if (tree_y > (screen_height() + TREE_HEIGHT)) {
		sprite_move_to(tree, rand_offroad_x(), 0);
	}
	
	update_speed(tree);
} // end update_tree

void update_tree2 (void) {
	int tree2_y = round(sprite_y(tree2));
	
	if (tree2_y > (screen_height() + TREE_HEIGHT)) {
		sprite_move_to(tree2, rand_offroad_x(), 0);
	}
	
	update_speed(tree2);
} // end update_tree

void update_house (void) {
	int house_y = round(sprite_y(house));
	
	if (house_y > (screen_height() + HOUSE_HEIGHT)) {
		sprite_move_to(house, rand_offroad_x(), 0);
	}
	
	update_speed(house);
} // end update_house

void update_house2 (void) {
	int house2_y = round(sprite_y(house2));
	
	if (house2_y > (screen_height() + HOUSE_HEIGHT)) {
		sprite_move_to(house2, rand_offroad_x(), 0);
	}
	
	update_speed(house2);
} // end update_house2

void update_person (void) {
	int person_y = round(sprite_y(person));
	
	if (person_y > (screen_height() + PERSON_HEIGHT)) {
		sprite_move_to(person, rand_offroad_x(), 0);
	}
	
	update_speed(person);
} // end update_person

void update_person2 (void) {
	int person2_y = round(sprite_y(person2));
	
	if (person2_y > (screen_height() + PERSON_HEIGHT)) {
		sprite_move_to(person2, rand_offroad_x(), 0);
	}
	
	update_speed(person2);
} // end update_person2

void update_roadBlock (void) {
	int roadBlock_y = round(sprite_y(roadBlock));
	
	if (roadBlock_y > (screen_height() + ROADBLOCK_HEIGHT)) {
		sprite_move_to(roadBlock, rand_onroad_x(), 0);
	}
	
	update_speed(roadBlock);
} // end update_roadBlock

void update_roadBlock2 (void) {
	int roadBlock2_y = round(sprite_y(roadBlock2));
	
	if (roadBlock2_y > (screen_height() + ROADBLOCK_HEIGHT)) {
		sprite_move_to(roadBlock2, rand_onroad_x(), 0);
	}
	
	update_speed(roadBlock2);
} // end update_roadBlock2

void update_bus (void) {
	int bus_y = round(sprite_y(bus));
	
	if (bus_y > (screen_height() + BUS_HEIGHT)) {
		sprite_move_to(bus, rand_onroad_x(), 0);
	}
	
	update_speed(bus);
} // end update_bus

void update_bus2 (void) {
	int bus2_y = round(sprite_y(bus2));
	
	if (bus2_y > (screen_height() + BUS_HEIGHT)) {
		sprite_move_to(bus2, rand_onroad_x(), 0);
	}
	
	update_speed(bus2);
} // end update_bus2

void update_distance (void) {

	if (current_speed == 1) {
		distance_travelled += 0.0001;
	} else if (current_speed == 2) {
		distance_travelled += 0.0002;
	} else if (current_speed == 3) {
		distance_travelled += 0.0003;
	} else if (current_speed == 4) {
		distance_travelled += 0.0004;
	} else if (current_speed == 5) {
		distance_travelled += 0.0005;
	} else if (current_speed == 6) {
		distance_travelled += 0.0006;
	} else if (current_speed == 7) {
		distance_travelled += 0.0007;
	} else if (current_speed == 8) {
		distance_travelled += 0.0008;
	} else if (current_speed == 9) {
		distance_travelled += 0.0009;
	} else if (current_speed == 10) {
		distance_travelled += 0.001;
	}
} // end update_distance

void update_finishLine (void) {
	if (distance_travelled >= MAX_DISTANCE) {
		update_speed(finishLine);
	}
} // end update_finishLine

void update_fuel (void) {
	int fuel_y = round(sprite_y(fuel));
	
	if (fuel_y >= 0) {
		update_speed(fuel);
	} 
	
	if (fuel_y > (screen_height() + FUEL_HEIGHT + rand_fuel_y())) {
		sprite_move_to(fuel, rand_fuel_x(), 0);
	}
} // end update_fuel

void update_fuel_remaining (void) {
	
	if (fuel_remaining > 1) {
		if (current_speed == 1) {
			fuel_remaining -= 0.00001;
		} else if (current_speed == 2) {
			fuel_remaining -= 0.00002;
		} else if (current_speed == 3) {
			fuel_remaining -= 0.00003;
		} else if (current_speed == 4) {
			fuel_remaining -= 0.00004;
		} else if (current_speed == 5) {
			fuel_remaining -= 0.00005;
		} else if (current_speed == 6) {
			fuel_remaining -= 0.00006;
		} else if (current_speed == 7) {
			fuel_remaining -= 0.00007;
		} else if (current_speed == 8) {
			fuel_remaining -= 0.00008;
		} else if (current_speed == 9) {
			fuel_remaining -= 0.00009;
		} else if (current_speed == 10) {
			fuel_remaining -= 0.0001;
		} 
	}
} // end update_fuel_remaining

//
bool sprites_collided (sprite_id sprite1, sprite_id sprite2) {
	bool collided = true;
	
	int sprite1_top = round(sprite_y(sprite1)),
		sprite1_bot = sprite1_top + sprite_height(sprite1) - 1,
		sprite1_left = round(sprite_x(sprite1)),
		sprite1_right = sprite1_left + sprite_width(sprite1) - 1;
		
	int sprite2_top = round(sprite_y(sprite2)),
		sprite2_bot = sprite2_top + sprite_height(sprite2) - 1,
		sprite2_left = round(sprite_x(sprite2)),
		sprite2_right = sprite2_left + sprite_width(sprite2) - 1;
		
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

bool fuel_collided (sprite_id sprite1, sprite_id sprite2) {
	bool collided = true;

	int sprite1_y = round(sprite_y(sprite1));
	int sprite2_y = round(sprite_y(sprite2));
	int sprite1_height = sprite_height(sprite1);
	int sprite2_height = sprite_height(sprite2);
	
	if (sprite2_y >= sprite1_y + sprite1_height) {
		collided = false;
	}
	
	if (sprite1_y >= sprite2_y + sprite2_height) {
		collided = false;
	}
	
	return collided;	
} // end fuel_collided

void begin_refuel (void) {
	if (fuel_remaining >= MAX_FUEL) return;
	
	begin_refuel_amount = fuel_remaining;
	if (refuel_timer == NULL) {
		refuel_timer = create_timer(3 * MILLISECONDS / refuel_stages);
	} else {
		timer_reset(refuel_timer);
	}
	stage_fuel = (MAX_FUEL - begin_refuel_amount) / refuel_stages;
} // end begin_refuel

void refuel (void) {
	if (refuel_timer == NULL) return;
	if (timer_expired(refuel_timer)) {
		fuel_remaining += stage_fuel;
	}
	if (fuel_remaining >= MAX_FUEL) {
		fuel_remaining = MAX_FUEL;
	}
} // end refuel

void update_collision (void) {
	if (condition > 0) {
		condition -= 25;
		if (condition == 75) {
			current_condition = "Great";
		} else if (condition == 50) {
			current_condition = "Good";
		} else {
			current_condition = "Bad";
		}
	}
	
	if (condition == 0) {
		do_game_over();
		return;
	}
	
	car_safe_spot();
	current_speed = 0;
	fuel_remaining = 100;
} // end update_collision

bool car_safe_spot (void) {
	bool safe = false;
	
	while ( !safe ) {
		sprite_move_to(car, rand_onroad_x(), (screen_height() - CAR_HEIGHT - 5));
		if (sprites_collided(car, tree)) {
			safe = false;
		} else if (sprites_collided(car, tree2)) {
			safe = false;
		} else if (sprites_collided(car, house)) {
			safe = false;
		} else if (sprites_collided(car, house2)) {
			safe = false;
		} else if (sprites_collided(car, person)) {
			safe = false;
		} else if (sprites_collided(car, person2)) {
			safe = false;
		} else if (sprites_collided(car, roadBlock)) {
			safe = false;
		} else if (sprites_collided(car, roadBlock2)) {
			safe = false;
		} else if (sprites_collided(car, bus)) {
			safe = false;
		} else if (sprites_collided(car, bus2)) {
			safe = false;
		} else if (sprites_collided(car, fuel)) {
			safe = false;
		} else {
			safe = true;
		}
	}
	return safe;
} // end car_safe_spot

void finishLine_collision (void) {
	int car_y = round(sprite_y(car));
	int finishLine_y = round(sprite_y(finishLine));
	
	if (car_y == (finishLine_y - CAR_HEIGHT)) {
		do_game_over();
		return;
	}
} // end finishLine_collision

void do_game_over (void) {
	bool valid_input = false;
	game_over = true;
	clear_screen();
	
	while(!valid_input) {

		if (condition == 0) {
			draw_formatted(screen_width() / 2 - 6, screen_height() / 2 - 10, "GAME OVER!");
		} else {
			draw_formatted(screen_width() / 2 - 6, screen_height() / 2 - 10, "YOU WON!");
		}
		
		draw_formatted(screen_width() / 2 - 10, screen_height() / 2 - 5, 
					   " Time = %02i:%02i:%02i", minutes, seconds, milliseconds);
		draw_formatted(screen_width() / 2 - 12, screen_height() / 2 - 4,
					   "Distance travelled: %0.0lf", distance_travelled);
		
		draw_formatted(screen_width() / 2 - 14, screen_height() / 2,
					   "Would you like to play again?");
		draw_formatted(screen_width() / 2 - 8, screen_height() / 2 + 2,
					   "<Y>es or <N>o");
	
		show_screen();

		int key = get_char();
		
		if (!((key == 'n') || (key == 'y'))) {
			valid_input = false;
		} else {
			valid_input = true;
			if (key == 'y') {
				new_game = true;
				game_over = false;
				
				time_reset();
				condition = 100;
				current_condition = "Excellent";
				current_speed = 0;
				fuel_remaining = 100;
				distance_travelled = 0;
				
				main();
			}
		}
	}
} // end do_game_over

void time_reset (void) {
	milliseconds = 0;
	seconds = 0;
} // end time_reset
