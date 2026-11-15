/*
 * xcfb_screen.c - Screen initialization and colormap management
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>

#include "xcfb.h"
#include "resource.h"
#include "windowstr.h"

unsigned char *xcfb_fb = NULL;
struct xcfb_input *xcfb_in = NULL;
unsigned long *xcfb_pal = NULL;

static int fb_fd = -1;

int xcfb_gettimeofday(struct timeval *tv, void *tz)
{
	int (*real_gtod)() = (int (*)())gettimeofday;
	return (*real_gtod)(tv, (void *)0);
}

static long xcfbEventTime(ScreenPtr pScreen)
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

static Bool xcfbCursorOffScreen(ScreenPtr *pScreen, int *x, int *y)
{
	return FALSE;
}

static void xcfbCrossScreen(ScreenPtr pScreen, Bool entering)
{
}

extern void miPointerQueueEvent();

static miPointerCursorFuncRec xcfbPointerCursorFuncs = {
	xcfbEventTime,
	xcfbCursorOffScreen,
	xcfbCrossScreen,
	miPointerQueueEvent
};

void xcfbBlockHandler(int index, pointer blockData, pointer pTimeout, pointer pReadmask)
{
	static struct timeval poll_tv = { 0, 20000 };
	static struct timeval zero_tv = { 0, 0 };
	struct timeval **tvp = (struct timeval **)pTimeout;

	if (!tvp)
		return;

	if (xcfbInputPending()) {
		*tvp = &zero_tv;
		xcfbInputReady++;
	} else {
		if (*tvp == NULL || (*tvp)->tv_sec > 0 || (*tvp)->tv_usec > 20000) {
			*tvp = &poll_tv;
		}
	}
}

void xcfbWakeupHandler(int index, pointer blockData, unsigned long result, pointer pReadmask)
{
	if (xcfbInputPending()) {
		xcfbInputReady++;
	}
}

static int xcfbOpenFramebuffer(void)
{
	caddr_t addr;

	/* Try /dev/memregion/12 first */
	fb_fd = open("/dev/memregion/12", O_RDWR);
	if (fb_fd < 0) {
		/* Try /dev/mem at 0xfd000000 */
		fb_fd = open("/dev/mem", O_RDWR);
		if (fb_fd >= 0) {
			addr = mmap((caddr_t)0, XCFB_MEM_SIZE, PROT_READ|PROT_WRITE,
				MAP_SHARED, fb_fd, (off_t)0xfd000000);
			if (addr != (caddr_t)-1) {
				xcfb_fb = (unsigned char *)addr;
				xcfb_in = (struct xcfb_input *)(xcfb_fb + XCFB_INPUT_OFFSET);
				xcfb_pal = (unsigned long *)(xcfb_fb + XCFB_PALETTE_OFFSET);
				return 0;
			}
			close(fb_fd);
			fb_fd = -1;
		}
		/* Fallback: allocate anonymous memory for testing */
		ErrorF("xcfb: Warning, could not open /dev/memregion/12 or /dev/mem, using malloc\n");
		xcfb_fb = (unsigned char *)malloc(XCFB_MEM_SIZE);
		if (!xcfb_fb) return -1;
		memset(xcfb_fb, 0, XCFB_MEM_SIZE);
		xcfb_in = (struct xcfb_input *)(xcfb_fb + XCFB_INPUT_OFFSET);
		xcfb_pal = (unsigned long *)(xcfb_fb + XCFB_PALETTE_OFFSET);
		return 0;
	}

	addr = mmap((caddr_t)0, XCFB_MEM_SIZE, PROT_READ|PROT_WRITE,
		MAP_SHARED, fb_fd, (off_t)0);
	if (addr == (caddr_t)-1) {
		ErrorF("xcfb: mmap failed\n");
		close(fb_fd);
		return -1;
	}

	xcfb_fb = (unsigned char *)addr;
	xcfb_in = (struct xcfb_input *)(xcfb_fb + XCFB_INPUT_OFFSET);
	memset((void *)xcfb_in, 0, sizeof(struct xcfb_input));
	xcfb_pal = (unsigned long *)(xcfb_fb + XCFB_PALETTE_OFFSET);
	return 0;
}

static ColormapPtr xcfbInstalledMap = NULL;

