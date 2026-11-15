# Motorola DeltaWINDOWS MVME393 Driver & Packet Protocol

## 1. Background
In Motorola DeltaWINDOWS (System V/68 Release 3 and early System V/88), the MVME393 graphics controller was driven by a multi-part architecture:
1. `393dl`: User-space firmware download utility that transfers 68010 control firmware and TMS34010 GSP firmware into the MVME393 Dual-Port RAM.
2. `x393.o`: Kernel character/packet-queue driver providing `/dev/393` and ioctl interfaces.
3. `Xcfb`: X11 server communicating with the hardware framebuffer and reading mouse/keyboard input from the DPR circular packet queue.

---

## 2. Firmware Download Protocol (`393dl`)

Firmware downloads are coordinated through command packets placed in the Dual-Port RAM window:
* **Base Register Address:** `0xfd800000`
* **Packet Structure:**
  * Byte 0 &ndash; 1: Command Opcode
    * `Opcode 4`: Start TMS34010 execution
    * `Opcode 5`: 68010 code download
    * `Opcode 6`: TMS34010 code / VRAM download
  * Byte 2 &ndash; 3: Target count (bytes or words)
  * Byte 4 &ndash; 7: Target start address in MVME393 memory
  * Byte 8 &ndash; 11: Status / Completion flag
  * Byte 12 &ndash; 13: Chunk flag (1 = more chunks to follow, 0 = final chunk)
  * Byte 14+: Payload data

---

## 3. Circular Input Queue Protocol

Once the 68010 firmware is running, user input events (from the serial mouse or keyboard attached to the MVME393 serial port) are placed in a 2048-byte circular ring buffer in Dual-Port RAM:
* **Tail Offset Pointer:** `0xfd84f2f6` (16-bit word, updated by host X server)
* **Head Offset Pointer:** `0xfd84f2f8` (16-bit word, updated by 68010 firmware)
* **Ring Base:** `0xfd84f2fc`

### Packet Format (3 bytes per event):
* `Byte 0`: Button status mask (Bit 0: Right, Bit 1: Middle, Bit 2: Left)
* `Byte 1`: Delta X (signed 8-bit integer, -128 .. +127)
* `Byte 2`: Delta Y (signed 8-bit integer, -128 .. +127)

### Queue Processing Algorithm:
```c
unsigned short tail = mvme393_dpr[0xf2f6 / 2];
unsigned short head = mvme393_dpr[0xf2f8 / 2];
unsigned char *ring = (unsigned char *)mvme393_dpr + 0xf2fc;

while (tail != head) {
    signed char rdx = (signed char)ring[(tail + 1) % 2048];
    signed char rdy = (signed char)ring[(tail + 2) % 2048];

    if (rdx != 0 || rdy != 0) {
        miPointerDeltaCursor(screenInfo.screens[0], (int)rdx, (int)rdy, TRUE);
    }
    tail = (tail + 3) % 2048;
}
mvme393_dpr[0xf2f6 / 2] = tail;
```
