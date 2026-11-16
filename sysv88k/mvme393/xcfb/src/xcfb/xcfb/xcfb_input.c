/*
 * xcfb_input.c - Input devices (mouse & keyboard) and event loop
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>

#include "xcfb.h"
#include "keysym.h"
#include "inputstr.h"
#include "windowstr.h"

extern WindowPtr *WindowTable;

static DevicePtr xcfb_mouse_dev = NULL;
static DevicePtr xcfb_kbd_dev = NULL;
static int last_buttons = 0;
static unsigned long lastEventTime = 0;

static unsigned short *mvme393_dpr = NULL;
static int dpr_fd = -1;
static int cursor_initialized = 0;

void xcfbInitInput(void)
{
	caddr_t p;

	dpr_fd = open("/dev/mem", O_RDWR);
	if (dpr_fd >= 0) {
		p = (caddr_t)mmap((caddr_t)0, 0x10000, PROT_READ|PROT_WRITE,
				  MAP_SHARED, dpr_fd, (off_t)0xfd840000);
		if (p != (caddr_t)-1) {
			mvme393_dpr = (unsigned short *)p;
			mvme393_dpr[0xf2f6 / 2] = mvme393_dpr[0xf2f8 / 2];
			ErrorF("xcfb: MVME393 Dual-Port RAM mapped at 0xfd840000 (status=0x%04x, q_head=0x%04x)\n",
			       mvme393_dpr[0], mvme393_dpr[0xf2f8 / 2]);
		}
	}
}

Bool xcfbInputPending(void)
{
	if (!WindowTable || !WindowTable[0] || !WindowTable[0]->realized)
		return FALSE;
	if (xcfb_mouse_dev == NULL || !xcfb_mouse_dev->on)
		return FALSE;

	if (!cursor_initialized)
		return TRUE;

	if (xcfb_in != NULL) {
		if (xcfb_in->event_pending || xcfb_in->kbd_pending)
			return TRUE;
	}
	if (mvme393_dpr != NULL) {
		unsigned short tail = mvme393_dpr[0xf2f6 / 2];
		unsigned short head = mvme393_dpr[0xf2f8 / 2];
		if (head != tail)
			return TRUE;
	}
	return FALSE;
}

int TimeSinceLastInputEvent(void)
{
	struct timeval tv;
	unsigned long now;
	gettimeofday(&tv, 0);
	now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	if (lastEventTime == 0)
		lastEventTime = now;
	return (int)(now - lastEventTime);
}

Bool LegalModifier(key, pDev)
	unsigned int key;
	DevicePtr pDev;
{
	return TRUE;
}

static void xcfbBell(int percent, DeviceIntPtr device, pointer ctrl, int unused)
{
}

static void xcfbKbdCtrl(DeviceIntPtr device, KeybdCtrl *ctrl)
{
}

static void xcfbMouseCtrl(DeviceIntPtr device, PtrCtrl *ctrl)
{
}

static int xcfbMouseGetMotionEvents(DeviceIntPtr pDev, xTimecoord *buff,
	unsigned long start, unsigned long stop, ScreenPtr pScreen)
{
	return 0;
}

/* US ASCII Keymap for KeyCodes 8..140 */
static KeySym xcfb_map[] = {
	/* 8-15 */
	XK_BackSpace, NoSymbol,
	XK_Tab, NoSymbol,
	XK_Linefeed, NoSymbol,
	XK_Clear, NoSymbol,
	XK_Clear, NoSymbol,
	XK_Return, NoSymbol,
	XK_Next, NoSymbol,
	XK_Insert, NoSymbol,
	/* 16-23: Modifiers */
	XK_Shift_L, NoSymbol,
	XK_Control_L, NoSymbol,
	XK_Alt_L, NoSymbol,
	XK_Alt_R, NoSymbol,
	XK_Caps_Lock, NoSymbol,
	XK_Shift_R, NoSymbol,
	XK_Control_R, NoSymbol,
	XK_Meta_L, NoSymbol,
	/* 24-31: Navigation */
	XK_Left, NoSymbol,
	XK_Up, NoSymbol,
	XK_Right, NoSymbol,
	XK_Escape, NoSymbol,
	XK_Down, NoSymbol,
	XK_Home, NoSymbol,
	XK_End, NoSymbol,
	XK_Prior, NoSymbol,
	/* 32-39 */
	XK_space, XK_space,
	XK_exclam, NoSymbol,
	XK_quotedbl, NoSymbol,
	XK_numbersign, NoSymbol,
	XK_dollar, NoSymbol,
	XK_percent, NoSymbol,
	XK_ampersand, NoSymbol,
	XK_apostrophe, XK_quotedbl,
	/* 40-47 */
	XK_parenleft, NoSymbol,
	XK_parenright, NoSymbol,
	XK_asterisk, NoSymbol,
	XK_plus, NoSymbol,
	XK_comma, XK_less,
	XK_minus, XK_underscore,
	XK_period, XK_greater,
	XK_slash, XK_question,
	/* 48-57: '0' - '9' */
	XK_0, XK_parenright,
	XK_1, XK_exclam,
	XK_2, XK_at,
	XK_3, XK_numbersign,
	XK_4, XK_dollar,
	XK_5, XK_percent,
	XK_6, XK_asciicircum,
	XK_7, XK_ampersand,
	XK_8, XK_asterisk,
	XK_9, XK_parenleft,
	/* 58-64 */
	XK_colon, NoSymbol,
	XK_semicolon, XK_colon,
	XK_less, NoSymbol,
	XK_equal, XK_plus,
	XK_greater, NoSymbol,
	XK_question, NoSymbol,
	XK_at, NoSymbol,
	/* 65-90: 'A' - 'Z' */
	XK_a, XK_A,
	XK_b, XK_B,
	XK_c, XK_C,
	XK_d, XK_D,
	XK_e, XK_E,
	XK_f, XK_F,
	XK_g, XK_G,
	XK_h, XK_H,
	XK_i, XK_I,
	XK_j, XK_J,
	XK_k, XK_K,
	XK_l, XK_L,
	XK_m, XK_M,
	XK_n, XK_N,
	XK_o, XK_O,
	XK_p, XK_P,
	XK_q, XK_Q,
	XK_r, XK_R,
	XK_s, XK_S,
	XK_t, XK_T,
	XK_u, XK_U,
	XK_v, XK_V,
	XK_w, XK_W,
	XK_x, XK_X,
	XK_y, XK_Y,
	XK_z, XK_Z,
	/* 91-96 */
	XK_bracketleft, XK_braceleft,
	XK_backslash, XK_bar,
	XK_bracketright, XK_braceright,
	XK_asciicircum, NoSymbol,
	XK_underscore, NoSymbol,
	XK_grave, XK_asciitilde,
	/* 97-122: lowercase 'a' - 'z' */
	XK_a, XK_A,
	XK_b, XK_B,
	XK_c, XK_C,
	XK_d, XK_D,
	XK_e, XK_E,
	XK_f, XK_F,
	XK_g, XK_G,
	XK_h, XK_H,
	XK_i, XK_I,
	XK_j, XK_J,
	XK_k, XK_K,
	XK_l, XK_L,
	XK_m, XK_M,
	XK_n, XK_N,
	XK_o, XK_O,
	XK_p, XK_P,
	XK_q, XK_Q,
	XK_r, XK_R,
	XK_s, XK_S,
	XK_t, XK_T,
	XK_u, XK_U,
	XK_v, XK_V,
	XK_w, XK_W,
	XK_x, XK_X,
	XK_y, XK_Y,
	XK_z, XK_Z,
	/* 123-127 */
	XK_braceleft, NoSymbol,
	XK_bar, NoSymbol,
	XK_braceright, NoSymbol,
	XK_asciitilde, NoSymbol,
	XK_Delete, NoSymbol,
	/* 128-139: F1 - F12 */
	XK_F1, NoSymbol,
	XK_F2, NoSymbol,
	XK_F3, NoSymbol,
	XK_F4, NoSymbol,
	XK_F5, NoSymbol,
	XK_F6, NoSymbol,
	XK_F7, NoSymbol,
	XK_F8, NoSymbol,
	XK_F9, NoSymbol,
	XK_F10, NoSymbol,
	XK_F11, NoSymbol,
	XK_F12, NoSymbol,
	/* 140 */
	XK_Meta_R, NoSymbol
};

