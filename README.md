# IOSurface

IOSurface for pearOS: the protocol of the Mac's IOSurface framework (found on a real Mac and in `IOSurface.framework`), the tests, and the notes.  The kernel half (the user client of `IOSurfaceRoot`) is in
[`darwinator`](https://github.com/pearOS-bsd/darwinator) (`src/iosurface.c`); this repository has what does not belong in a kernel module.

| path | what |
|---|---|
| `docs/PROTOCOL.md` | what the framework asks the kernel (selectors, structures, traps), with the evidence |
| `tests/iosurf.c` | a plain C program: create a surface, lock, fill, unlock, look it up by id.  Build on a Mac (`clang iosurf.c -framework IOSurface -framework CoreFoundation`) and run on the Mac and on pearOS: the output must be the same |

Why it matters: the contents of the layers of an AppKit / SwiftUI window are IOSurfaces; without them a window is empty.

Status: create, lock, unlock and look-up in one process work on pearOS.  Not done: look-up from another process, Mach ports of surfaces, release, planes, and the window server reading the memory.

Licence: BSD-2-Clause.  No Apple file is in this repository.
