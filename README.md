# IOSurface

IOSurface for pearOS: the protocol of the Mac's IOSurface framework (found on a real Mac and in `IOSurface.framework`), the tests, and the notes.  The kernel half (the user client of `IOSurfaceRoot`) is in
[`darwinator`](https://github.com/pearOS-bsd/darwinator) (`src/iosurface.c`); this repository has what does not belong in a kernel module.

| path | what |
|---|---|
| `docs/PROTOCOL.md` | what the framework asks the kernel (selectors, structures, traps), with the evidence |
| `tests/iosurf2.c` | the same across two processes: one makes a global surface and writes a pixel, a second one (spawned with the id) looks it up and reads it |
| `tests/iosurf-release.c` | release: 600 full-screen surfaces made and released one after the other, then a global surface looked up by a second process and gone after its last release (same output on the Mac and on pearOS) |
| `tests/iosurf.c` | a plain C program: create a surface, lock, fill, unlock, look it up by id.  Build on a Mac (`clang iosurf.c -framework IOSurface -framework CoreFoundation`) and run on the Mac and on pearOS: the output must be the same |

Why it matters: the contents of the layers of an AppKit / SwiftUI window are IOSurfaces; without them a window is empty.

Status: create, lock, unlock and look-up (in the same process and from another one, for global surfaces) work on pearOS, same output as the Mac.  Release works too: each process has one client of a surface with a use count, as IOSurfaceRootUserClient on the Mac; its last release unmaps the memory, and the surface goes when no process uses it (a process that exits loses its uses).  Not done: Mach ports of surfaces, planes, and the window server reading the memory.

Licence: BSD-2-Clause.  No Apple file is in this repository.
