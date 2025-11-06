#include <LPC17xx.h>
#include <stdint.h>

// ==================== CONSTANTS ====================
#define PWM_PIN (1 << 23)  // P2.23 ? PWM1.4
#define ROW0 (1 << 10)     // P2.10
#define COL0 (1 << 23)
#define COL1 (1 << 24)
#define COL2 (1 << 25)
#define COL3 (1 << 26)
#define COL_MASK (COL0 | COL1 | COL2 | COL3)

// ==================== GLOBAL VARIABLES ====================
volatile uint32_t duty_percent = 10;  // default brightness = 10%
uint32_t new_val=0;
unsigned char keypad_map[4][4] = {
    {'0','1','2','3'},
    {'4','5','6','7'},
    {'8','9','A','B'},
    {'C','D','E','F'}
};

// ==================== FUNCTION DECLARATIONS ====================
void delay_ms(uint32_t);
void keyboard_init(void);
char keyboard_getkey(void);
void pwm_init(void);
void pwm_set_duty(uint8_t percent);
void PWM1_IRQHandler(void);

// ==================== MAIN ====================
int main(void)
{
    char key;

    SystemInit();
    SystemCoreClockUpdate();

    keyboard_init();
    pwm_init();

    while (1)
    {
        key = keyboard_getkey();

        switch (key)
        {
            case '0': duty_percent = 10; break;
            case '1': duty_percent = 25; break;
            case '2': duty_percent = 50; break;
            case '3': duty_percent = 75; break;
            default: continue; // ignore other keys
        }

        pwm_set_duty(duty_percent);
    }
}

// ==================== PWM FUNCTIONS ====================

void pwm_init(void)
{
    // Configure P1.23 as PWM1.4 (Note: PINSEL3 bits 15:14 = 10)
    LPC_PINCON->PINSEL3 |= (2 << 14);     // Select PWM1.4
    LPC_SC->PCONP |= (1 << 6);            // Power up PWM1

    LPC_PWM1->PR = 0x00;                  // No prescaler
    LPC_PWM1->CTCR = 0x00;                // Timer mode

    LPC_PWM1->MCR = 0x03;                 // Interrupt + Reset on MR0
    LPC_PWM1->MR0 = 1000;                 // PWM period
    LPC_PWM1->MR4 = 100;                  // Default duty = 10%
    LPC_PWM1->LER = 0xFF;                 // Enable shadow latch
    LPC_PWM1->PCR = (1 << 12);            // Enable PWM1.4 output
    LPC_PWM1->TCR = (1 << 1);             // Reset counter
    LPC_PWM1->TCR = 0x09;                 // Enable counter + PWM mode

    NVIC_EnableIRQ(PWM1_IRQn);            // Enable PWM interrupt
}

void pwm_set_duty(uint8_t percent)
{
    if (percent > 100) percent = 100;
     new_val = (1000 * percent) / 100; // since MR0=1000
    LPC_PWM1->MR4 = new_val;
    LPC_PWM1->LER = (1 << 4); // Latch MR4 update
    delay_ms(100);
}

// ==================== PWM IRQ HANDLER ====================
void PWM1_IRQHandler(void)
{
    // Clear interrupt flag for MR0
    LPC_PWM1->IR = (1 << 0);
    // (optional) Can also adjust MR4 here dynamically if you want
}

// ==================== KEYBOARD FUNCTIONS ====================

void keyboard_init(void)
{
    LPC_PINCON->PINSEL3 &= 0xFFC03FFF;  // P1.23–P1.26 as GPIO
    LPC_PINCON->PINSEL4 &= 0xF00FFFFF;  // P2.10–P2.13 as GPIO

    LPC_GPIO2->FIODIR |= ROW0;          // Only Row0 used for this task
    LPC_GPIO1->FIODIR &= ~COL_MASK;     // Columns as input
}

char keyboard_getkey(void)
{
    unsigned long col_val;
    int col;

    while (1)
    {
        LPC_GPIO2->FIOCLR = ROW0;       // Clear row
        LPC_GPIO2->FIOSET = ROW0;       // Drive row0 high

        col_val = LPC_GPIO1->FIOPIN & COL_MASK;
        if (col_val)
        {
            delay_ms(500); // debounce

            if (col_val & COL0) col = 0;
            else if (col_val & COL1) col = 1;
            else if (col_val & COL2) col = 2;
            else if (col_val & COL3) col = 3;
            else continue;

            while ((LPC_GPIO1->FIOPIN & COL_MASK) != 0); // wait for release
            return keypad_map[0][col];
        }
    }
}

// ==================== DELAY FUNCTION ====================
void delay_ms(uint32_t count)
{
    uint32_t i, j;
    for (i = 0; i < count; i++)
        for (j = 0; j < 5000; j++);
}
