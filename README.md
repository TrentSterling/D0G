﻿# D0G
Say "map testchmb_a_01" to the robots!

2/3 of entire completed work will be uploaded here.
## Done
### Unreleased
* tier3.
* File system.
* App Framework dummy.
* Partially launcher.

### Released
* Common and public headers.
* Build makefiles.
* Limited 16-bit US locale `wchar` implementation (`swprintf` aliases ANSI `snprintf`, so it must not be used to format strings containing `wchar_t`, also `cstd/wchar.h` must be included `stdlib.h` to override `mbstowcs` and `wcstombs`).
* tier0.
* tier1.
* tier2.
* vstdlib.