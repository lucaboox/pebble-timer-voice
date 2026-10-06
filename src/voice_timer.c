#include "voice_timer.h"
#include "voice_duration.h"

#ifdef PBL_MICROPHONE
static DictationSession *s_session;
static bool s_active;
static VoiceTimerCreateCallback s_create;
static Window *s_error_window;
static TextLayer *s_error_text;

static void show_error(const char *message) {
  if (!s_error_window) {
    s_error_window = window_create();
    if (!s_error_window) return;
    Layer *root = window_get_root_layer(s_error_window);
    GRect bounds = layer_get_bounds(root);
    s_error_text = text_layer_create(GRect(12, 20, bounds.size.w - 24, bounds.size.h - 28));
    if (!s_error_text) {
      window_destroy(s_error_window);
      s_error_window = NULL;
      return;
    }
    text_layer_set_font(s_error_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_error_text, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_error_text));
  }
  text_layer_set_text(s_error_text, message); // All messages are static strings.
  if (!window_stack_contains_window(s_error_window)) window_stack_push(s_error_window, true);
}

static void dictation_callback(DictationSession *session, DictationSessionStatus status,
                               char *text, void *context) {
  s_active = false;
  // BACK/cancel in Pebble's dictation UI simply returns to the timer list.
  if (status == DictationSessionStatusFailureTranscriptionRejected) return;
  if (status != DictationSessionStatusSuccess) {
    show_error("Speech failed.\nCheck your phone\nand try again.\n\nBACK to return");
    return;
  }
  uint32_t seconds;
  if (!voice_duration_parse(text, &seconds)) {
    show_error("Say a duration:\n\"15 minutes\".\n5 seconds to\n24 hours.\n\nBACK to return");
    return;
  }
  if (!s_create || !s_create(seconds)) {
    show_error("Couldn't add timer.\nClear a timer and\ntry again.\n\nBACK to return");
  }
}
#endif

void voice_timer_init(VoiceTimerCreateCallback create_callback) {
#ifdef PBL_MICROPHONE
  s_create = create_callback;
#else
  (void)create_callback;
#endif
}

void voice_timer_start(void) {
#ifdef PBL_MICROPHONE
  if (s_active) return;
  // Retry creation on every attempt if the phone was unavailable earlier.
  if (!s_session) {
    s_session = dictation_session_create(256, dictation_callback, NULL);
    if (!s_session) {
      show_error("Voice unavailable.\nConnect your phone\nand enable dictation.\n\nBACK to return");
      return;
    }
    dictation_session_enable_confirmation(s_session, true);
    dictation_session_enable_error_dialogs(s_session, false);
  }
  s_active = true;
  if (dictation_session_start(s_session) != DictationSessionStatusSuccess) {
    s_active = false;
    show_error("Voice didn't start.\nCheck your phone\nand try again.\n\nBACK to return");
  }
#endif
}

void voice_timer_deinit(void) {
#ifdef PBL_MICROPHONE
  if (s_session) dictation_session_destroy(s_session);
  if (s_error_window) window_stack_remove(s_error_window, false);
  if (s_error_text) text_layer_destroy(s_error_text);
  if (s_error_window) window_destroy(s_error_window);
  s_session = NULL;
  s_error_text = NULL;
  s_error_window = NULL;
  s_active = false;
  s_create = NULL;
#endif
}
