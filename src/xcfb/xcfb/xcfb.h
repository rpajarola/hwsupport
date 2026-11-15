/*
 * xcfb.h - MVME393 Color Framebuffer X Server Header
 */

#ifndef _XCFB_H_
#define _XCFB_H_

struct timeval;
extern int xcfb_gettimeofday(struct timeval *tv, void *tz);
#ifndef gettimeofday
#define gettimeofday(a, b) xcfb_gettimeofday(a, b)
#endif


#include "X.h"
#define NEED_EVENTS
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#include "input.h"
#include "colormap.h"
#include "colormapst.h"
#include "cfb.h"
#include "mi.h"
#include "mipointer.h"

#define XCFB_WIDTH       1024
#define XCFB_HEIGHT      768
#define XCFB_DEPTH       8
#define XCFB_DPI         75
#define XCFB_MEM_SIZE    0x100000        /* 1 MB mapping */

/* Hardware control registers in the upper 64 KB of the 1 MB FB region */
#define XCFB_INPUT_OFFSET   0xfe000
#define XCFB_PALETTE_OFFSET 0xff000

struct xcfb_input {
	long dx;
	long dy;
	long buttons;
	unsigned long event_pending;
	unsigned long kbd_key;
	unsigned long kbd_down;
	unsigned long kbd_pending;
};

extern unsigned char *xcfb_fb;
extern struct xcfb_input *xcfb_in;
extern unsigned long *xcfb_pal;

extern Bool xcfbScreenInit(int index, ScreenPtr pScreen, int argc, char **argv);
extern Bool xcfbCloseScreen(int index, ScreenPtr pScreen);
extern Bool xcfbSaveScreen(ScreenPtr pScreen, int on);
extern void xcfbStoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs);

extern int xcfbInputReady;
extern void xcfbInitInput(void);
extern Bool xcfbInputPending(void);
extern void xcfbBlockHandler(int index, pointer blockData, pointer pTimeout, pointer pReadmask);
extern void xcfbWakeupHandler(int index, pointer blockData, unsigned long result, pointer pReadmask);

extern int xcfbMouseProc(DevicePtr pDev, int what);
extern int xcfbKeybdProc(DevicePtr pDev, int what);
extern void xcfbProcessInputEvents(void);

#endif /* _XCFB_H_ */
