#include "main.h"
#include "button_interrupts.h"

#define LED0_PIN   GPIO_PIN_5    // PA5, D13
#define LED1_PIN   GPIO_PIN_6    // PA6, D12
#define BTN0_PIN   GPIO_PIN_0    // PB0, A3, pull-up,   pressed = LOW
#define BTN1_PIN   GPIO_PIN_1    // PC1, A4, pull-down, pressed = HIGH

static uint32_t b1_last_edge = 0;

extern "C" void EXTI0_IRQHandler(void)      // B0, both edges
{
    EXTI->PR = BTN0_PIN;                    // acknowledge the edge

    if (HAL_GPIO_ReadPin(GPIOB, BTN0_PIN) == GPIO_PIN_RESET)
        HAL_GPIO_WritePin(GPIOA, LED0_PIN, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(GPIOA, LED0_PIN, GPIO_PIN_RESET);
}

extern "C" void EXTI1_IRQHandler(void)      // B1, both edges
{
    EXTI->PR = BTN1_PIN;                    // acknowledge the edge

    uint32_t now   = HAL_GetTick();
    uint32_t quiet = now - b1_last_edge;    // ms since the previous edge
    b1_last_edge   = now;

    if (quiet < 20) return;                 // inside a bounce burst, ignore

    if (HAL_GPIO_ReadPin(GPIOC, BTN1_PIN) == GPIO_PIN_SET)
        HAL_GPIO_TogglePin(GPIOA, LED1_PIN);   // press toggles, release doesn't
}

void button_interrupts_run(void)
{
    GPIO_InitTypeDef g = {0};

    g.Pin   = LED0_PIN | LED1_PIN;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &g);

    g.Pin  = BTN0_PIN;
    g.Mode = GPIO_MODE_IT_RISING_FALLING;
    g.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &g);

    g.Pin  = BTN1_PIN;
    g.Mode = GPIO_MODE_IT_RISING_FALLING;
    g.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &g);

    HAL_GPIO_WritePin(GPIOA, LED0_PIN | LED1_PIN, GPIO_PIN_RESET);

    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);

    while (1) { }
}