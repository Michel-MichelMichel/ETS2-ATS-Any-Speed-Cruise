# Changelog

## 1.1.0 - 2026-11-15

- New key setup tool `any_speed_cruise_config.exe`: click "Change", press the
  key or button, and `any_speed_cruise.ini` is filled in.
- The plugin accepts wheel / gamepad buttons (`joy1.b5`) in addition to
  keyboard keys.
- `install.ps1` finds the game through the Steam libraries (`-Game ets2|ats`).

## 1.0.0 - 2026-11-15

First version. Built with SCS SDK 1.16, for ETS2 1.62 (and ATS).

- Holds the speed with the throttle below the game's cruise control limit.
- "Resume" works at any speed, including from a standstill.
- Increase / decrease the target while the plugin holds the speed.
- Automatic handover to the game's own cruise control above the limit.
- Lets go on braking, cruise key, parking brake, reverse gear, engine off or
  pause.
- Keys and settings in `any_speed_cruise.ini`.
