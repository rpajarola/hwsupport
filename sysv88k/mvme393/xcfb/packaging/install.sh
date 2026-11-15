#!/bin/sh
# Standalone installer for MVME393 Xcfb Server
set -e

echo "=========================================================="
echo "  Installing MVME393 Xcfb Color Framebuffer Server"
echo "=========================================================="

[ -d /usr/bin/X11 ] || mkdir -p /usr/bin/X11

echo "Installing binaries into /usr/bin/X11..."
if [ -f Xcfb ]; then
    cp Xcfb /usr/bin/X11/Xcfb
    chmod 755 /usr/bin/X11/Xcfb
fi

if [ -f startxcfb ]; then
    cp startxcfb /usr/bin/X11/startxcfb
    chmod 755 /usr/bin/X11/startxcfb
fi

if [ -f enable-xdm ]; then
    cp enable-xdm /usr/bin/X11/enable-xdm
    chmod 755 /usr/bin/X11/enable-xdm
fi

if [ -f disable-xdm ]; then
    cp disable-xdm /usr/bin/X11/disable-xdm
    chmod 755 /usr/bin/X11/disable-xdm
fi

if [ -f x393inject ]; then
    cp x393inject /usr/bin/X11/x393inject
    chmod 755 /usr/bin/X11/x393inject
fi

# Ensure rgb database exists if included
if [ -f rgb.dir ] && [ -f rgb.pag ]; then
    [ -d /usr/lib/X11 ] || mkdir -p /usr/lib/X11
    cp rgb.dir rgb.pag /usr/lib/X11/
    chmod 644 /usr/lib/X11/rgb.dir /usr/lib/X11/rgb.pag
fi

# Device node for MVME393 VME A24 window
if [ ! -c /dev/memregion/12 ]; then
    echo "Creating /dev/memregion/12..."
    [ -d /dev/memregion ] || mkdir -p /dev/memregion
    mknod /dev/memregion/12 c 19 140
    chmod 666 /dev/memregion/12
fi

# UNIX domain socket directory for X11 clients
[ -d /tmp/.X11-unix ] || mkdir -p /tmp/.X11-unix
chmod 1777 /tmp/.X11-unix

# Default X server symlink
if [ ! -f /usr/bin/X11/X ] && [ ! -h /usr/bin/X11/X ]; then
    echo "Creating /usr/bin/X11/X -> /usr/bin/X11/Xcfb symlink..."
    ln -s /usr/bin/X11/Xcfb /usr/bin/X11/X
fi

echo "=========================================================="
echo "Installation complete!"
echo ""
echo "Options:"
echo "  1. Manual X11 session:  /usr/bin/X11/startxcfb"
echo "  2. Enable XDM login:    /usr/bin/X11/enable-xdm"
echo "  3. Disable XDM login:   /usr/bin/X11/disable-xdm"
echo "=========================================================="

if [ "$1" = "--enable-xdm" ]; then
    /usr/bin/X11/enable-xdm
fi

exit 0
