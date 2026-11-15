#
# Standalone Makefile for Xcfb (MVME393 X11R4 Server for Motorola SVR4 m88k)
#

CC = /bin/cc
CFLAGS = -O -DSYSV -DSVR4 -DMOTOROLA -D__m88k__ -DM40 -DSHAPE -DMITMISC -DTCPCONN -DUNIXCONN -DNDBM -DFONT_SNF -I./include -I./include/X11 -I./dix -I./mi -I./mfb -I./cfb -I./ext -I./Xdmcp -I./xcfb -I/usr/ucbinclude
LD = /usr/ccs/bin/ld
LDFLAGS =
LIBS = -L/usr/ucblib -ldbm -lucb -lsocket -lnsl -lm

DIX_OBJS = dix/atom.o dix/colormap.o dix/cursor.o dix/devices.o \
           dix/dispatch.o dix/dixfonts.o dix/dixutils.o dix/events.o \
           dix/extension.o dix/gc.o dix/globals.o dix/glyphcurs.o \
           dix/grabs.o dix/initatoms.o dix/main.o dix/property.o \
           dix/resource.o dix/swaprep.o dix/swapreq.o dix/tables.o \
           dix/window.o

OS_OBJS = os/access.o os/auth.o os/connection.o os/fonttype.o \
          os/io.o os/mitauth.o os/oscolor.o os/osfonts.o \
          os/osinit.o os/utils.o os/WaitFor.o os/xdmauth.o \
          os/xdmcp.o

SNF_OBJS = snf/snffonts.o
FONT_OBJS = fontutil/fontdir.o

MI_OBJS = mi/miarc.o mi/mibitblt.o mi/mibstore.o mi/micursor.o \
          mi/midash.o mi/midispcur.o mi/miexpose.o mi/mifillarc.o \
          mi/mifillrct.o mi/mifpolycon.o mi/miglblt.o mi/miinitext.o \
          mi/mipointer.o mi/mipoly.o mi/mipolycon.o mi/mipolygen.o \
          mi/mipolypnt.o mi/mipolyrect.o mi/mipolyseg.o mi/mipolytext.o \
          mi/mipolyutil.o mi/mipushpxl.o mi/miregion.o mi/mispans.o \
          mi/misprite.o mi/mivaltree.o mi/miwideline.o mi/miwindow.o \
          mi/mizerarc.o mi/mizerline.o

MFB_OBJS = mfb/maskbits.o mfb/mfbbitblt.o mfb/mfbbres.o mfb/mfbbresd.o \
           mfb/mfbbstore.o mfb/mfbclip.o mfb/mfbcmap.o mfb/mfbfillarc.o \
           mfb/mfbfillrct.o mfb/mfbfillsp.o mfb/mfbfont.o mfb/mfbgc.o \
           mfb/mfbgetsp.o mfb/mfbhrzvert.o mfb/mfbimage.o mfb/mfbline.o \
           mfb/mfbmisc.o mfb/mfbpixmap.o mfb/mfbpolypnt.o mfb/mfbpushpxl.o \
           mfb/mfbscrclse.o mfb/mfbscrinit.o mfb/mfbsetsp.o mfb/mfbtile.o \
           mfb/mfbwindow.o mfb/mfbzerarc.o mfb/mfbseg.o mfb/mfbpntwin.o \
           mfb/mfbpgbwht.o mfb/mfbpgbblak.o mfb/mfbpgbinv.o \
           mfb/mfbigbwht.o mfb/mfbigbblak.o \
           mfb/mfbpawhite.o mfb/mfbpablack.o mfb/mfbpainv.o \
           mfb/mfbtewhite.o mfb/mfbteblack.o

CFB_OBJS = cfb/cfb8bit.o cfb/cfbbitblt.o cfb/cfbbres.o cfb/cfbbresd.o \
           cfb/cfbbstore.o cfb/cfbcmap.o cfb/cfbfillarc.o cfb/cfbfillrct.o \
           cfb/cfbfillsp.o cfb/cfbgc.o cfb/cfbgetsp.o cfb/cfbglblt8.o \
           cfb/cfbhrzvert.o cfb/cfbimage.o cfb/cfbline.o cfb/cfbmskbits.o \
           cfb/cfbpixmap.o cfb/cfbpntwin.o cfb/cfbpolypnt.o cfb/cfbpush8.o \
           cfb/cfbrctstp8.o cfb/cfbscrinit.o cfb/cfbsetsp.o cfb/cfbteblt8.o \
           cfb/cfbtegblt.o cfb/cfbtileodd.o cfb/cfbwindow.o cfb/cfbzerarc.o \
           cfb/cfbseg.o

EXT_OBJS = ext/mitmisc.o ext/shape.o

XAU_OBJS = Xau/AuDispose.o Xau/AuFileName.o Xau/AuGetAddr.o Xau/AuLock.o \
           Xau/AuRead.o Xau/AuUnlock.o Xau/AuWrite.o

XDMCP_OBJS = Xdmcp/A8Eq.o Xdmcp/AA16.o Xdmcp/AA32.o Xdmcp/AA8.o \
             Xdmcp/AofA8.o Xdmcp/CA8.o Xdmcp/CmpKey.o \
             Xdmcp/DA16.o Xdmcp/DA32.o Xdmcp/DA8.o Xdmcp/DAofA8.o \
             Xdmcp/DecKey.o Xdmcp/Decrypt.o Xdmcp/Encrypt.o Xdmcp/Fill.o \
             Xdmcp/Flush.o Xdmcp/GenKey.o Xdmcp/IncKey.o Xdmcp/RA16.o \
             Xdmcp/RA32.o Xdmcp/RA8.o Xdmcp/RaA16.o Xdmcp/RaA32.o \
             Xdmcp/RaA8.o Xdmcp/RaAoA8.o Xdmcp/RAofA8.o Xdmcp/RC16.o \
             Xdmcp/RC32.o Xdmcp/RC8.o Xdmcp/RHead.o Xdmcp/RR.o \
             Xdmcp/WA16.o Xdmcp/WA32.o Xdmcp/WA8.o Xdmcp/WAofA8.o \
             Xdmcp/WC16.o Xdmcp/WC32.o Xdmcp/WC8.o Xdmcp/Whead.o

