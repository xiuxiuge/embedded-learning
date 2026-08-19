#include <stdint.h>
/*
    寄存器控制PC13和PA5的GPIO口，PC13为输入，PA5为输出
    实现功能：
    	按住按键灯熄灭，松开灯亮
*/
#define RCC_AHB1ENR   (*(volatile uint32_t*)0x40023830)
#define GPIOA_MODER   (*(volatile uint32_t*)0x40020000)
#define GPIOA_PUPDR   (*(volatile uint32_t*)0x4002000C)
#define GPIOA_IDR     (*(volatile uint32_t*)0x40020010)
#define GPIOA_BSRR    (*(volatile uint32_t*)0x40020018)

#define GPIOC_MODER   (*(volatile uint32_t*)0x40020800)
#define GPIOC_PUPDR   (*(volatile uint32_t*)0x4002080C)
#define GPIOC_IDR     (*(volatile uint32_t*)0x40020810)

void delay(volatile uint32_t count)
{
    while (count--);
}

int main(void)
{

    // GPIOA clock
    RCC_AHB1ENR |= (1<<0);

    // PA5 output
    GPIOA_MODER &= ~(3<<10);
    GPIOA_MODER |= (1<<10);

    // GPIOC clock
    RCC_AHB1ENR |= (1<<2);

    // PC13 input
    GPIOC_MODER &= ~(3<<26);

    // PC13 pull-up
    GPIOC_PUPDR &= ~(3<<26);
    GPIOC_PUPDR |= (1<<26);

    while(1)
    {

        if(GPIOC_IDR & (1<<13))
        {
            // 未按
            GPIOA_BSRR = (1<<5);
        }
        else
        {
            // 按下
            GPIOA_BSRR = (1<<(5+16));
        }
    }
}
