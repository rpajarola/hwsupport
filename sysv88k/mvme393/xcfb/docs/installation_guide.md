# MVME393 X Server Installation & Operation Guide

This guide describes how to install and run the native X11R4 `Xcfb` server on **Motorola System V/88 Release 4 (SVR4 R40V4.x)** for Motorola MVME187, MVME188, and MVME197 platforms.

---

## 1. System Requirements
* **Architecture:** Motorola 88000 RISC (m88k)
* **Operating System:** Motorola System V/88 Release 4 (R40V4.2, R40V4.3, R40V4.4)
* **Graphics Hardware:** Motorola MVME393 Color Framebuffer Board
* **Disk Space:** ~2.5 MB free in `/usr` and `/tmp`

---

## 2. Installation Methods

### Method 1: SVR4 Datastream Package (`pkgadd`) &mdash; Recommended
The SVR4 datastream package is located in `packages/xcfb-1.0.0-m88k.pkg`. It installs non-interactively and sets up all required system dependencies automatically.

```sh
# pkgadd -d /path/to/xcfb-1.0.0-m88k.pkg xcfb
```

During installation, the package's `postinstall` script automatically:
1. Checks for `/dev/memregion/12` and creates `mknod /dev/memregion/12 c 19 140` (mode 0666) if missing.
2. Creates the `/tmp/.X11-unix` socket directory with sticky mode 1777.
3. Symlinks `/usr/bin/X11/X` to `/usr/bin/X11/Xcfb`.

#### Verifying Installation
```sh
# pkginfo -l xcfb
   PKGINST:  xcfb
      NAME:  MVME393 Color Framebuffer X Server (X11R4)
  CATEGORY:  graphics,system
      ARCH:  m88k
   VERSION:  1.0.0
    VENDOR:  Motorola / Open Source Community
    STATUS:  completely installed
```

#### Uninstalling
```sh
# pkgrm xcfb
```

---

### Method 2: Standalone Archive (`install.sh`)
If installing from the compressed tarball `packages/xcfb-1.0.0-m88k.tar.Z`:

```sh
# zcat xcfb-1.0.0-m88k.tar.Z | tar xvf -
# cd xcfb-1.0.0-m88k
# sh install.sh
```

To install and immediately enable XDM graphical login in a single command:
```sh
# sh install.sh --enable-xdm
```

---

## 3. Starting the X Server

### Automated Desktop Launcher
The package installs `/usr/bin/X11/startxcfb`, which handles display socket cleanup, launches the server, verifies connectivity, and starts the window manager:

```sh
# /usr/bin/X11/startxcfb
```

If `$HOME/.xinitrc` exists, `startxcfb` will execute it. Otherwise, it defaults to:
* Motif Window Manager (`mwm`)
* Analog clock (`xclock -geometry 160x160+20+20`)
* Terminal (`xterm`)

### Manual Startup
To run the server manually in the background:
```sh
# /usr/bin/X11/Xcfb :0 &
# export DISPLAY=:0.0
# /usr/bin/X11/mwm &
# /usr/bin/X11/xclock &
```

---

## 4. XDM Graphical Login Integration

To configure the Motorola SVR4 system to start the X Display Manager on display `:0` and present a graphical login prompt automatically on boot:

```sh
# /usr/bin/X11/enable-xdm
```

To revert back to console login and disable XDM:
```sh
# /usr/bin/X11/disable-xdm
```

For full details on XDM configuration, MIT magic cookie authorization, and `/etc/default/login` settings, see [docs/xdm_setup.md](xdm_setup.md).

---

## 5. Transferring to Real Hardware

### Via SCSI Tape
```sh
tar cvf /dev/nst0 xcfb-1.0.0-m88k.pkg
```
On the SysV/88 machine:
```sh
tar xvf /dev/rmt/c0d4s0
pkgadd -d xcfb-1.0.0-m88k.pkg xcfb
```

### Via Raw SCSI Disk Partition
```sh
dd if=packages/xcfb-1.0.0-m88k.pkg of=/dev/sdX bs=64k
```
On SysV/88:
```sh
pkgadd -d /dev/rdsk/m187_c0d1s7 xcfb
```
