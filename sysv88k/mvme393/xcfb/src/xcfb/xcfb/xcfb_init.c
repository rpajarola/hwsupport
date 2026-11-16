/*
 * xcfb_init.c - Server initialization and input processing
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>

#include "xcfb.h"
#include "xcfb_version.h"
#include "opaque.h"

#define NUMFORMATS 2
static PixmapFormatRec formats[] = {
	{ 1, 1, BITMAP_SCANLINE_PAD },   /* 1-bit deep */
	{ 8, 8, BITMAP_SCANLINE_PAD },   /* 8-bit deep */
};

long xcfbInputReady = 0;
static long zero = 0;

void InitOutput(ScreenInfo *screenInfo, int argc, char **argv)
{
	int i;

	screenInfo->imageByteOrder = IMAGE_BYTE_ORDER;
	screenInfo->bitmapScanlineUnit = BITMAP_SCANLINE_UNIT;
	screenInfo->bitmapScanlinePad = BITMAP_SCANLINE_PAD;
	screenInfo->bitmapBitOrder = BITMAP_BIT_ORDER;

	screenInfo->numPixmapFormats = NUMFORMATS;
	for (i = 0; i < NUMFORMATS; i++)
		screenInfo->formats[i] = formats[i];

	AddScreen(xcfbScreenInit, argc, argv);
}

void InitInput(int argc, char **argv)
{
	DevicePtr pMouse, pKbd;

	xcfbInitInput();

	pMouse = AddInputDevice(xcfbMouseProc, TRUE);
	pKbd = AddInputDevice(xcfbKeybdProc, TRUE);

	if (!pMouse || !pKbd) {
		FatalError("xcfb: failed to create input devices\n");
	}

	RegisterPointerDevice(pMouse);
	RegisterKeyboardDevice(pKbd);

	if (screenInfo.numScreens > 0)
		miRegisterPointerDevice(screenInfo.screens[0], pMouse);

	SetInputCheck(&zero, &xcfbInputReady);
}

void ProcessInputEvents(void)
{
	xcfbProcessInputEvents();
}

void AbortDDX(void)
{
}

void ddxGiveUp(void)
{
}

int ddxProcessArgument(int argc, char *argv[], int i)
{
	if (strcmp(argv[i], "-version") == 0) {
		ErrorF("Xcfb version %s for Motorola System V/88 (m88k)\n", XCFB_VERSION_STRING);
		exit(0);
	}
	return 0;
}

void ddxUseMsg(void)
{
	ErrorF("Xcfb %s - Motorola MVME393 Color Framebuffer Server\n", XCFB_VERSION_STRING);
	ErrorF("-version               print server version and exit\n");
}