static KeySymsRec xcfbKeySyms = {
	xcfb_map,
	8,
	140,
	2
};

static CARD8 xcfbModMap[MAP_LENGTH];

int xcfbKeybdProc(DevicePtr pDev, int what)
{
	switch (what) {
	case DEVICE_INIT:
		xcfb_kbd_dev = pDev;
		memset(xcfbModMap, 0, sizeof(xcfbModMap));
		xcfbModMap[16] = ShiftMask;    /* Shift_L */
		xcfbModMap[21] = ShiftMask;    /* Shift_R */
		xcfbModMap[17] = ControlMask;  /* Control_L */
		xcfbModMap[22] = ControlMask;  /* Control_R */
		xcfbModMap[18] = Mod1Mask;     /* Alt_L */
		xcfbModMap[19] = Mod1Mask;     /* Alt_R */
		xcfbModMap[20] = LockMask;     /* Caps_Lock */
		xcfbModMap[23] = Mod4Mask;     /* Meta_L */
		xcfbModMap[140] = Mod4Mask;    /* Meta_R */
		InitKeyboardDeviceStruct(
			pDev,
			&xcfbKeySyms,
			xcfbModMap,
			xcfbBell,
			xcfbKbdCtrl);
		break;
	case DEVICE_ON:
		pDev->on = TRUE;
		break;
	case DEVICE_OFF:
	case DEVICE_CLOSE:
		pDev->on = FALSE;
		break;
	}
	return Success;
}

