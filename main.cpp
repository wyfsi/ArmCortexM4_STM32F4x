// DISCOVERY BOARD CODE
#include <cstdint>

// Debug-watch values; each call to rotate_values() moves each value to the next variable.
volatile uint32_t a = 13U;
volatile uint32_t b = 69U;
volatile uint32_t c = 420U;

namespace {

// STM32F4 register addresses: RCC enables peripheral clocks; GPIOD configures and drives the LED.
constexpr uintptr_t RCC_AHB1ENR = 0x40023830U;
constexpr uintptr_t GPIOD_MODER = 0x40020C00U;
constexpr uintptr_t GPIOD_ODR = 0x40020C14U;
// Cortex-M4 SysTick control, reload, and current-value registers.
constexpr uintptr_t SYSTICK_CTRL = 0xE000E010U;
constexpr uintptr_t SYSTICK_LOAD = 0xE000E014U;
constexpr uintptr_t SYSTICK_VAL = 0xE000E018U;

// On the STM32F407G-DISC1, PD12-PD15 drive green LD4, orange LD3, red LD5, and blue LD6.
constexpr uint32_t DISCOVERY_LED_MASK =
    (1U << 12U) | (1U << 13U) | (1U << 14U) | (1U << 15U);
// At the reset-default 16 MHz processor clock, 16000 ticks are approximately 1 ms.
constexpr uint32_t SYSTICK_RELOAD_1MS = 16000U - 1U;
constexpr uint32_t SYSTICK_COUNTFLAG = 1U << 16U;

// Return a volatile reference to a memory-mapped register so reads and writes reach hardware.
// Used by the GPIO and SysTick routines in this file.
inline volatile uint32_t& reg(uintptr_t address) {
    return *reinterpret_cast<volatile uint32_t*>(address);
}

// Enable GPIOD, clear the four Discovery LEDs, and configure PD12-PD15 as outputs.
// Called once at startup so the LEDs begin off before the startup delay.
void initialize_discovery_leds() {
    reg(RCC_AHB1ENR) |= 1U << 3U;
    reg(GPIOD_ODR) &= ~DISCOVERY_LED_MASK;

    for (uint32_t pin = 12U; pin <= 15U; ++pin) {
        const uint32_t mode_shift = pin * 2U;
        reg(GPIOD_MODER) =
            (reg(GPIOD_MODER) & ~(3U << mode_shift)) | (1U << mode_shift);
    }
}

// Start SysTick with the processor clock so it raises COUNTFLAG about once per millisecond.
// Called once at startup; delay_ms() uses this timer to pace the LED heartbeat.
void initialize_systick() {
    reg(SYSTICK_LOAD) = SYSTICK_RELOAD_1MS;
    reg(SYSTICK_VAL) = 0U;
    reg(SYSTICK_CTRL) = (1U << 2U) | (1U << 0U);
}

// Wait for a number of SysTick milliseconds.
// Used between LED changes to create a visible blink without an external timer.
void delay_ms(uint32_t milliseconds) {
    while (milliseconds > 0U) {
        while ((reg(SYSTICK_CTRL) & SYSTICK_COUNTFLAG) == 0U) {
        }
        --milliseconds;
    }
}

} // namespace

// Rotate a -> b, b -> c, and c -> a without losing the original value of a.
// Called by main() so the three debug-watch values advance on every heartbeat cycle.
void rotate_values() {
    const uint32_t saved_a = a;
    a = b;
    b = c;
    c = saved_a;
}

// Firmware entry point called by Reset_Handler after startup initialization.
// Turns all four Discovery LEDs on after a second startup delay, then keeps them on.
int main() {
    rotate_values();
    initialize_discovery_leds();
    initialize_systick();

    delay_ms(100U);
    reg(GPIOD_ODR) |= DISCOVERY_LED_MASK;

    while (true) {
        delay_ms(1000U);
        // Toggle the four Discovery LEDs on and off to create a heartbeat.
        reg(GPIOD_ODR) ^= DISCOVERY_LED_MASK;
    }

    // alternative way to manually toggle the LEDs on and off without using the XOR operator:
    // while (true) {
    //     reg(GPIOD_ODR) |= DISCOVERY_LED_MASK;
    //     delay_ms(500U);
    //     reg(GPIOD_ODR) &= ~DISCOVERY_LED_MASK;
    //     delay_ms(500U);
    // }

    // alternative way to manually toggle the LEDs with a button press without using the XOR operator:
    // while (true) {
    //     if (reg(GPIOA_IDR) & (1U << 0U)) { // if the user button is pressed
    //         reg(GPIOD_ODR) |= DISCOVERY_LED_MASK;
    //     } else {
    //         reg(GPIOD_ODR) &= ~DISCOVERY_LED_MASK;
    //     }
    // }
}
