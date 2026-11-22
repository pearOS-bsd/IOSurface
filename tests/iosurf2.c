/* two processes: this one makes a surface and writes a pixel, a second one (started with the id) looks it up and reads it */
#include <IOSurface/IOSurface.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

extern char **environ;
#include <spawn.h>
int main(int argc, char **argv)
{
	if (argc > 1) {	/* the second process: look the surface up by id */
		IOSurfaceRef c = IOSurfaceLookup((IOSurfaceID)atoi(argv[1]));
		if (!c) { printf("iosurf2: lookup %s failed\n", argv[1]); return 2; }
		IOSurfaceLock(c, 0, NULL);
		printf("iosurf2: other process sees %ux%u, first pixel %08x\n", (unsigned)IOSurfaceGetWidth(c), (unsigned)IOSurfaceGetHeight(c), *(unsigned *)IOSurfaceGetBaseAddress(c));
		IOSurfaceUnlock(c, 0, NULL);
		return 0;
	}
	int fds[2]; pipe(fds);
	CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	int w = 64, h = 48, b = 4, f = 'BGRA';
	CFNumberRef n;
	n = CFNumberCreate(NULL, kCFNumberIntType, &w); CFDictionarySetValue(d, kIOSurfaceWidth, n);
	n = CFNumberCreate(NULL, kCFNumberIntType, &h); CFDictionarySetValue(d, kIOSurfaceHeight, n);
	n = CFNumberCreate(NULL, kCFNumberIntType, &b); CFDictionarySetValue(d, kIOSurfaceBytesPerElement, n);
	n = CFNumberCreate(NULL, kCFNumberIntType, &f); CFDictionarySetValue(d, kIOSurfacePixelFormat, n);
	CFDictionarySetValue(d, CFSTR("IOSurfaceIsGlobal"), kCFBooleanTrue);	/* a surface found by id from another process must be global */
	IOSurfaceRef s = IOSurfaceCreate(d);
	if (!s) { puts("iosurf2: create failed"); return 1; }
	IOSurfaceLock(s, 0, NULL); *(unsigned *)IOSurfaceGetBaseAddress(s) = 0xdeadbeef; IOSurfaceUnlock(s, 0, NULL);
	IOSurfaceID id = IOSurfaceGetID(s);
	{ char ids[16]; pid_t p; snprintf(ids, sizeof ids, "%u", id); char *av[] = { argv[0], ids, NULL }; posix_spawn(&p, argv[0], NULL, NULL, av, environ); int st; waitpid(p, &st, 0); }

	return 0;
}
