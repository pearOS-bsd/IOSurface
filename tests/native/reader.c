/* iosurf-native-read ID: a native FreeBSD program that opens the memory of an IOSurface through /dev/iosurface (darwinator src/iosurface_dev.h),
 * checks the pattern of maker.c (pixel i = i) and writes 0xC0FFEE into pixel 0.  cc -I darwinator/src reader.c -o iosurf-native-read */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "iosurface_dev.h"
int main(int argc, char **argv)
{
	if (argc < 2) return 2;
	int d = open("/dev/iosurface", O_RDWR);
	struct iosurf_open io = { .id = (uint32_t)strtoul(argv[1], NULL, 0) };
	if (d < 0 || ioctl(d, IOSURF_IOC_OPEN, &io) != 0) { perror("iosurf-native-read: IOSURF_IOC_OPEN"); return 1; }
	printf("reader: surface %u: %ux%u, row %u, format %#x, %ju bytes, fd %d\n", io.id, io.width, io.height, io.bytes_per_row, io.pixel_format, (uintmax_t)io.alloc_size, io.fd);
	size_t len = (io.alloc_size + 4095) & ~(size_t)4095;
	uint8_t *m = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, io.fd, 0);
	if (m == MAP_FAILED) { perror("mmap"); return 1; }
	int bad = 0;
	for (uint32_t y = 0; y < io.height; y++) for (uint32_t x = 0; x < io.width; x++) if (((uint32_t *)(m + y * io.bytes_per_row))[x] != y * io.width + x) bad++;
	printf("reader: pattern %s (%d wrong)\n", bad ? "WRONG" : "ok", bad);
	((uint32_t *)m)[0] = 0xC0FFEE;
	return bad ? 1 : 0;
}