static void xcfbInstallColormap(ColormapPtr cmap)
{
	int i;
	Entry *pent;
	VisualPtr pVisual = cmap->pVisual;
	unsigned long r, g, b;

	if (cmap == xcfbInstalledMap)
		return;

	if (xcfbInstalledMap)
		WalkTree(xcfbInstalledMap->pScreen, TellLostMap, (char *) &(xcfbInstalledMap->mid));

	if ((pVisual->class | DynamicClass) == DirectColor) {
		for (i = 0; i < 256; i++) {
			pent = &cmap->red[(i & pVisual->redMask) >> pVisual->offsetRed];
			r = (pent->co.local.red >> 8) & 0xff;
			pent = &cmap->green[(i & pVisual->greenMask) >> pVisual->offsetGreen];
			g = (pent->co.local.green >> 8) & 0xff;
			pent = &cmap->blue[(i & pVisual->blueMask) >> pVisual->offsetBlue];
			b = (pent->co.local.blue >> 8) & 0xff;
			if (xcfb_pal) xcfb_pal[i] = (r << 16) | (g << 8) | b;
		}
	} else {
		for (i = 0, pent = cmap->red; i < pVisual->ColormapEntries && i < 256; i++, pent++) {
			if (pent->fShared) {
				r = (pent->co.shco.red->color >> 8) & 0xff;
				g = (pent->co.shco.green->color >> 8) & 0xff;
				b = (pent->co.shco.blue->color >> 8) & 0xff;
			} else {
				r = (pent->co.local.red >> 8) & 0xff;
				g = (pent->co.local.green >> 8) & 0xff;
				b = (pent->co.local.blue >> 8) & 0xff;
			}
			if (xcfb_pal) xcfb_pal[i] = (r << 16) | (g << 8) | b;
		}
	}
	xcfbInstalledMap = cmap;
	WalkTree(cmap->pScreen, TellGainedMap, (char *) &(cmap->mid));
}

static void xcfbUninstallColormap(ColormapPtr cmap)
{
	if (cmap == xcfbInstalledMap) {
		Colormap defMapID = cmap->pScreen->defColormap;
		if (cmap->mid != defMapID) {
			ColormapPtr defMap = (ColormapPtr) LookupIDByType(defMapID, RT_COLORMAP);
			if (defMap)
				(*cmap->pScreen->InstallColormap)(defMap);
		}
	}
}

static int xcfbListInstalledColormaps(ScreenPtr pScreen, Colormap *pCmapList)
{
	if (xcfbInstalledMap) {
		*pCmapList = xcfbInstalledMap->mid;
		return 1;
	}
	return 0;
}

Bool xcfbScreenInit(int index, ScreenPtr pScreen, int argc, char **argv)
{
	ErrorF("xcfbScreenInit: entering, index=%d\n", index);
	if (!xcfb_fb) {
		if (xcfbOpenFramebuffer() < 0)
			return FALSE;
	}

	ErrorF("xcfbScreenInit: calling cfbScreenInit...\n");
	if (!cfbScreenInit(pScreen, (pointer)xcfb_fb,
			XCFB_WIDTH, XCFB_HEIGHT,
			XCFB_DPI, XCFB_DPI, XCFB_WIDTH)) {
		ErrorF("xcfbScreenInit: cfbScreenInit failed!\n");
		return FALSE;
	}

	pScreen->CloseScreen = xcfbCloseScreen;
	pScreen->SaveScreen = xcfbSaveScreen;
	pScreen->InstallColormap = xcfbInstallColormap;
	pScreen->UninstallColormap = xcfbUninstallColormap;
	pScreen->ListInstalledColormaps = xcfbListInstalledColormaps;
	pScreen->StoreColors = xcfbStoreColors;

	pScreen->BlockHandler = xcfbBlockHandler;
	pScreen->WakeupHandler = xcfbWakeupHandler;

	ErrorF("xcfbScreenInit: calling miDCInitialize...\n");
	miDCInitialize(pScreen, &xcfbPointerCursorFuncs);

	ErrorF("xcfbScreenInit: calling cfbCreateDefColormap...\n");
	if (!cfbCreateDefColormap(pScreen)) {
		ErrorF("xcfbScreenInit: cfbCreateDefColormap failed!\n");
		return FALSE;
	}
	ErrorF("xcfbScreenInit: completed successfully!\n");

	return TRUE;
}

Bool xcfbCloseScreen(int index, ScreenPtr pScreen)
{
	return TRUE;
}

Bool xcfbSaveScreen(ScreenPtr pScreen, int on)
{
	return TRUE;
}

void xcfbStoreColors(ColormapPtr pmap, int ndef, xColorItem *pdefs)
{
	int i, px;
	unsigned long r, g, b;

	if (pmap != xcfbInstalledMap)
		return;

	for (i = 0; i < ndef; i++) {
		px = pdefs[i].pixel;
		if (px >= 0 && px < 256 && xcfb_pal != NULL) {
			r = (pdefs[i].red >> 8) & 0xff;
			g = (pdefs[i].green >> 8) & 0xff;
			b = (pdefs[i].blue >> 8) & 0xff;
			xcfb_pal[px] = (r << 16) | (g << 8) | b;
		}
	}
}
