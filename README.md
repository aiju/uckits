This is my ambitious goal to create a multi-platform 'framework' for C microcontroller stuff.

Goals:

- Support both higher level and lower level styles easily and support a reasonable degree of intermixing.
- Reasonably self contained package, no extra dependencies (other than gcc and something to program the micro).
- Provide a consistent C library.
- No ANSI C compliance, rather building on Plan 9 libc.
- Probably 32-bit micros only (don't want to bother with too much esoteric Harvard arch silliness).
- Currently using gcc just because it supports a large number of architectures.

Progress so far:

- Still pretty early, STM32 only (and really only STM32F103 in places).
- Still scratching my head about floating point.
- PIC32 may come at some point.
- Not happy about the Rube-Goldberg Makefile.
- So far only tested on Linux.

Build instructions:

- cd ext ; sh fetch.sh to download gcc and binutils
- cd gcc ; sh build.sh to build gcc
- make in top level to build libraries
- make your favourite example and download .bin to controller.

MIT licensed.
