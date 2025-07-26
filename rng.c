// SPDX-License-Identifier: WTFPL

#include <time.h>
#include <stdlib.h>
#include "rng.h"

void initialize_random_generator(void)
{
	time_t t;

	t = time(NULL);
	srand(t);
}

int random_value(int min, int max)
{
	int n = max - min + 1;

	return (rand() % n) + min;
}
