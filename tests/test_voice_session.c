#include <assert.h>
#include <stdio.h>
#include "../src/voice_timer.c"
#undef printf

static unsigned checks, starts, creates;
static bool phone_available = true, accept_timer = true, error_visible;
static uint32_t created_seconds;
static const char *error_message;
static DictationSessionStatus start_status = DictationSessionStatusSuccess;
static DictationSessionStatusCallback captured_callback;
#define CHECK(x) do { assert(x); ++checks; } while (0)

DictationSession *dictation_session_create(uint32_t size, DictationSessionStatusCallback callback, void *context) {
  CHECK(size == 256);
  captured_callback = callback;
  return phone_available ? (DictationSession*)1 : NULL;
}
void dictation_session_enable_confirmation(DictationSession *session, bool enabled) { CHECK(enabled); }
void dictation_session_enable_error_dialogs(DictationSession *session, bool enabled) { CHECK(!enabled); }
DictationSessionStatus dictation_session_start(DictationSession *session) { ++starts; return start_status; }
void dictation_session_destroy(DictationSession *session) { CHECK(session != NULL); }
Window *window_create(void) { return (Window*)2; }
Layer *window_get_root_layer(const Window *window) { return (Layer*)3; }
GRect layer_get_bounds(const Layer *layer) { return GRect(0, 0, 144, 168); }
TextLayer *text_layer_create(GRect bounds) { return (TextLayer*)4; }
void window_destroy(Window *window) {}
void text_layer_set_font(TextLayer *text, GFont font) {}
GFont fonts_get_system_font(const char *key) { return (GFont)5; }
void text_layer_set_text_alignment(TextLayer *text, GTextAlignment alignment) {}
void layer_add_child(Layer *parent, Layer *child) {}
Layer *text_layer_get_layer(TextLayer *text) { return (Layer*)6; }
void text_layer_set_text(TextLayer *text, const char *message) { error_message = message; }
bool window_stack_contains_window(Window *window) { return error_visible; }
void window_stack_push(Window *window, bool animated) { error_visible = true; }
bool window_stack_remove(Window *window, bool animated) { error_visible = false; return true; }
void text_layer_destroy(TextLayer *text) {}
void app_log(uint8_t level, const char *file, int line, const char *fmt, ...) {}

static bool create(uint32_t seconds) {
  ++creates;
  created_seconds = seconds;
  return accept_timer;
}

int main(void) {
  voice_timer_init(create);
  phone_available = false;
  voice_timer_start();
  CHECK(starts == 0 && creates == 0 && error_visible);
  CHECK(strstr(error_message, "Voice unavailable") != NULL);
  error_visible = false;
  phone_available = true;
  voice_timer_start();
  CHECK(starts == 1 && s_active);
  voice_timer_start();
  CHECK(starts == 1); // Holding/re-entering does not start duplicate sessions.
  captured_callback(s_session, DictationSessionStatusSuccess, "15 minutes", NULL);
  CHECK(creates == 1 && created_seconds == 900 && !s_active);
  CHECK(!error_visible);
  voice_timer_start();
  captured_callback(s_session, DictationSessionStatusFailureTranscriptionRejected, NULL, NULL);
  CHECK(creates == 1 && !error_visible && !s_active);
  voice_timer_start();
  captured_callback(s_session, DictationSessionStatusFailureConnectivityError, NULL, NULL);
  CHECK(creates == 1 && error_visible && !s_active);
  error_visible = false;
  voice_timer_start();
  captured_callback(s_session, DictationSessionStatusSuccess, "two days", NULL);
  CHECK(creates == 1 && error_visible);
  CHECK(strstr(error_message, "Say a duration") != NULL);
  error_visible = false;
  accept_timer = false;
  voice_timer_start();
  captured_callback(s_session, DictationSessionStatusSuccess, "30 minutes", NULL);
  CHECK(creates == 2 && created_seconds == 1800 && error_visible);
  CHECK(strstr(error_message, "Clear a timer") != NULL);
  error_visible = false;
  start_status = DictationSessionStatusFailureInternalError;
  voice_timer_start();
  CHECK(!s_active && error_visible);
  voice_timer_deinit();
  CHECK(!s_session && !s_error_window && !s_error_text && !error_visible);
  printf("Passed %u speech session checks.\n", checks);
  return 0;
}
