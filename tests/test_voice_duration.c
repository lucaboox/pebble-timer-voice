#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "voice_duration.h"

static unsigned checks;
static void valid(const char *text, unsigned expected) {
  uint32_t seconds = 0;
  assert(voice_duration_parse(text, &seconds));
  assert(seconds == expected);
  checks += 2;
}
static void invalid(const char *text) {
  uint32_t seconds = 123;
  assert(!voice_duration_parse(text, &seconds));
  assert(seconds == 123);
  checks += 2;
}

int main(void) {
  valid("Set a 15 minute timer", 900);
  valid("please start a timer for twenty minutes", 1200);
  valid("30 minutes", 1800);
  valid("one hour and thirty minutes", 5400);
  valid("one and a half hours", 5400);
  valid("two and a quarter minutes", 135);
  valid("half an hour", 1800);
  valid("quarter of an hour", 900);
  valid("a minute", 60);
  valid("set a timer for an hour", 3600);
  valid("TWENTY-FIVE MINUTES.", 1500);
  valid("15-minute timer", 900);
  valid("one hundred and twenty five seconds", 125);
  valid("one hour, two minutes and five seconds!", 3725);
  valid("24 hours", 86400);
  valid("86400 seconds", 86400);
  valid("5 seconds", 5);
  valid("0 hours 15 minutes", 900);
  valid("ninety nine minutes", 5940);
  invalid(NULL);
  invalid("");
  invalid("set a timer");
  invalid("15");
  invalid("15 bananas");
  invalid("two days and one minute");
  invalid("minus five minutes");
  invalid("negative 15 minutes");
  invalid("-15 minutes");
  invalid("1.5 minutes");
  invalid("0 minutes");
  invalid("4 seconds");
  invalid("25 hours");
  invalid("24 hours and 1 second");
  invalid("9999999999999999999999 seconds");
  invalid("one two minutes");
  invalid("one hundred hundred minutes");
  invalid("minutes");
  invalid("half a second");
  invalid("cancel a 15 minute timer");
  invalid("set an alarm for 15 minutes");
  invalid("10:30");
  char long_text[300];
  memset(long_text, 'a', sizeof(long_text) - 1);
  long_text[sizeof(long_text) - 1] = '\0';
  invalid(long_text);
  assert(!voice_duration_parse("15 minutes", NULL));
  ++checks;
  for (unsigned n = 5; n <= 86400; n += 37) {
    char phrase[50];
    snprintf(phrase, sizeof(phrase), "set a timer for %u seconds", n);
    valid(phrase, n);
  }
  printf("Passed %u voice duration checks.\n", checks);
  return 0;
}