XCFB_OBJS = xcfb/xcfb_init.o xcfb/xcfb_screen.o xcfb/xcfb_input.o

ALL_OBJS = $(XCFB_OBJS) $(DIX_OBJS) $(OS_OBJS) $(FONT_OBJS) $(SNF_OBJS) \
           $(CFB_OBJS) $(MFB_OBJS) $(MI_OBJS) $(EXT_OBJS) $(XAU_OBJS) $(XDMCP_OBJS)

all: Xcfb rgb/rgb bdftosnf/bdftosnf

Xcfb: $(ALL_OBJS)
	$(CC) $(CFLAGS) -o Xcfb $(ALL_OBJS) $(LIBS)

rgb/rgb: rgb/rgb.c
	$(CC) -O -DSYSV -DSVR4 -DNDBM -I./include -I./include/X11 -I/usr/ucbinclude rgb/rgb.c -o rgb/rgb -L/usr/ucblib -ldbm -lucb

bdftosnf/bdftosnf: bdftosnf/bdftosnf.c bdftosnf/fontutil.c
	$(CC) $(CFLAGS) -o bdftosnf/bdftosnf bdftosnf/bdftosnf.c bdftosnf/fontutil.c

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

mfb/mfbseg.o: mfb/mfbline.c
	$(CC) $(CFLAGS) -DPOLYSEGMENT -c mfb/mfbline.c -o mfb/mfbseg.o

cfb/cfbseg.o: cfb/cfbline.c
	$(CC) $(CFLAGS) -DPOLYSEGMENT -c cfb/cfbline.c -o cfb/cfbseg.o

mfb/mfbpgbwht.o: mfb/mfbplygblt.c
	$(CC) $(CFLAGS) -DMFBPOLYGLYPHBLT=mfbPolyGlyphBltWhite -DOPEQ='|=' -c mfb/mfbplygblt.c -o mfb/mfbpgbwht.o

mfb/mfbpgbblak.o: mfb/mfbplygblt.c
	$(CC) $(CFLAGS) -DMFBPOLYGLYPHBLT=mfbPolyGlyphBltBlack -DOPEQ='&=~' -c mfb/mfbplygblt.c -o mfb/mfbpgbblak.o

mfb/mfbpgbinv.o: mfb/mfbplygblt.c
	$(CC) $(CFLAGS) -DMFBPOLYGLYPHBLT=mfbPolyGlyphBltInvert -DOPEQ='^=' -c mfb/mfbplygblt.c -o mfb/mfbpgbinv.o

mfb/mfbigbwht.o: mfb/mfbimggblt.c
	$(CC) $(CFLAGS) -DMFBIMAGEGLYPHBLT=mfbImageGlyphBltWhite -DOPEQ='|=' -c mfb/mfbimggblt.c -o mfb/mfbigbwht.o

mfb/mfbigbblak.o: mfb/mfbimggblt.c
	$(CC) $(CFLAGS) -DMFBIMAGEGLYPHBLT=mfbImageGlyphBltBlack -DOPEQ='&=~' -c mfb/mfbimggblt.c -o mfb/mfbigbblak.o

mfb/mfbpawhite.o: mfb/mfbpntarea.c
	$(CC) $(CFLAGS) -DMFBSOLIDFILLAREA=mfbSolidWhiteArea -DMFBSTIPPLEFILLAREA=mfbStippleWhiteArea -DOPEQ='|=' -DEQWHOLEWORD='=~0' -c mfb/mfbpntarea.c -o mfb/mfbpawhite.o

mfb/mfbpablack.o: mfb/mfbpntarea.c
	$(CC) $(CFLAGS) -DMFBSOLIDFILLAREA=mfbSolidBlackArea -DMFBSTIPPLEFILLAREA=mfbStippleBlackArea -DOPEQ='&=~' -DEQWHOLEWORD='=0' -c mfb/mfbpntarea.c -o mfb/mfbpablack.o

mfb/mfbpainv.o: mfb/mfbpntarea.c
	$(CC) $(CFLAGS) -DMFBSOLIDFILLAREA=mfbSolidInvertArea -DMFBSTIPPLEFILLAREA=mfbStippleInvertArea -DOPEQ='^=' -DEQWHOLEWORD='^=~0' -c mfb/mfbpntarea.c -o mfb/mfbpainv.o

mfb/mfbtewhite.o: mfb/mfbtegblt.c
	$(CC) $(CFLAGS) -DMFBTEGLYPHBLT=mfbTEGlyphBltWhite -DOP= -DCLIPTETEXT=mfbImageGlyphBltWhite -c mfb/mfbtegblt.c -o mfb/mfbtewhite.o

mfb/mfbteblack.o: mfb/mfbtegblt.c
	$(CC) $(CFLAGS) -DMFBTEGLYPHBLT=mfbTEGlyphBltBlack -DOP='~' -DCLIPTETEXT=mfbImageGlyphBltBlack -c mfb/mfbtegblt.c -o mfb/mfbteblack.o

clean:
	rm -f $(ALL_OBJS) Xcfb
