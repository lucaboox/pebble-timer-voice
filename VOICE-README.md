# Timer Voice

A voice-enabled variant of [Core Devices' Pebble Timer](https://github.com/coredevices/pebble-timer).
Based on upstream commit `ebe0ac397cd0a85c7680de3d20702325232a0853` (version 1.2.7).

## Use on Pebble Time

1. Open **Timer Voice**. The timer list starts on the **+** row, including when the list is empty.
2. **Hold the middle-right SELECT button on +** to open Pebble's dictation screen.
3. Say a duration and accept the transcription. The timer starts and opens its normal detail screen.
4. A short SELECT on + opens the original manual duration picker.

The + row shows **Hold to speak** on watches with microphones. Holding SELECT on timer rows does not create a timer.

Examples:

- "15 minutes"
- "Set a twenty minute timer"
- "Start a timer for 30 minutes"
- "One hour and thirty minutes"
- "One and a half hours"
- "Half an hour"
- "45 seconds"

English durations from 5 seconds through 24 hours are supported, including spelled-out numbers, hours/minutes/seconds, and half/quarter units. Specify a unit for every number. Use "one and a half minutes" rather than a decimal like "1.5 minutes".

The watch needs a connected phone with working Pebble dictation. Pebble's own speech service handles transcription; this project needs no API key. BACK/cancel in the speech screen returns without creating a timer. Invalid durations and connection failures show an error; press BACK to return and hold + to retry. If all eight timer slots are occupied, voice creation asks you to clear a timer rather than removing one automatically.

Voice timers use the same timer model, timeline-pin threshold, persistence, wakeup, detail controls, alarms and recent-use ordering as manual timers. A timer expiring during speech keeps its alarm screen above the new timer's detail view.

## Finished timers

On **Time's Up!**, press **SELECT (middle-right), DOWN (bottom-right), or BACK** to dismiss the alert and delete that finished timer. Its slot is freed and the deletion is saved immediately. A short vibration confirms dismissal. **UP** snoozes for one minute and keeps the timer. Running and manually paused timers stay in the list. If the alert closes by itself, the completed timer remains available to delete from its detail screen.

## CloudPebble and GitHub

The app is named **Timer Voice** and has its own UUID, so it can be installed alongside the original Timer app. Its timers are stored separately.

For ZIP import, use `Pebble-Timer-Voice-CloudPebble.zip` from the parent folder. Select **basalt** for Pebble Time, compile, then install the resulting app. The other upstream platform targets are retained; speech is compiled only when the platform supports a microphone.

For repeat updates, import/link [lucaboox/pebble-timer-voice](https://github.com/lucaboox/pebble-timer-voice) in CloudPebble, using branch **main**. Push code changes to GitHub and pull them into the same CloudPebble project before compiling/installing again. `package.json`, `wscript`, `src`, and `resources` are at the repository root. The local checkout uses `origin` for this repository and `upstream` for the original Core Devices source.

## Verification

Run `tests/run-tests.ps1 -SdkIncludePath <official basalt include directory>` with Clang and Windows C runtime headers/libraries configured. The tests exercise the actual duration parser and speech handlers, and create timers through the actual app callback and upstream timer model, with Pebble OS/UI calls mocked.

Validated locally:

- 4,757 duration checks (examples, limits, malformed commands, overflow).
- 19 speech session checks (phone recovery, confirmation, cancellation, errors, duplicate-session prevention, cleanup).
- 50 timer lifecycle checks (short press, duration conversion, starting, timeline threshold, ordering, alert priority, capacity, dismissal, freeing slots, restoring saved data, preserving other timers, snooze).
- Every watch C source compiled to Cortex-M4 ARM objects against official Pebble SDK 4.33.1 basalt headers.

A complete `.pbw` build and on-watch dictation/display testing still need to be performed in CloudPebble. The standalone ZIP includes the resource files and phone JavaScript used by the original timer app.

## Modified code

`src/voice_duration.*` parses durations. `src/voice_timer.*` handles dictation and error messages. `src/menu_window.*` adds the long-press callback and hint. `src/main.c` connects spoken durations to the existing creation flow, exposes + on first launch, and deletes dismissed finished timers. `src/popup_window.*` routes BACK to dismissal and exposes the alarm's timer; `src/detail_window.*` exposes the displayed timer so its references can be cleared before deletion. `package.json` gives this variant its own identity. `wscript` uses the current SDK build API.

See the original `README.md` for the upstream app description and credits.
