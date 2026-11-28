# What the IOSurface framework asks of the kernel

Found with an interposer on a Mac (`SwiftUI/tools/msgtrace.c`: `IOServiceOpen`, `IOConnectCallMethod`, `IOConnectTrap`) and in the code of `IOSurface.framework` (`_IOSurfaceClientCreateChild`, `_IOSurfaceClientLock`),
macOS 26.7.2 x86_64.

* `IOServiceOpen(IOSurfaceRoot, type 0)` gives a connection.
* `IOConnectCallMethod`:
  * **selector 13**: no input, 40 bytes out (the limits of a surface; the Mac's answer is copied: `3f 00 00 00 3f 00 00 00 00 00 04 00 …`).
  * **selector 4 = look-up**: one scalar (the surface id), the same 0xc68 bytes out as create; the kernel maps the memory into the calling task.  Only a surface made with `IOSurfaceIsGlobal = true` can be found by id from another process; otherwise `kIOReturnBadArgument` (`0xe00002c2`), seen on the Mac.
  * **selector 44**, one scalar: `kIOReturnNotFound` (`0xe00002f0`) for an unknown value.
  * **selector 0 = create**: one scalar (the id of the parent surface, 0); the properties as a binary serialized dictionary (`OSSerializeBinary`: signature `0xd3`, the dictionary header `0x81000005`,
    then symbol / value pairs; numbers are 8 bytes; keys `IOSurfaceWidth`, `IOSurfaceHeight`, `IOSurfaceBytesPerElement`, `IOSurfacePixelFormat`, `IOSurfaceName`, optional `IOSurfaceBytesPerRow`, `IOSurfaceAllocSize`);
    **0xc68 bytes out**.
* The 0xc68 bytes: `@0` the address at which the memory of the surface is mapped **in the calling task** (the kernel maps it; there is no `IOConnectMapMemory`), `@8` / `@16` two pointers into a region
  that the kernel keeps for the client, `@24` the id, `@32` allocation size, `@40` width, `@48` height, `@56` bytes per row (`roundup(width * bytes per element, 64)`), `@72` pixel format, `@80` allocation size,
  `@96` bytes per element and two flag bytes, `@120` a counter with a flag; the rest zero for one plane.
* Fast methods are **mach trap 100** (`iokit_user_client_trap`, `IOConnectTrapN`): index 2 lock (`id`, `options`, `*seed`), 3 unlock, 5 release.
* Lock, unlock and look-up of a surface that the process made itself need no kernel call beyond these.

Not yet found: `IOSurfaceCreateMachPort` / `IOSurfaceLookupFromMachPort`, planes (the tail of the structure), release.

## Ports (2026-11-28, decompiled from IOSurface.framework of macOS 26.7.2)
* `IOSurfaceClientCreateMachPortWithOptions(client, options)`: `IOConnectCallMethod(conn, 35, {id (client + 0x88), options}, 2 scalars in, no struct, 1 scalar out)`; the scalar out is the name of a send right in the caller's task (0 on failure).
* `IOSurfaceClientLookupFromMachPort(port)`: `IOConnectCallMethod(conn, 34, {port}, 1 scalar in, 0xc68 bytes out)`, the same structure as create and lookup; when its first u64 is -1 the caller has the surface already and the framework takes its cached client by the id at 0x18 (no new client, no release trap); otherwise it makes a client from the structure (and calls trap 5, release, if that fails).
* pearOS: a port of kind KP_IOSURF in darwinator; each port is one use of the surface, so a surface stays while a port names it, as on the Mac (`tests/iosurf-port.c`).
