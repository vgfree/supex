#pragma once

#include <stdint.h>
#include <unistd.h>

int get_length_of_road(uint64_t roadid);
uint64_t get_nextrandomroad_of_road(uint64_t roadid);
