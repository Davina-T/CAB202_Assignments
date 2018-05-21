#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cab202_graphics.h>
#include <cab202_sprites.h>
#include <cab202_timers.h>

char * condition[4] = {"Excellent", "Great", "Good", "Bad"};

void stringtest (void) {
	
	printf(" * Condition = %2s", condition[0]);
	printf(" * Condition = %2s", condition[1]);
	printf(" * Condition = %2s", condition[2]);	
	printf(" * Condition = %2s", condition[3]);	
}
	

int main( void ) {
	
	stringtest();
	
	return 0;
}
