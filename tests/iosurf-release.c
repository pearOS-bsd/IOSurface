/* release: surfaces that are released go.  600 full-screen surfaces (8 MB each) are made, written and released one after the other: with the memory kept for ever the
 * kernel runs out of slots or of memory long before the end.  Then a global surface is looked up by a second process, released by its maker, and the id must not be
 * found any more once nobody uses it.  The output must be the same on the Mac and on pearOS. */
#include <IOSurface/IOSurface.h>
#include <CoreFoundation/CoreFoundation.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

static IOSurfaceRef
make(int w, int h, int global)
{
	CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	int b = 4, f = 'BGRA';
	CFNumberRef n;

	n = CFNumberCreate(NULL, kCFNumberIntType, &w); CFDictionarySetValue(d, kIOSurfaceWidth, n); CFRelease(n);
	n = CFNumberCreate(NULL, kCFNumberIntType, &h); CFDictionarySetValue(d, kIOSurfaceHeight, n); CFRelease(n);
	n = CFNumberCreate(NULL, kCFNumberIntType, &b); CFDictionarySetValue(d, kIOSurfaceBytesPerElement, n); CFRelease(n);
	n = CFNumberCreate(NULL, kCFNumberIntType, &f); CFDictionarySetValue(d, kIOSurfacePixelFormat, n); CFRelease(n);
	if (global)
		CFDictionarySetValue(d, CFSTR("IOSurfaceIsGlobal"), kCFBooleanTrue);
	IOSurfaceRef s = IOSurfaceCreate(d);
	CFRelease(d);
	return s;
}

int
main(int argc, char **argv)
{
	if (argc > 1) {		/* the second process: look the surface up, read it, release it */
		IOSurfaceRef c = IOSurfaceLookup((IOSurfaceID)atoi(argv[1]));
		if (!c) { printf("iosurf-release: child: lookup failed\n"); return 2; }
		printf("iosurf-release: child: first pixel %08x\n", *(unsigned *)IOSurfaceGetBaseAddress(c));
		CFRelease(c);
		return 0;
	}
	for (int i = 0; i < 600; i++) {
		IOSurfaceRef s = make(1920, 1080, 0);
		if (!s) { printf("iosurf-release: surface %d not made\n", i); return 1; }
		IOSurfaceLock(s, 0, NULL);
		memset(IOSurfaceGetBaseAddress(s), 0x5a, IOSurfaceGetAllocSize(s));	/* every page is touched: kept memory would really be used */
		IOSurfaceUnlock(s, 0, NULL);
		CFRelease(s);
	}
	printf("iosurf-release: 600 surfaces of 1920x1080 made and released\n");

	IOSurfaceRef g = make(64, 48, 1);
	IOSurfaceID id = IOSurfaceGetID(g);
	*(unsigned *)IOSurfaceGetBaseAddress(g) = 0x11223344;
	char ids[16]; snprintf(ids, sizeof ids, "%u", id);
	char *av[] = { argv[0], ids, NULL };
	pid_t pid; int st;
	if (posix_spawn(&pid, argv[0], NULL, NULL, av, environ) != 0) { printf("iosurf-release: spawn failed\n"); return 1; }
	waitpid(pid, &st, 0);
	IOSurfaceRef again = IOSurfaceLookup(id);	/* still there: its maker uses it */
	printf("iosurf-release: after the child, lookup %s\n", again ? "found" : "NOT found");
	if (again) CFRelease(again);
	CFRelease(g);
	IOSurfaceRef gone = IOSurfaceLookup(id);
	printf("iosurf-release: after the last release, lookup %s\n", gone ? "FOUND" : "not found");
	return 0;
}
