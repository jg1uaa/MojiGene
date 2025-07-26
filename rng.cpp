// SPDX-License-Identifier: WTFPL

#include <random>
extern "C" {
#include "rng.h"
}

std::random_device rd;
std::mt19937 r(rd());

extern "C" void initialize_random_generator(void)
{
	// do nothing
}

extern "C" int random_value(int min, int max)
{
	std::uniform_int_distribution<int> d(min, max);

	return d(r);
}
