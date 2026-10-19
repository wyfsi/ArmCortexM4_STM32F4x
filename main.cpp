#include <cstdint>

// Debug-watch values; each call to rotate_values() moves each value to the next variable.
volatile uint32_t a = 13U;
volatile uint32_t b = 69U;
volatile uint32_t c = 420U;

namespace {

// STM32F4 register addresses: RCC enables peripheral clocks; GPIOD configures and drives the LED.
constexpr uintptr_t RCC_AHB1ENR = 0x40023830U;
constexpr uintptr_t GPIOA_MODER = 0x40020000U;
constexpr uintptr_t GPIOA_ODR    = 0x40020014U;
// Cortex-M4 SysTick control, reload, and current-value registers.
constexpr uintptr_t SYSTICK_CTRL = 0xE000E010U;
constexpr uintptr_t SYSTICK_LOAD = 0xE000E014U;
constexpr uintptr_t SYSTICK_VAL = 0xE000E018U;

// For this DevEBox STM32F407VET6 board, PA6 = D2 (use 7U for D3)
constexpr uint32_t DEV_E_BOX_D2_PIN = 6U;
constexpr uint32_t DEV_E_BOX_D3_PIN = 7U;
constexpr uint32_t DEV_E_BOX_D2_MASK = 1U << DEV_E_BOX_D2_PIN;
constexpr uint32_t DEV_E_BOX_D3_MASK = 1U << DEV_E_BOX_D3_PIN;
// At the reset-default 16 MHz processor clock, 16000 ticks are approximately 1 ms.
constexpr uint32_t SYSTICK_RELOAD_1MS = 16000U - 1U;
constexpr uint32_t SYSTICK_COUNTFLAG = 1U << 16U;


// Return a volatile reference to a memory-mapped register so reads and writes reach hardware.
// Used by the GPIO and SysTick routines in this file.
inline volatile uint32_t& reg(uintptr_t address) {
    return *reinterpret_cast<volatile uint32_t*>(address);
}

// Enable GPIOD and configure PA6 as an output for DevEBox LED D2.
void initialize_D2_led() {
    reg(RCC_AHB1ENR) |= (1U << 0U); // Enable GPIOA clock
    reg(GPIOA_ODR) |= DEV_E_BOX_D2_MASK; // Set PA6 high (LED off)
    reg(GPIOA_MODER) &= ~(3U << (DEV_E_BOX_D2_PIN * 2U)); // Clear mode bits for PA6
    reg(GPIOA_MODER) |= (1U << (DEV_E_BOX_D2_PIN * 2U)); // Set PA6 as output
}
void initialize_D3_led() {
    reg(RCC_AHB1ENR) |= (1U << 0U); // Enable GPIOA clock
    reg(GPIOA_ODR) |= DEV_E_BOX_D3_MASK; // Set PA7 high (LED off)
    reg(GPIOA_MODER) &= ~(3U << (DEV_E_BOX_D3_PIN * 2U)); // Clear mode bits for PA7
    reg(GPIOA_MODER) |= (1U << (DEV_E_BOX_D3_PIN * 2U)); // Set PA7 as output
}

// Drive a GPIOD pin high or low without exposing register operations to the caller.
// Used by main() to turn DevEBox D2 on and off using its pin number.
void set_led(bool on, uint32_t pin_mask) {
    if (on) {
        reg(GPIOA_ODR) &= ~pin_mask;         // low = LED on
    } else {
        reg(GPIOA_ODR) |= pin_mask;          // high = LED off
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
// Sets up DevEBox LED D2 on PA6, then rotates the watch values and blinks D2 repeatedly.
int main() {
    initialize_D2_led();
    initialize_D3_led();
    initialize_systick();

    // Test all
    reg(RCC_AHB1ENR) |= (1U << 0U) | (1U << 2U) | (1U << 3U);

    while (true) {
        rotate_values();
        set_led(true, DEV_E_BOX_D2_MASK);
        set_led(true, DEV_E_BOX_D3_MASK);
        delay_ms(1000U);
        set_led(false, DEV_E_BOX_D2_MASK);
        // Not turning on D3 to test all
        delay_ms(1000U);
    }
}
