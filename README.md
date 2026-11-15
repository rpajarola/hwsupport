# Vintage Hardware Support & Driver Repository

A collection of hardware device drivers, X11 servers, kernel modules, firmware download utilities, and distribution packages for vintage UNIX platforms (Motorola System V/88, System V/68, etc.).

---

## Repository Index

| Architecture / OS | Hardware Board / Device | Component | Path | Description |
| :--- | :--- | :--- | :--- | :--- |
| **m88k (SysV/88 R4)** | **Motorola MVME393** | X11 Color Framebuffer Server (`Xcfb`) | [`sysv88k/mvme393/xcfb/`](sysv88k/mvme393/xcfb/) | Native 1024×768×8bpp X11R4 server, SVR4 datastream package, mouse/keyboard integration, and launcher |

---

## Directory Organization

```
hwsupport/
├── README.md                      # Top-level catalog and repository overview
└── sysv88k/                       # Motorola System V/88 (m88k)
    └── mvme393/                   # MVME393 VMEbus Graphics Controller
        ├── README.md              # Board overview
        └── xcfb/                  # X11R4 Color Framebuffer Server & Packages
            ├── README.md          # Comprehensive component documentation
            ├── packages/          # Ready-to-install SVR4 .pkg & tar.Z packages
            ├── packaging/         # SVR4 package creation scripts (pkgmk/pkgproto)
            ├── src/               # Native C source trees (Xcfb, x393inject)
            ├── docs/              # Hardware reference, installation guide, protocols
            └── screenshots/       # Visual verifications & desktop captures
```

---

## Quick Navigation

* **[Motorola MVME393 X Server (`Xcfb`)](sysv88k/mvme393/xcfb/README.md)**
  * [SVR4 Package (`xcfb-1.0.0-m88k.pkg`)](sysv88k/mvme393/xcfb/packages/xcfb-1.0.0-m88k.pkg)
  * [Standalone Tarball (`xcfb-1.0.0-m88k.tar.Z`)](sysv88k/mvme393/xcfb/packages/xcfb-1.0.0-m88k.tar.Z)
  * [Installation Guide](sysv88k/mvme393/xcfb/docs/installation_guide.md)
  * [Hardware Specification](sysv88k/mvme393/xcfb/docs/mvme393_hardware.md)
  * [DeltaWINDOWS Protocol Reference](sysv88k/mvme393/xcfb/docs/deltawindows_protocol.md)
