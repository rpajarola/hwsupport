/*
 * x393inject.c - Mouse and keyboard input injector for Motorola MVME393
 *
 * Supports two input delivery paths:
 * 1. Framebuffer direct input structure at offset 0xfe000 (/dev/memregion/12)
 * 2. MVME393 Dual-Port RAM packet ring buffer at 0xfd840000 (/dev/mem)
 *
 * For Motorola System V/88 Release 4 on MVME187 / MVME188
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <poll.h>

struct xcfb_input {
	long dx;
	long dy;
	long buttons;
	unsigned long event_pending;
	unsigned long kbd_key;
	unsigned long kbd_down;
	unsigned long kbd_pending;
};

static void usage(const char *prog)
{
	fprintf(stderr, "Usage: %s [-m dx dy buttons] [-k key down|up] [-d]\n", prog);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -m dx dy buttons   Inject mouse motion (dx, dy) and button state (1=L, 2=M, 4=R)\n");
	fprintf(stderr, "  -k keycode 0|1     Inject keycode event (1=press, 0=release)\n");
	fprintf(stderr, "  -d                 Run default automated demo test suite\n");
	exit(1);
}

int main(int argc, char **argv)
{
	int fd = open("/dev/memregion/12", O_RDWR);
	int mem_fd;
	char *addr;
	struct xcfb_input *in;
	int i;

	if (fd < 0) {
		perror("open /dev/memregion/12");
		return 1;
	}

	addr = (char *)mmap((caddr_t)0, 0x100000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == (char *)-1) {
		perror("mmap /dev/memregion/12");
		close(fd);
		return 1;
	}

	in = (struct xcfb_input *)(addr + 0xfe000);

	if (argc >= 5 && argv[1][0] == '-' && argv[1][1] == 'm') {
		int dx = atoi(argv[2]);
		int dy = atoi(argv[3]);
		int buttons = atoi(argv[4]);
		in->dx += dx;
		in->dy += dy;
		in->buttons = buttons;
		in->event_pending = 1;
		printf("Injected mouse: dx=%d, dy=%d, buttons=%d\n", dx, dy, buttons);
	} else if (argc >= 4 && argv[1][0] == '-' && argv[1][1] == 'k') {
		int key = atoi(argv[2]);
		int down = atoi(argv[3]);
		in->kbd_key = key;
		in->kbd_down = down ? 1 : 0;
		in->kbd_pending = 1;
		printf("Injected key: code=%d, state=%s\n", key, down ? "down" : "up");
	} else {
		/* Default automated demonstration sequence */
		printf("Injecting mouse motion: moving pointer to center (dx=20, dy=16 x 20 steps)...\n");
		for (i = 0; i < 20; i++) {
			in->dx += 20;
			in->dy += 16;
			in->event_pending = 1;
			poll(0, 0, 50);
		}

		printf("Clicking left mouse button (press)...\n");
		in->buttons = 4;
		in->event_pending = 1;
		poll(0, 0, 100);

		printf("Releasing left mouse button (release)...\n");
		in->buttons = 0;
		in->event_pending = 1;
		poll(0, 0, 100);

		/* Also test hardware DPR ring queue if /dev/mem is accessible */
		mem_fd = open("/dev/mem", O_RDWR);
		if (mem_fd >= 0) {
			char *dpr = (char *)mmap((caddr_t)0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (off_t)0xfd840000);
			if (dpr != (char *)-1) {
				unsigned short *dpr_w = (unsigned short *)dpr;
				unsigned char *ring = (unsigned char *)dpr + 0xf2fc;
				unsigned short head = dpr_w[0xf2f8 / 2];
				printf("Injecting packets into MVME393 DPR queue at 0xfd840000...\n");
				for (i = 0; i < 10; i++) {
					ring[head % 2048] = 0;
					ring[(head + 1) % 2048] = (unsigned char)10;
					ring[(head + 2) % 2048] = (unsigned char)10;
					head = (head + 3) % 2048;
					dpr_w[0xf2f8 / 2] = head;
					poll(0, 0, 50);
				}
				munmap(dpr, 0x10000);
			}
			close(mem_fd);
		}

		printf("Injection test completed!\n");
	}

	munmap(addr, 0x100000);
	close(fd);
	return 0;
}
