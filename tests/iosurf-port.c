/* iosurf-port: IOSurfaceCreateMachPort / IOSurfaceLookupFromMachPort.  A surface with a pattern, a port of it; the port looked up in the same process
 * (the cached surface), the surface released while the port holds it and looked up again (the memory still there), the port sent in a Mach message to a
 * port of the process and looked up from the name received, then the port deallocated.  Same output on the Mac and on pearOS.
 * Build on a Mac: clang iosurf-port.c -framework IOSurface -framework CoreFoundation -o iosurf-port */
#include <IOSurface/IOSurface.h>
#include <mach/mach.h>
#include <stdio.h>
static void put(CFMutableDictionaryRef d, CFStringRef k, int v) { CFNumberRef n = CFNumberCreate(NULL, kCFNumberIntType, &v); CFDictionarySetValue(d, k, n); CFRelease(n); }
int main(void)
{
	CFMutableDictionaryRef d = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	put(d, kIOSurfaceWidth, 32); put(d, kIOSurfaceHeight, 16); put(d, kIOSurfaceBytesPerElement, 4); put(d, kIOSurfacePixelFormat, 'BGRA');
	IOSurfaceRef s = IOSurfaceCreate(d);
	uint32_t id = IOSurfaceGetID(s);
	IOSurfaceLock(s, 0, NULL); *(uint32_t *)IOSurfaceGetBaseAddress(s) = 0x5eedf00d; IOSurfaceUnlock(s, 0, NULL);
	mach_port_t port = IOSurfaceCreateMachPort(s);
	printf("iosurf-port: port %s\n", port != MACH_PORT_NULL ? "made" : "NULL");
	IOSurfaceRef a = IOSurfaceLookupFromMachPort(port);
	printf("iosurf-port: same process: %s (%s)\n", a ? "found" : "NOT FOUND", a == s ? "the same object" : "another object");
	if (a) CFRelease(a);
	CFRelease(s);
	IOSurfaceRef b = IOSurfaceLookupFromMachPort(port);
	printf("iosurf-port: after the release: %s, id %s, first pixel %#x\n", b ? "found" : "NOT FOUND", b && IOSurfaceGetID(b) == id ? "same" : "different", b ? *(uint32_t *)IOSurfaceGetBaseAddress(b) : 0);
	if (b) CFRelease(b);
	/* through a message */
	mach_port_t rcv; mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &rcv);
	struct { mach_msg_header_t h; mach_msg_body_t b; mach_msg_port_descriptor_t p; } m = { 0 };
	m.h.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_MAKE_SEND, 0) | MACH_MSGH_BITS_COMPLEX; m.h.msgh_size = sizeof m; m.h.msgh_remote_port = rcv; m.h.msgh_id = 7;
	m.b.msgh_descriptor_count = 1; m.p.name = port; m.p.disposition = MACH_MSG_TYPE_COPY_SEND; m.p.type = MACH_MSG_PORT_DESCRIPTOR;
	kern_return_t kr = mach_msg(&m.h, MACH_SEND_MSG, sizeof m, 0, 0, 0, 0);
	struct { mach_msg_header_t h; mach_msg_body_t b; mach_msg_port_descriptor_t p; mach_msg_trailer_t t; } in = { 0 };
	kr = kr ? kr : mach_msg(&in.h, MACH_RCV_MSG, 0, sizeof in, rcv, 0, 0);
	IOSurfaceRef c = kr == 0 ? IOSurfaceLookupFromMachPort(in.p.name) : NULL;
	printf("iosurf-port: through a message: %s, first pixel %#x\n", c ? "found" : "NOT FOUND", c ? *(uint32_t *)IOSurfaceGetBaseAddress(c) : 0);
	if (c) CFRelease(c);
	mach_port_deallocate(mach_task_self(), port);
	if (kr == 0) mach_port_deallocate(mach_task_self(), in.p.name);
	IOSurfaceRef e = IOSurfaceLookup(id);
	printf("iosurf-port: after the ports went: lookup by id %s\n", e ? "found" : "not found");
	return 0;
}
