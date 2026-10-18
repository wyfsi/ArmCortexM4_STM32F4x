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

// On the STM32F407G-DISC1, PD12 drives the green user LED LD4.
constexpr uint32_t GREEN_LED_PIN = 12U;
constexpr uint32_t GREEN_LED_MASK = 1U << GREEN_LED_PIN;
// At the reset-default 16 MHz processor clock, 16000 ticks are approximately 1 ms.
constexpr uint32_t SYSTICK_RELOAD_1MS = 16000U - 1U;
constexpr uint32_t SYSTICK_COUNTFLAG = 1U << 16U;

// Return a volatile reference to a memory-mapped register so reads and writes reach hardware.
// Used by the GPIO and SysTick routines in this file.
inline volatile uint32_t& reg(uintptr_t address) {
    return *reinterpret_cast<volatile uint32_t*>(address);
}

// Enable GPIOD and configure PD12 as an output for green LED LD4.
// Called once at startup before the program changes the LED state.
void initialize_green_led() {
    reg(RCC_AHB1ENR) |= 1U << 3U;

    constexpr uint32_t mode_shift = GREEN_LED_PIN * 2U;
    reg(GPIOD_MODER) =
        (reg(GPIOD_MODER) & ~(3U << mode_shift)) | (1U << mode_shift);
    reg(GPIOD_ODR) &= ~GREEN_LED_MASK;
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
// Sets up the green Discovery LED, then rotates the watch values and blinks the LED repeatedly.
int main() {
    initialize_green_led();
    initialize_systick();

    while (true) {
        rotate_values();
        delay_ms(500U);
        reg(GPIOD_ODR) ^= GREEN_LED_MASK;
    }
}
