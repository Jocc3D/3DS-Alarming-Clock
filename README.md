# 3DS Alarming Clock
3DS Alarming Clock is a simple yet efficient open source alarm clock app capable of ringing even while the console is in sleep mode!

> This app is **NOT** finished. Expect bugs and weird things. Any feedback is appreciated! =D


## Why 3DS Alarming Clock even exist?

Why would you need an alarm on your 3DS?" you might ask. "Isn't it just better to use your phone?" some will say — and honestly, they're not entirely wrong. However, for a long time I had issues with my phone: the battery drained way too fast and the charging port was damaged, so leaving it charging overnight was essentially a coin flip, it could disconnect at some point during the night or something. On top of that, phone alarms have one tiny, minuscule, incredibly small problem: you can just turn them off and fall right back asleep.

Over time I realized my 3DS had significantly better battery life and, more importantly, didn't have a busted charging port and a failing battery. That's when I told myself "yeah maybe this could work." After quite a bit of research on this unknown world of Homebrew programming I decided to create 3DS Alarming Clock and share it with everyone! Maybe it's not perfect, but it definitively works!

## Features
* Alarm rings even in sleep mode (console closed)
* Sleeping-proof!
  * While the alarm is ringing, you can't go to the HOME menu or lower the volume
  * You can enable a small challenge to turn off the alarm!
* 6 independent alarms, each with its own settings:
  * Hour and minute to ring
  * Volume — independent from the console's volume!
  * LED color and pattern when the alarm rings
  * A simple button combination challenge to dismiss the alarm
* 12-hour and 24-hour clock format support
* English and Spanish language support (more languages coming soon! (I hope so!!!))
* LED notification with multiple colors and gentle patterns

## Known Issues
* The alarm keeps ringing even outside the app if you go to the HOME menu while configuring the alarm volume
* LED sometimes doesn't turn on when the alarm rings
* Alarm keeps ringing for a few seconds after turning it off
* Alarms setted to current hour will instantly ring

## Credits
* **Jocc3D** — app development, banner, icons, SFX and other assets
* **PabloMK7** — [libncsnd](https://github.com/PabloMK7/libncsnd), which makes audio playback possible with the console closed

## Building
Requires [devkitARM](https://devkitpro.org/) with the following libraries from the devkitPro pacman repository:
* `3ds-citro2d`
* `3ds-citro3d`
* `3ds-libctru`
* `3ds-mcuhwc`
* [libncsnd](https://github.com/PabloMK7/libncsnd) by PabloMK7

## Disclaimer
This app's code was developed with AI assistance. However, no visual or audio assets (e.g. banner, icon, sounds) were AI-generated.
