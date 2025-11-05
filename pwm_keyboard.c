#include <LPC17xx.h>

// Global variable to store PWM value
unsigned long pwm_value = 0;

// Function to initialize PWM (Pulse Width Modulation)
void PWM_Init(void){
    LPC_PINCON->PINSEL3 |= 0x02 << 8;   // P1.20 -> PWM1.2
    LPC_SC->PCONP |= 1 << 6;            // Power up PWM1
    LPC_PWM1->PR = 0;                   // No prescale
    LPC_PWM1->CTCR = 0;                 // Timer mode
    LPC_PWM1->MR0 = 1000;               // PWM period
    LPC_PWM1->MR2 = 500;                // Initial duty = 50%
    LPC_PWM1->LER = 0x05;               // Enable MR0, MR2 latch
    LPC_PWM1->PCR = 1 << 10;            // Enable PWM1.2 output
    LPC_PWM1->TCR = 0x09;               // Enable counter and PWM mode
    NVIC_EnableIRQ(PWM1_IRQn);          // Enable PWM1 interrupt
}

// Function to initialize keyboard GPIO pins
void Keyboard_Init(void){
    LPC_PINCON->PINSEL0 &= ~0x0FFF;     // P0.0–P0.3 as GPIO
    LPC_GPIO0->FIODIR &= ~0x0F;         // Input
}

// Simple delay for debouncing
void Delay(unsigned long count){
    unsigned long i;                    // ? Declare here
    for(i = 0; i < count; i++);
}

// Function to read keyboard input with debouncing
unsigned char Read_Keyboard(void){
    unsigned char i;   // ? Declare outside the for loop
    for(i = 0; i < 4; i++){
        if(!(LPC_GPIO0->FIOPIN & (1 << i))){
            Delay(500000);
            if(!(LPC_GPIO0->FIOPIN & (1 << i))){
                while(!(LPC_GPIO0->FIOPIN & (1 << i)));
                Delay(500000);
                return i;
            }
        }
    }
    return 0xFF;
}

// Main program
int main(void){
    unsigned char key;
    const unsigned long pwm_values[] = {100, 250, 500, 750};

    PWM_Init();
    Keyboard_Init();

    while(1){
        key = Read_Keyboard();
        if(key < 4){
            LPC_PWM1->MR2 = pwm_values[key];
            LPC_PWM1->LER |= 0x04;
        }
    }
    return 0;
}

// PWM interrupt handler
void PWM1_IRQHandler(void){
    LPC_PWM1->IR |= 0x01; // Clear MR0 interrupt flag
}
