#pragma once
#include <pebble.h>

typedef bool (*VoiceTimerCreateCallback)(uint32_t seconds);
void voice_timer_init(VoiceTimerCreateCallback create_callback);
void voice_timer_start(void);
void voice_timer_deinit(void);
