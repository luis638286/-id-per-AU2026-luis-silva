#include "main.h"
#include "led_test.h"

#define LED0_PIN   GPIO_PIN_5    // PA5, D13
#define LED1_PIN   GPIO_PIN_6    // PA6, D12

#define BTN0_PORT  GPIOB         // PB0, A3, pull-up,   pressed = LOW
#define BTN0_PIN   GPIO_PIN_0
#define BTN1_PORT  GPIOC         // PC1, A4, pull-down, pressed = HIGH
#define BTN1_PIN   GPIO_PIN_1

#define DEBOUNCE_SAMPLES  4      // 4 x 5 ms = 20 ms of stability required
#define LOOP_TICK_MS      5

static void gpio_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};

    // LEDs as push-pull outputs
    g.Pin   = LED0_PIN | LED1_PIN;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &g);

    // B0 as input with internal pull-up
    g.Pin  = BTN0_PIN;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(BTN0_PORT, &g);

    // B1 as input with internal pull-down
    g.Pin  = BTN1_PIN;
    g.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(BTN1_PORT, &g);

    HAL_GPIO_WritePin(GPIOA, LED0_PIN | LED1_PIN, GPIO_PIN_RESET);
}

void gpio_poc_run(void)
{
    gpio_init();

    uint8_t b1_state = 0;   // debounced state of B1
    uint8_t b1_count = 0;   // samples seen at the new level

    while (1)
    {
        // B0: level driven. Pressed pulls PB0 to GND, so RESET means pressed.
        if (HAL_GPIO_ReadPin(BTN0_PORT, BTN0_PIN) == GPIO_PIN_RESET)
            HAL_GPIO_WritePin(GPIOA, LED0_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(GPIOA, LED0_PIN, GPIO_PIN_RESET);

        // B1: edge driven. Only a level that held for DEBOUNCE_SAMPLES counts.
        uint8_t raw = HAL_GPIO_ReadPin(BTN1_PORT, BTN1_PIN);

        if (raw != b1_state) {
            if (++b1_count >= DEBOUNCE_SAMPLES) {
                b1_state = raw;
                b1_count = 0;
                if (raw)                                   // press, not release
                    HAL_GPIO_TogglePin(GPIOA, LED1_PIN);
            }
        } else {
            b1_count = 0;
        }

        HAL_Delay(LOOP_TICK_MS);
    }
}