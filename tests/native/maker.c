/* maker: a Mac program that makes a global 64x32 BGRA IOSurface, fills it with a pattern (pixel i = i), prints the id, waits for the
 * native reader (iosurf-native-read) to write 0xC0FFEE into pixel 0, and checks it.  Build on a Mac: clang maker.c -framework IOSurface -framework CoreFoundation -o maker */
#include <IOSurface/IOSurface.h>
#include <stdio.h>
#include <unistd.h>
static void put(CFMutableDictionaryRef d, CFStringRef k, int v) { CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &v); CFDictionarySetValue(d, k, n); CFRelease(n); }
int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	put(d, kIOSurfaceWidth, 64); put(d, kIOSurfaceHeight, 32); put(d, kIOSurfaceBytesPerElement, 4); put(d, kIOSurfacePixelFormat, 'BGRA');
	CFDictionarySetValue(d, CFSTR("IOSurfaceIsGlobal"), kCFBooleanTrue);
	IOSurfaceRef s = IOSurfaceCreate(d);
	if (!s) { printf("maker: no surface\n"); return 1; }
	IOSurfaceLock(s, 0, NULL);
	uint32_t *p = IOSurfaceGetBaseAddress(s); size_t row = IOSurfaceGetBytesPerRow(s);
	for (int y = 0; y < 32; y++) for (int x = 0; x < 64; x++) ((uint32_t *)((char *)p + y * row))[x] = (uint32_t)(y * 64 + x);
	IOSurfaceUnlock(s, 0, NULL);
	printf("maker: id %u row %zu\n", IOSurfaceGetID(s), row);
	for (int i = 0; i < 100 && p[0] != 0xC0FFEE; i++) usleep(50000);
	printf("maker: pixel 0 = %#x: %s\n", p[0], p[0] == 0xC0FFEE ? "ok, written by the native program" : "FAIL");
	return p[0] == 0xC0FFEE ? 0 : 1;
}
