#include "voice_duration.h"
#include <stddef.h>
#include <string.h>

#define MAX_SECONDS 86400u

static bool digit(char c) { return c >= '0' && c <= '9'; }
static bool letter(char c) { return c >= 'a' && c <= 'z'; }

static int number(const char *word) {
  static const char *const small[] = {
    "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine",
    "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen",
    "seventeen", "eighteen", "nineteen"
  };
  static const char *const tens[] = {
    "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"
  };
  if (!strcmp(word, "a") || !strcmp(word, "an")) return 1;
  for (unsigned i = 0; i < sizeof(small) / sizeof(small[0]); ++i)
    if (!strcmp(word, small[i])) return (int)i;
  for (unsigned i = 0; i < sizeof(tens) / sizeof(tens[0]); ++i)
    if (!strcmp(word, tens[i])) return (int)(i + 2) * 10;
  if (!digit(*word)) return -1;
  unsigned value = 0;
  for (; *word; ++word) {
    if (!digit(*word)) return -1;
    value = value * 10 + (unsigned)(*word - '0');
    if (value > MAX_SECONDS) return -2;
  }
  return (int)value;
}

static unsigned unit(const char *word) {
  if (!strcmp(word, "hour") || !strcmp(word, "hours") || !strcmp(word, "hr") || !strcmp(word, "hrs")) return 3600;
  if (!strcmp(word, "minute") || !strcmp(word, "minutes") || !strcmp(word, "min") || !strcmp(word, "mins")) return 60;
  if (!strcmp(word, "second") || !strcmp(word, "seconds") || !strcmp(word, "sec") || !strcmp(word, "secs")) return 1;
  return 0;
}

static bool filler(const char *word) {
  static const char *const allowed[] = {
    "set", "start", "create", "a", "an", "timer", "for", "please", "and", "me"
  };
  for (unsigned i = 0; i < sizeof(allowed) / sizeof(allowed[0]); ++i)
    if (!strcmp(word, allowed[i])) return true;
  return false;
}

bool voice_duration_parse(const char *text, uint32_t *seconds) {
  if (!text || !seconds || !*text || strlen(text) >= 256) return false;
  char buffer[256];
  unsigned length = 0;
  for (; text[length]; ++length) {
    char c = text[length];
    if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    // A hyphen in "15-minute" is fine; a negative or decimal number is not.
    if ((c == '-' && digit(text[length + 1])) ||
        (c == '.' && length && digit(text[length - 1]) && digit(text[length + 1]))) return false;
    if (!letter(c) && !digit(c) && c != ' ' && c != '\t' && c != '\n' &&
        c != '\r' && c != '-' && c != ',' && c != '.' && c != '!' && c != '?') return false;
    buffer[length] = (letter(c) || digit(c)) ? c : ' ';
  }
  buffer[length] = '\0';
  char *tokens[48];
  int count = 0;
  char *p = buffer;
  while (*p) {
    while (*p == ' ') ++p;
    if (!*p) break;
    if (count == 48) return false;
    tokens[count++] = p;
    while (*p && *p != ' ') ++p;
    if (*p) *p++ = '\0';
  }

  unsigned total = 0;
  for (int i = 0; i < count;) {
    unsigned fraction = 0; // divisor for half/quarter
    unsigned value = 0;
    int j = i;
    if (!strcmp(tokens[j], "half") || !strcmp(tokens[j], "quarter")) {
      fraction = !strcmp(tokens[j++], "half") ? 2 : 4;
      if (j < count && !strcmp(tokens[j], "of")) ++j;
      if (j < count && (!strcmp(tokens[j], "a") || !strcmp(tokens[j], "an"))) ++j;
    } else {
      int part = number(tokens[j]);
      if (part < 0) {
        if (part == -2 || !filler(tokens[j])) return false;
        ++i;
        continue;
      }
      value = (unsigned)part;
      int previous = part;
      bool numeric = digit(tokens[j][0]);
      ++j;
      while (j < count) {
        if (!strcmp(tokens[j], "hundred") && !numeric && previous > 0 && previous < 10) {
          value *= 100;
          previous = 100;
          ++j;
          continue;
        }
        if (previous >= 100 && !strcmp(tokens[j], "and") && j + 1 < count && number(tokens[j + 1]) >= 0) ++j;
        if (j == count) break;
        part = number(tokens[j]);
        if (part < 0 || numeric || digit(tokens[j][0]) ||
            !(previous == 100 || (previous >= 20 && previous % 10 == 0 && part > 0 && part < 10))) break;
        value += (unsigned)part;
        previous = part;
        ++j;
      }
      // "One and a half hours" and "two and a quarter minutes".
      int k = j;
      if (k < count && !strcmp(tokens[k], "and")) {
        ++k;
        if (k < count && !strcmp(tokens[k], "a")) ++k;
        if (k < count && (!strcmp(tokens[k], "half") || !strcmp(tokens[k], "quarter"))) {
          fraction = !strcmp(tokens[k], "half") ? 2 : 4;
          j = k + 1;
        }
      }
    }
    unsigned multiplier = j < count ? unit(tokens[j]) : 0;
    if (!multiplier) {
      if (!fraction && (!strcmp(tokens[i], "a") || !strcmp(tokens[i], "an"))) { ++i; continue; }
      return false;
    }
    if (value > MAX_SECONDS / multiplier || (fraction && multiplier % fraction)) return false;
    unsigned add = value * multiplier + (fraction ? multiplier / fraction : 0);
    if (add > MAX_SECONDS || total > MAX_SECONDS - add) return false;
    total += add;
    i = j + 1;
  }
  if (total < 5 || total > MAX_SECONDS) return false;
  *seconds = total;
  return true;
}
