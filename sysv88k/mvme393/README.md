# Motorola MVME393 Hardware Support for System V/88 (m88k)

This directory contains hardware drivers, display servers, utilities, and documentation for the **Motorola MVME393 VMEbus Graphics Controller** on **Motorola System V/88 Release 4 (m88k SVR4)** running on Motorola single-board computers (MVME187, MVME188, MVME197).

## Components

* **[`xcfb/`](xcfb/)**: **X11R4 Color Framebuffer Server (`Xcfb 1.0.0`)**
  * Native 1024 × 768 × 8-bit pseudocolor X server.
  * Official SVR4 datastream package (`xcfb-1.0.0-m88k.pkg`) for `pkgadd`.
  * Standalone compressed distribution archive (`xcfb-1.0.0-m88k.tar.Z`) with automated `install.sh`.
  * Input integration supporting both direct memory staging (`0xfe000`) and the hardware Dual-Port RAM ring queue (`0xfd840000`).
  * Automated launcher (`startxcfb`) for X11 + Motif Window Manager (`mwm`) + `xclock`.
  * Technical hardware specifications, protocol docs, and desktop screenshots.
