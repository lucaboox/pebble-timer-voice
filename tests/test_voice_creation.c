#include <assert.h>
#include <stdio.h>
#define main static __attribute__((unused)) watch_app_main
#include "../src/main.c"
#undef main
#undef printf

static unsigned checks, voice_starts, detail_pushes, pin_sends, pulses, refreshes, manual_opens;
static bool alert_visible;
static CountdownTimer *displayed;
static CountdownTimer *alarm_timer, *edited_timer;
static unsigned popup_closes, detail_closes, setting_closes, pin_deletes;
static unsigned clock_offset;
static int32_t saved_count;
static unsigned char saved_data[COUNTDOWN_TIMERS_MAX][256];
static size_t saved_sizes[COUNTDOWN_TIMERS_MAX];
#define CHECK(x) do { assert(x); ++checks; } while (0)

void voice_timer_start(void) { ++voice_starts; }
void setting_window_set_timer(SettingWindow *window, CountdownTimer *timer) { CHECK(timer == NULL); edited_timer = timer; }
void setting_window_push(SettingWindow *window, bool animated) { ++manual_opens; }
void setting_window_pop(SettingWindow *window, bool animated) { ++setting_closes; }
CountdownTimer *setting_window_get_timer(SettingWindow *window) { return edited_timer; }
status_t persist_write_bool(const uint32_t key, const bool value) { return 0; }
void menu_window_reload_data(MenuWindow *window) { ++refreshes; }
void menu_window_refresh(MenuWindow *window) {}
void detail_window_set_countdown_timer(DetailWindow *window, CountdownTimer *timer) { displayed = timer; }
void detail_window_push(DetailWindow *window, bool animated) { ++detail_pushes; }
void detail_window_deep_refresh(DetailWindow *window) {}
void detail_window_pop(DetailWindow *window, bool animated) { ++detail_closes; }
bool detail_window_get_topmost_window(DetailWindow *window) { return false; }
CountdownTimer *detail_window_get_countdown_timer(DetailWindow *window) { return displayed; }
bool popup_window_get_topmost_window(PopupWindow *window) { return alert_visible; }
CountdownTimer *popup_window_get_countdown_timer(PopupWindow *window) { return alarm_timer; }
void popup_window_set_countdown_timer(PopupWindow *window, CountdownTimer *timer) { alarm_timer = timer; }
void popup_window_pop(PopupWindow *window, bool animated) { ++popup_closes; alert_visible = false; }
void phone_send_pin(CountdownTimer *timer) { ++pin_sends; }
void phone_delete_pin(CountdownTimer *timer) { ++pin_deletes; }
void vibes_short_pulse(void) { ++pulses; }
bool app_timer_reschedule(AppTimer *timer, uint32_t timeout) { return true; }
uint16_t time_ms(time_t *seconds, uint16_t *milliseconds) {
  if (seconds) *seconds = time(NULL) + clock_offset;
  if (milliseconds) *milliseconds = 0;
  return 0;
}
void app_log(uint8_t level, const char *file, int line, const char *fmt, ...) {}
status_t persist_write_int(const uint32_t key, const int32_t value) {
  if (key == COUNTDOWN_TIMER_PERSIST_KEY) saved_count = value;
  return 0;
}
int persist_write_data(const uint32_t key, const void *data, const size_t size) {
  unsigned slot = key - COUNTDOWN_TIMER_PERSIST_KEY - 1;
  assert(slot < COUNTDOWN_TIMERS_MAX && size <= sizeof(saved_data[slot]));
  memcpy(saved_data[slot], data, size);
  saved_sizes[slot] = size;
  return (int)size;
}
int32_t persist_read_int(const uint32_t key) { return saved_count; }
bool persist_exists(const uint32_t key) { return key > COUNTDOWN_TIMER_PERSIST_KEY && key <= COUNTDOWN_TIMER_PERSIST_KEY + (unsigned)saved_count; }
int persist_get_size(const uint32_t key) { return (int)saved_sizes[key - COUNTDOWN_TIMER_PERSIST_KEY - 1]; }
int persist_read_data(const uint32_t key, void *data, const size_t size) {
  memcpy(data, saved_data[key - COUNTDOWN_TIMER_PERSIST_KEY - 1], size);
  return (int)size;
}

