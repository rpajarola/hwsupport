#!/bin/sh
# Standalone installer for MVME393 Xcfb Server
set -e

echo "Installing MVME393 Xcfb Color Framebuffer Server..."

[ -d /usr/bin/X11 ] || mkdir -p /usr/bin/X11
cp Xcfb /usr/bin/X11/Xcfb
chmod 755 /usr/bin/X11/Xcfb

cp startxcfb /usr/bin/X11/startxcfb
chmod 755 /usr/bin/X11/startxcfb

if [ -f x393inject ]; then
    cp x393inject /usr/bin/X11/x393inject
    chmod 755 /usr/bin/X11/x393inject
fi

if [ ! -c /dev/memregion/12 ]; then
    echo "Creating /dev/memregion/12..."
    mknod /dev/memregion/12 c 19 140
    chmod 666 /dev/memregion/12
fi

[ -d /tmp/.X11-unix ] || mkdir -p /tmp/.X11-unix
chmod 1777 /tmp/.X11-unix

if [ ! -f /usr/bin/X11/X ] && [ ! -h /usr/bin/X11/X ]; then
    ln -s /usr/bin/X11/Xcfb /usr/bin/X11/X
fi

echo "Installation complete!"
echo "Run /usr/bin/X11/startxcfb to launch X11 with Motif Window Manager."
