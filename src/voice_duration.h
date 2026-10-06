#pragma once
#include <stdbool.h>
#include <stdint.h>

// English duration, 5 seconds through 24 hours. Output is unchanged on failure.
bool voice_duration_parse(const char *text, uint32_t *seconds);
