# D0G
Say "rise and shine" to the robots!

An attempt to port Half-Life 2 and Portal to Android tablets and consoles.

## Done
### Unreleased
* VPhysics.

### Released
* Common and public headers.
* Build makefiles.
* Limited 16-bit US locale `wchar` implementation (`swprintf` aliases ANSI `snprintf`, so it must not be used to format strings containing `wchar_t`, also `cstd/wchar.h` must be included after `stdlib.h` to override `mbstowcs` and `wcstombs`).
* Tiers 0-3.
* Valve Standard Library.
* App framework.
* Launcher (partially).
* VPhysics Airboat.
