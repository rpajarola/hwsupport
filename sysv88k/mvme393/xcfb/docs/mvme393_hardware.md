# Motorola MVME393 VMEbus Graphics Controller Technical Reference

## 1. Overview
The **Motorola MVME393** is a high-performance 6U VMEbus graphics display controller designed for UNIX workstations and embedded industrial display systems. It provides high-resolution 8-bit pseudocolor graphics with dedicated hardware co-processors for graphics acceleration and communication.

## 2. Hardware Architecture
* **Display Resolution:** 1024 × 768 pixels, 8 bits per pixel (256 colors from a 16.7-million color palette).
* **Frame Buffer Memory:** 1 MB to 2 MB high-speed video RAM (VRAM).
* **Onboard Processors:**
  * **Motorola MC68010 (10 MHz):** System supervisor handling VMEbus protocol communication, interrupt generation, circular packet queues, and host coordination.
  * **Texas Instruments TMS34010 Graphics System Processor (GSP):** Dedicated 32-bit graphics processor handling raster operations, polygon fills, line drawing, font rendering, and hardware BitBLT.
* **Dual-Port RAM (DPR):** 512 KB onboard SRAM mapped into VMEbus space for bidirectional communication between the VME host CPU (e.g., MVME187 / MVME188) and the onboard 68010 co-processor.
* **Color Palette (RAMDAC):** Inmos IMSG171 or Brooktree Bt478 color palette DAC providing 256 programmable color registers with 8-bit R, G, B channels.

---

## 3. VMEbus Address Mapping

The MVME393 responds to both VMEbus A24 and A32 addressing modes depending on jumper configuration:

### A24 Memory Region (Standard Unix Mapping)
Under Motorola System V/88 and System V/68, the framebuffer is accessible through the memory window driver `/dev/memregion/12` (major 19, minor 140):
* **Base Physical Address:** `0xfd000000` (mapped into VME A24 space)
* **Window Size:** `0x100000` (1 MB)
* **VRAM Plane:** `0x00000` &ndash; `0xbffff` (1024 × 768 × 1 byte = 786,432 bytes)
* **Host Direct Input Register:** `0xfe000` (Relative dx, dy, buttons, keycode, pending flags)
* **RAMDAC Palette Register:** `0xff000` (256 × 4-byte 32-bit ARGB/XRGB color cube entries)

### A32 Dual-Port RAM Region
* **Base Physical Address:** `0xfd800000` &ndash; `0xfd87ffff` (512 KB DPR)
* **Status Register:** `0xfd800000`
* **Packet Ring Buffer:** `0xfd840000` &ndash; `0xfd84ffff`
  * DPR Tail Pointer: Offset `0xf2f6`
  * DPR Head Pointer: Offset `0xf2f8`
  * Ring Queue Buffer: Offset `0xf2fc` &ndash; `0xfaff` (2048 bytes)

---

## 4. Input Delivery Channels
1. **Direct VME Memory Staging (`0xfe000`):**
   The host X server writes/reads mouse and keyboard state directly in the memory-mapped region at `0xfe000`:
   ```c
   struct xcfb_input {
       long dx;                   /* Relative mouse X movement */
       long dy;                   /* Relative mouse Y movement */
       long buttons;              /* Button mask (bit 0=R, 1=M, 2=L) */
       unsigned long event_pending;/* Set to 1 by injector/emulator, 0 by X server */
       unsigned long kbd_key;     /* Keycode (8..127 ASCII) */
       unsigned long kbd_down;    /* 1 = KeyPress, 0 = KeyRelease */
       unsigned long kbd_pending; /* Set to 1 by injector/emulator */
   };
   ```

2. **Dual-Port RAM Packet Ring (`0xfd840000`):**
   Firmware on the MVME393's 68010 inserts 3-byte packets `[button_state, delta_x, delta_y]` into the circular ring buffer starting at `0xf2fc`. The X server polls the head and tail pointers during `xcfbInputPending()` and drains events via `miPointerDeltaCursor()`.