int xcfbMouseProc(DevicePtr pDev, int what)
{
	BYTE map[4];

	switch (what) {
	case DEVICE_INIT:
		xcfb_mouse_dev = pDev;
		map[1] = 1;
		map[2] = 2;
		map[3] = 3;
		InitPointerDeviceStruct(
			pDev,
			map,
			3,
			xcfbMouseGetMotionEvents,
			xcfbMouseCtrl,
			0);
		break;
	case DEVICE_ON:
		pDev->on = TRUE;
		break;
	case DEVICE_OFF:
	case DEVICE_CLOSE:
		pDev->on = FALSE;
		break;
	}
	return Success;
}

void xcfbProcessInputEvents(void)
{
	xEvent xE;
	int bdiff, btn;
	struct timeval tv;
	unsigned long now;
	int dx = 0, dy = 0, buttons = 0;

	if (!WindowTable || !WindowTable[0] || !WindowTable[0]->realized) {
		xcfbInputReady = 0;
		return;
	}
	if (!xcfb_mouse_dev || !xcfb_mouse_dev->on) {
		xcfbInputReady = 0;
		return;
	}

	gettimeofday(&tv, 0);
	now = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	lastEventTime = now;

	/* 0. Display mouse cursor sprite immediately once root window is ready */
	if (!cursor_initialized) {
		cursor_initialized = 1;
		miPointerMoveCursor(screenInfo.screens[0], 512, 384, TRUE);
	}

	/* 1. Framebuffer direct input structure at 0xfe000 */
	if (xcfb_in != NULL && (xcfb_in->event_pending || xcfb_in->kbd_pending)) {
		dx = (int)xcfb_in->dx;
		dy = (int)xcfb_in->dy;
		buttons = (int)xcfb_in->buttons;

		xcfb_in->dx = 0;
		xcfb_in->dy = 0;
		xcfb_in->event_pending = 0;

		/* Mouse movement */
		if (dx != 0 || dy != 0) {
			miPointerDeltaCursor(screenInfo.screens[0], dx, dy, TRUE);
		}

		/* Mouse buttons (1 = Left, 2 = Middle, 4 = Right) */
		bdiff = buttons ^ last_buttons;
		if (bdiff) {
			for (btn = 1; btn <= 3; btn++) {
				int mask = (btn == 1) ? 4 : (btn == 2 ? 2 : 1);
				if (bdiff & mask) {
					xE.u.u.type = (buttons & mask) ? ButtonPress : ButtonRelease;
					xE.u.u.detail = btn;
					xE.u.keyButtonPointer.time = now;
					(*xcfb_mouse_dev->processInputProc)(&xE, xcfb_mouse_dev, 1);
				}
			}
			last_buttons = buttons;
		}

		/* Keyboard events */
		if (xcfb_in->kbd_pending && xcfb_kbd_dev && xcfb_kbd_dev->on) {
			xE.u.u.type = xcfb_in->kbd_down ? KeyPress : KeyRelease;
			xE.u.u.detail = (KeyCode)xcfb_in->kbd_key;
			xE.u.keyButtonPointer.time = now;
			(*xcfb_kbd_dev->processInputProc)(&xE, xcfb_kbd_dev, 1);
			xcfb_in->kbd_pending = 0;
		}
	}

	/* 2. MVME393 Dual-Port RAM queue (if mapped) */
	if (mvme393_dpr != NULL) {
		unsigned short tail = mvme393_dpr[0xf2f6 / 2];
		unsigned short head = mvme393_dpr[0xf2f8 / 2];
		unsigned char *ring = (unsigned char *)mvme393_dpr + 0xf2fc;

		while (tail != head) {
			signed char rdx = (signed char)ring[(tail + 1) % 2048];
			signed char rdy = (signed char)ring[(tail + 2) % 2048];

			if (rdx != 0 || rdy != 0) {
				miPointerDeltaCursor(screenInfo.screens[0], (int)rdx, (int)rdy, TRUE);
			}

			tail = (tail + 3) % 2048;
		}
		mvme393_dpr[0xf2f6 / 2] = tail;
	}

	xcfbInputReady = 0;
}
