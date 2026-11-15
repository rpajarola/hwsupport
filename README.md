# Motorola MVME393 Hardware Support & X11 Server

Hardware support, driver source, distribution packages, and documentation for the **Motorola MVME393 VMEbus Color Graphics Controller** running on **Motorola System V/88 Release 4 (m88k SVR4)** and compatible VME single-board computers (MVME187, MVME188, MVME197).

![MVME393 X11 Desktop with Motif Window Manager and Mouse](screenshots/x11_mouse_input_color.png)

---

## Highlights

* **Authentic SVR4 Datastream Package:** Ready-to-install `xcfb-1.0.0-m88k.pkg` installable via standard `pkgadd` without compiler or third-party dependencies.
* **Native X11R4 Color Framebuffer Server (`Xcfb`):** 1024 × 768 × 8-bit pseudocolor display engine with 256-color palette.
* **Dual Input Integration:** Supports both direct VME memory-mapped event injection (`0xfe000`) and the Motorola DeltaWINDOWS Dual-Port RAM hardware ring queue (`0xfd840000`).
* **Motif Window Manager (`mwm`) Verified:** Fully tested and verified running OSF/Motif window manager, `xclock`, and X11 client suites.

---

## Screenshots

| X11 Desktop (mwm + xclock + mouse) | Analog Clock Detail |
| :---: | :---: |
| ![Full Screen](screenshots/x11_mouse_input_color.png) | ![Clock Detail](screenshots/x11_xclock_window.png) |

---

## Repository Structure

```
hwsupport/
├── packages/                  # Ready-to-install binary distribution archives
│   ├── xcfb-1.0.0-m88k.pkg    # Official SVR4 datastream package (pkgadd)
│   ├── xcfb-1.0.0-m88k.tar.Z  # Standalone compressed tarball (install.sh)
│   └── checksums.txt          # Cryptographic checksums (MD5 & SHA256)
├── packaging/                 # SVR4 package definitions & installer scripts
│   ├── pkginfo                # Package metadata
│   ├── prototype              # Package prototype file
│   ├── postinstall            # Automated environment & device setup script
│   ├── preremove              # Clean uninstallation script
│   ├── startxcfb              # Desktop & X server launcher
│   └── install.sh             # Standalone shell installer
├── src/
│   ├── xcfb/                  # Complete X11R4 Xcfb source tree for m88k SVR4
│   │   ├── Makefile           # Standalone build Makefile
│   │   ├── xcfb/              # MVME393 DDX driver (screen, input, palette)
│   │   ├── cfb/               # Color framebuffer graphics routines
│   │   ├── mfb/               # Monochrome framebuffer routines
│   │   ├── mi/                # Machine-independent graphics & cursor sprite
│   │   ├── dix/               # Device-independent X server core
│   │   ├── os/                # SVR4 OS interface
│   │   └── include/           # Server headers
│   └── tools/
│       └── x393inject.c       # Mouse & keyboard injector utility
├── screenshots/               # Framebuffer captures and visual verifications
└── docs/                      # Technical documentation
    ├── mvme393_hardware.md    # Hardware register & memory mapping reference
    ├── installation_guide.md  # Detailed installation walkthrough
    └── deltawindows_protocol.md# Firmware download & DPR queue protocol
```

---

## Quickstart: Installing the Package

### 1. Using SVR4 Package Manager (`pkgadd`) &mdash; Recommended
On your Motorola System V/88 system, run as root:

```sh
pkgadd -d packages/xcfb-1.0.0-m88k.pkg xcfb
```

The installer automatically creates `/dev/memregion/12` (`c 19 140`), prepares `/tmp/.X11-unix` with sticky permissions (`1777`), and links `/usr/bin/X11/X` to `/usr/bin/X11/Xcfb`.

Query package status:
```sh
pkginfo -l xcfb
```

### 2. Using Standalone Installer
```sh
zcat packages/xcfb-1.0.0-m88k.tar.Z | tar xvf -
cd xcfb-1.0.0-m88k && sh install.sh
```

---

## Running X11

To launch the X server together with the Motif Window Manager and desktop clock:

```sh
/usr/bin/X11/startxcfb
```

Or start manually:
```sh
/usr/bin/X11/Xcfb :0 &
export DISPLAY=:0.0
/usr/bin/X11/mwm &
/usr/bin/X11/xclock &
```

---

## Building from Source

To build `Xcfb` natively on a Motorola System V/88 machine:

```sh
cd src/xcfb
make
```

The resulting `Xcfb` binary will be produced in the current directory and can be stripped with `strip Xcfb`.

---

## Technical Details & Resolved Challenges

1. **`gettimeofday` ABI Mismatch on m88k SVR4:**
   Motorola's `libucb` implementation of `gettimeofday(struct timeval *tv, struct timezone *tz)` checks the second parameter (`tz`, passed in register `r3`). Legacy X11 compatibility macros defined `gettimeofday(a, b)` as `gettimeofday(a)`, leaving uninitialized register garbage in `r3` and causing a `st r0, r0, r24` SIGSEGV at `0x000a039c`. A dedicated trampoline in `xcfb_screen.c` zeroes the second argument via an unprototyped function pointer, eliminating the crash.

2. **C89 Variable Declarations:**
   Native SVR4 `cc` strictly enforces ANSI C89 declarations before executable statements. Several C99-style mid-block variable declarations in `cfbfillarc.c` and `cfbzerarc.c` were refactored into explicit compound statement blocks.

3. **`pScreen->RectIn` Null Wrapping:**
   Under certain window exposure conditions, `pScreen->RectIn` can be uninitialized during early server startup before visual map initialization. Explicit guard checks were introduced across `cfb*.c`, `misprite.c`, and `window.c`.

---

## License & Attribution

* X Window System core code is copyright &copy; The Open Group / Massachusetts Institute of Technology.
* MVME393 integration, packaging, and fixes are provided under the MIT License.
