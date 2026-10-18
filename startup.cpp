#include <cstdint>

extern "C" {

    extern uint32_t _estack;
    extern uint32_t _sdata, _edata, _sidata;
    extern uint32_t _sbss, _ebss;

    int _close(int file) { return -1; }
    int _lseek(int file, int ptr, int dir) { return 0; }
    int _read(int file, char *ptr, int len) { return 0; }
    int _write(int file, char *ptr, int len) { return len; }

    int main();

    void Reset_Handler() {
        // Copy .data section from FLASH to SRAM
        uint32_t *pSrc = &_sidata;
        uint32_t *pDest = &_sdata;
        while (pDest < &_edata) {
            *pDest++ = *pSrc++;
        }

        // Zero fill .bss section in SRAM
        pDest = &_sbss;
        while (pDest < &_ebss) {
            *pDest++ = 0;
        }

        // Call main
        main();

        while (true);
    }

    // Default handler for unused interrupts
    void Default_Handler() {
        while (true);
    }

}

// Unused interrupt vectors in this minimal table route to the default handler.
[[gnu::section(".isr_vector")]]
void (* const g_pfnVectors[23])(void) = {
    (void (*)(void))(&_estack), // 0: Initial Stack Pointer
    Reset_Handler,              // 1: Reset
    Default_Handler,            // 2: NMI
    Default_Handler,            // 3: HardFault
    Default_Handler,            // 4: MemManage
    Default_Handler,            // 5: BusFault
    Default_Handler,            // 6: UsageFault
    Default_Handler,            // 7-10: Reserved core exceptions
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,            // 11: SVCall
    Default_Handler,            // 12: Debug monitor
    Default_Handler,            // 13: Reserved
    Default_Handler,            // 14: PendSV
    Default_Handler,            // 15: SysTick
    Default_Handler,            // 16: IRQ0
    Default_Handler,            // 17: IRQ1
    Default_Handler,            // 18: IRQ2
    Default_Handler,            // 19: IRQ3
    Default_Handler,            // 20: IRQ4
    Default_Handler,            // 21: IRQ5
    Default_Handler,            // 22: IRQ6 / EXTI0
};
