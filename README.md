# Timer Voice
This checkout adds **hold SELECT on + to speak a timer duration**. See [VOICE-README.md](VOICE-README.md) for usage, import instructions and validation.

Import `https://github.com/lucaboox/pebble-timer-voice` into CloudPebble (branch **main**), select **basalt** for Pebble Time, compile and install. For future updates, pull the latest GitHub changes into the same CloudPebble project, then compile and install again.

On **Time's Up!**, **SELECT, DOWN or BACK** dismisses and deletes the finished timer. **UP** snoozes it for one minute.

## Original Pebble Timer

Simple pebble timer built to be published by Pebble in the store.

![alt Timer Menu](https://github.com/pebble/pebble-timer/blob/release-1.0/assets/Menu.png)
![alt Timer Setting](https://github.com/pebble/pebble-timer/blob/release-1.0/assets/Setting 1.png)
![alt Timer Detail](https://github.com/pebble/pebble-timer/blob/release-1.0/assets/Bubbles.png)
![alt Timer Pin](https://github.com/pebble/pebble-timer/blob/release-1.0/assets/Pin.png)

Timers appear in a list on the main screen. Add a timer via the "+" icon at the top of the list similarly
to the alarm app. When setting a timer, the time the timer will end is displayed under the entry fields if
the timer duration is greater than 15 minutes. Also, the timer will send a pin to the timeline if the duration
is greater than 15 minutes. A detailed view of the timer can be accessed by either selecting one of the 
menu items on the main screen, or by choosing "Open timer" in the pin's action menu. In the detailed view,
the timer can be played and paused as well as edited and deleted.

If the app is closed out, it will automatically open and vibrate when a timer goes off. From this timer ended
screen, the user can dismiss the timer or snooze the timer. In this variant, SELECT, DOWN, or BACK
dismisses and deletes the finished timer; UP snoozes it for one minute. An alert that times out
automatically leaves the completed timer in the list.

## Changelog

### v1.2.1
- Added Gabbro platform support
- Improved UI scaling and font sizing across all screen sizes (Emery, Gabbro)
- Improved setting window UI scaling with dynamic layout calculations
- Fixed menu icon rendering artifact (black block/triangle) on round displays
- Cleaned play icon anti-aliasing for crisp rendering on all platforms

Thanks to [Thomas Buchheit](https://github.com/thomas5014) for contributing Emery/Gabbro support and UI scaling improvements.