int main(void) {
  menu_window_click_callback(0, NULL);
  CHECK(manual_opens == 1 && voice_starts == 0 && s_countdown_timers_count == 0);
  menu_window_long_click_callback(1, NULL);
  CHECK(voice_starts == 0);
  menu_window_long_click_callback(0, NULL);
  CHECK(voice_starts == 1);
  CHECK(voice_timer_create_callback(900));
  CHECK(s_countdown_timers_count == 1);
  CHECK(displayed == s_countdown_timers[0]);
  CHECK(countdown_timer_get_duration(displayed) == 900000);
  CHECK(!countdown_timer_get_paused(displayed));
  CHECK(detail_pushes == 1 && pin_sends == 1 && pulses == 1 && refreshes == 1);
  CountdownTimer *first = displayed;
  CHECK(voice_timer_create_callback(1200));
  CHECK(displayed == s_countdown_timers[0] && displayed != first);
  CHECK(s_countdown_timers[1] == first);
  CHECK(countdown_timer_get_duration(displayed) == 1200000);
  alert_visible = true;
  CHECK(voice_timer_create_callback(30));
  CHECK(detail_pushes == 2); // Do not cover an alarm that appeared while speaking.
  CHECK(pin_sends == 2); // Short timers do not get timeline pins.
  for (int i = 3; i < COUNTDOWN_TIMERS_MAX; ++i) CHECK(voice_timer_create_callback(60 + i));
  CHECK(s_countdown_timers_count == COUNTDOWN_TIMERS_MAX);
  CountdownTimer *before[COUNTDOWN_TIMERS_MAX];
  memcpy(before, s_countdown_timers, sizeof(before));
  unsigned pulses_before = pulses;
  CHECK(!voice_timer_create_callback(1800));
  CHECK(pulses == pulses_before && s_countdown_timers_count == COUNTDOWN_TIMERS_MAX);
  CHECK(memcmp(before, s_countdown_timers, sizeof(before)) == 0);
  countdown_timer_list_destroy_all(s_countdown_timers, &s_countdown_timers_count);
  CHECK(s_countdown_timers_count == 0);
  alert_visible = false;
  CHECK(voice_timer_create_callback(5));
  CountdownTimer *finished = displayed;
  CHECK(voice_timer_create_callback(300));
  CountdownTimer *running = displayed;
  clock_offset = 6;
  CHECK(countdown_timer_check_ended(s_countdown_timers, s_countdown_timers_count) == finished);
  CHECK(countdown_timer_get_paused(finished) && countdown_timer_get_current_time(finished) == 0);
  alarm_timer = displayed = edited_timer = finished;
  alert_visible = true;
  unsigned pulses_before_dismiss = pulses;
  popup_window_stop_timer_callback(NULL);
  CHECK(!alert_visible && popup_closes == 1);
  CHECK(s_countdown_timers_count == 1 && s_countdown_timers[0] == running);
  CHECK(!alarm_timer && !displayed && !edited_timer);
  CHECK(detail_closes == 1 && setting_closes == 1);
  CHECK(saved_count == 1 && pulses == pulses_before_dismiss + 1);
  CHECK(!countdown_timer_get_paused(running));
  // Reload the actual stored timer model: the completed timer stays removed.
  CountdownTimer *restored[COUNTDOWN_TIMERS_MAX] = {0};
  uint8_t restored_count = 0;
  countdown_timer_list_load(restored, COUNTDOWN_TIMERS_MAX, &restored_count, COUNTDOWN_TIMER_PERSIST_KEY);
  CHECK(restored_count == 1 && countdown_timer_get_duration(restored[0]) == 300000);
  countdown_timer_list_destroy_all(restored, &restored_count);
  popup_window_stop_timer_callback(NULL); // Repeat/stray dismissal is harmless.
  CHECK(s_countdown_timers_count == 1 && pulses == pulses_before_dismiss + 1);

  CHECK(voice_timer_create_callback(5));
  CountdownTimer *snoozed = displayed;
  clock_offset += 6;
  CHECK(countdown_timer_check_ended(s_countdown_timers, s_countdown_timers_count) == snoozed);
  alarm_timer = snoozed;
  popup_window_snooze_timer_callback(snoozed, NULL);
  CHECK(s_countdown_timers_count == 2 && !countdown_timer_get_paused(snoozed));
  CHECK(countdown_timer_get_current_time(snoozed) == COUNTDOWN_TIMER_SNOOZE_DELAY);
  // Pausing a live timer and a stray dismissal must not delete it.
  countdown_timer_stop(snoozed, &s_countdown_timer_id_max);
  popup_window_stop_timer_callback(NULL);
  CHECK(s_countdown_timers_count == 2 && countdown_timer_get_paused(snoozed));
  CHECK(countdown_timer_get_current_time(snoozed) > 0);
  // Dismiss the final timer, and check that the empty list is saved as empty.
  countdown_timer_list_destroy_all(s_countdown_timers, &s_countdown_timers_count);
  CHECK(voice_timer_create_callback(900));
  finished = displayed;
  clock_offset += 901;
  CHECK(countdown_timer_check_ended(s_countdown_timers, s_countdown_timers_count) == finished);
  alarm_timer = finished;
  edited_timer = NULL;
  popup_window_stop_timer_callback(NULL);
  CHECK(s_countdown_timers_count == 0 && saved_count == 0);
  CHECK(pin_deletes == 1);
  printf("Passed %u timer lifecycle checks.\n", checks);
  return 0;
}
