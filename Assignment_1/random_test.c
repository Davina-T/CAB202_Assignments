#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>

void randomness (void) {
	int now = get_current_time();
	srand(now);
}

void rand_offroad_x (void) {
	// choose a random position off the road on both sides
	int offroad1_max = 20;
	int offroad1_min = 1;
	int offroad1_num = rand() % (offroad1_max - offroad1_min + 1) + offroad1_min;
	
	int offroad2_max = 60;
	int offroad2_min = 41;
	int offroad2_num = rand() % (offroad2_max - offroad2_min + 1) + offroad2_min;
	
	// randomly choose between the two random numbers
	int offroad_array[] = {offroad1_num, offroad2_num};
	
	int random_choice = rand() % 2;
	
	int random_pos = offroad_array[random_choice];

	
	printf("Random number is : %2i\n", random_pos);
}

void rand_onroad_x (void) {
	int onroad_max = 20; //((screen_width() / 3) * 2 - 1);
	int onroad_min = 1; //(screen_width() / 3 - 1);
	int onroad_num = rand() % (onroad_max - onroad_min + 1) + onroad_min;
	
	printf ("Random number is : %2i\n", onroad_num);
}

int main( void ) {
	
	randomness();
	
	rand_offroad_x();
	rand_offroad_x();
	rand_offroad_x();
	rand_offroad_x();
	rand_onroad_x();
	rand_onroad_x();
	rand_onroad_x();
	rand_onroad_x();
	
	return 0;
}
