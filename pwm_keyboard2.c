#include <LPC17xx.h>

void delay_ms(unsigned int ms);
void pwm_init(void);
void set_intensity(unsigned int percent);
unsigned char read_key(void);

int main(void)
{
    SystemInit();
    SystemCoreClockUpdate();
    pwm_init();  // Initialize PWM on P1.23

    // Configure keyboard (Row 0–3 -> P0.10–P0.13, Col 0–3 -> P0.4–P0.7)
    LPC_PINCON->PINSEL0 &= 0xFF0000FF;   // P0.4–P0.7, P0.10–P0.13 as GPIO
    LPC_GPIO0->FIODIR |=  (0x00003C00);  // Rows (P0.10–P0.13) as output
    LPC_GPIO0->FIODIR &= ~(0x000000F0);  // Columns (P0.4–P0.7) as input

    while (1)
    {
        unsigned char key = read_key();

        if (key == 0) set_intensity(10);  // Key 0 pressed
        else if (key == 1) set_intensity(25);  // Key 1 pressed
        else if (key == 2) set_intensity(50);  // Key 2 pressed
        else if (key == 3) set_intensity(75);  // Key 3 pressed
    }
}

//---------------- PWM Initialization ----------------
void pwm_init(void)
{
    LPC_PINCON->PINSEL3 |= (2 << 14); // Select P1.23 as PWM1.4
	  LPC_SC->PCONP |= 1 << 6;            // Power up PWM1
    LPC_PWM1->PR = 0x00;              // No prescaler
	  LPC_PWM1->CTCR = 0;                 // Timer mode
    LPC_PWM1->MCR = 0x3;         // Reset on MR0 and gen interrupt
    LPC_PWM1->MR0 = 1000;             // Period
    LPC_PWM1->MR4 = 100;              // Default = 10%
    LPC_PWM1->LER = 0xFF;             // Latch enable
    LPC_PWM1->PCR = (1 << 12);        // Enable PWM1.4 output
    LPC_PWM1->TCR = (1 << 1);         // Reset timer
    LPC_PWM1->TCR = 0x09;               // Enable counter and PWM mode
    NVIC_EnableIRQ(PWM1_IRQn);          // Enable PWM1 interrupt
}

//---------------- Set Intensity ----------------
void set_intensity(unsigned int percent)
{
    LPC_PWM1->MR4 = (percent * LPC_PWM1->MR0) / 100; // Adjust duty cycle
    LPC_PWM1->LER = 0x10; // Update MR4
    delay_ms(200); // Debounce
}

//---------------- Read Key Function ----------------
unsigned char read_key(void)
{
    unsigned int temp;
    unsigned char key = 255;

    // Drive only Row 0 (P0.10) HIGH; others LOW
    LPC_GPIO0->FIOCLR = 0x00003C00;
    LPC_GPIO0->FIOSET = (1 << 10);

    temp = LPC_GPIO0->FIOPIN & 0x000000F0; // Read columns P0.4–P0.7

    if (temp != 0)
    {
        if (temp == 0x10) key = 0; // Key '0'
        else if (temp == 0x20) key = 1; // Key '1'
        else if (temp == 0x40) key = 2; // Key '2'
        else if (temp == 0x80) key = 3; // Key '3'
    }
    return key;
}

//---------------- Simple Delay ----------------
void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++)
        for (j = 0; j < 4000; j++);
}

// PWM interrupt handler
void PWM1_IRQHandler(void){
    LPC_PWM1->IR |= 0x01; // Clear MR0 interrupt flag
}
