#include "msx_fusion.h"
/* Fusion-C 1.2 VpokeFirst also selects V9938 register 14. On a
 * TMS9918 that aliases register 6 and corrupts the sprite table base.
 * This implementation deliberately addresses the MSX1 16 KB VRAM only. */
void VpokeFirst(unsigned int address) {
    OutPort(0x99,(unsigned char)address);
    OutPort(0x99,((address >> 8)&0x3f)|0x40);
}
void Vpoke(unsigned int address,unsigned char data) {
    DisableInterrupt();VpokeFirst(address);OutPort(0x98,data);EnableInterrupt();
}
