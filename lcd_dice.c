#include <LPC17xx.h>
#include <stdlib.h>

#define RS_CTRL 0x08000000    // P0.27 -> RS
#define EN_CTRL 0x10000000    // P0.28 -> EN
#define DT_CTRL 0x07800000    // P0.23–P0.26 -> Data lines (D4–D7)

// Global variables
unsigned long int temp1 = 0, temp2 = 0, i, j;
unsigned char flag1 = 0, flag2 = 0;
unsigned char msg[] = "Dice: ";

void lcd_write(void);
void port_write(void);
void delay_lcd(unsigned long);
void lcd_init(void);
void display_dice(void);

// LCD initialization commands
unsigned long int init_command[] = {0x30,0x30,0x30,0x20,0x28,0x0C,0x01,0x80};

// -------------------------------------------------------------
// MAIN FUNCTION
// -------------------------------------------------------------
int main(void) {
    // Configure LCD pins as output
    LPC_GPIO0->FIODIR = DT_CTRL | RS_CTRL | EN_CTRL;
    lcd_init();

    // Configure EINT0 (SW2 on P2.10)
    LPC_PINCON->PINSEL4 |= (1 << 20);  // P2.10 as EINT0
    LPC_SC->EXTMODE = 0x1;             // Edge sensitive
    LPC_SC->EXTPOLAR = 0x0;            // Falling edge
    NVIC_EnableIRQ(EINT0_IRQn);        // Enable EINT0 interrupt

    // Display startup message
    flag1 = 1;
    for (i = 0; msg[i] != '\0'; i++) {
        temp1 = msg[i];
        lcd_write();
    }
    temp1 = '0'; 
    lcd_write();

    while (1); // Wait for interrupts
}

// -------------------------------------------------------------
// LCD Initialization
// -------------------------------------------------------------
void lcd_init(void) {
    for (i = 0; i < 8; i++) {
        temp1 = init_command[i];
        flag1 = 0;        // Command mode
        lcd_write();
    }
}

// -------------------------------------------------------------
// External Interrupt Handler (EINT0 -> SW2 Press)
// -------------------------------------------------------------
void EINT0_IRQHandler(void) {
    flag1 = 0; temp1 = 0x01; lcd_write();    // Clear display
    temp1 = 0x80; lcd_write();               // Move to first line
    flag1 = 1;

    for (i = 0; msg[i] != '\0'; i++) {       // Display "Dice: "
        temp1 = msg[i];
        lcd_write();
    }

    j = (rand() % 6) + 1;                    // Generate 1–6
    temp1 = '0' + j;                         // Convert to ASCII
    lcd_write();

    LPC_SC->EXTINT = 1;                      // Clear interrupt flag
}

// -------------------------------------------------------------
// LCD Data Write Function (4-bit Mode)
// -------------------------------------------------------------
void lcd_write(void) {
    flag2 = (flag1 == 1) ? 0 : (((temp1 == 0x30) || (temp1 == 0x20)) ? 1 : 0);

    temp2 = (temp1 & 0xF0) << 19;    // Upper nibble ? P0.23–P0.26
    port_write();
    temp2 = (temp1 & 0x0F) << 23;    // Lower nibble ? P0.23–P0.26
    port_write();
}

// -------------------------------------------------------------
// Port Write to LCD
// -------------------------------------------------------------
void port_write(void) {
    LPC_GPIO0->FIOPIN = temp2;       // Send data
    if (flag1 == 0) 
        LPC_GPIO0->FIOCLR = RS_CTRL; // RS = 0 for command
    else 
        LPC_GPIO0->FIOSET = RS_CTRL; // RS = 1 for data

    LPC_GPIO0->FIOSET = EN_CTRL;     // EN = 1 (High pulse)
    delay_lcd(100);
    LPC_GPIO0->FIOCLR = EN_CTRL;     // EN = 0
    delay_lcd(5000);
}

// -------------------------------------------------------------
// Delay Function
// -------------------------------------------------------------
void delay_lcd(unsigned long r1) {
    unsigned long r;
    for (r = 0; r < r1; r++);
}
