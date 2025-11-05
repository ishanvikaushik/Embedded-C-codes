#include <LPC17xx.h>

#define RS_CTRL  0x08000000  // P0.27
#define EN_CTRL  0x10000000  // P0.28
#define DT_CTRL  0x07800000  // P0.23 to P0.26 data lines

unsigned long int temp1 = 0, temp2 = 0, i, j;
unsigned char flag1 = 0, flag2 = 0;
unsigned char msg[] = {"WELCOME "};

void lcd_write(void);
void port_write(void);
void delay_lcd(unsigned int);

unsigned long int init_command[] = {
  0x30, 0x30, 0x30, 0x20, 0x28, 0x0C, 0x06, 0x01, 0x80
};

int main(void) {
    SystemInit();
    SystemCoreClockUpdate();

    LPC_GPIO0->FIODIR = DT_CTRL | RS_CTRL | EN_CTRL; // Config LCD pins as output

    flag1 = 0; // Command mode
    for (i = 0; i < 9; i++) {
        temp1 = init_command[i];
        lcd_write(); // Send init commands to LCD
    }

    flag1 = 1; // Data mode
    i = 0;
    while (msg[i] != '\0') {
        temp1 = msg[i];
        lcd_write(); // Send data bytes
        i++;
    }

    while (1); // Infinite loop
}

//--------------------------------------------
// LCD Write Function
//--------------------------------------------
void lcd_write(void) {
    // For initial commands in 8-bit mode, only send higher nibble
    flag2 = (flag1 == 1) ? 0 :
             ((temp1 == 0x30) || (temp1 == 0x20)) ? 1 : 0;

    temp2 = temp1 & 0xF0;      // Send higher nibble
    temp2 = temp2 << 19;       // Align to P0.23–P0.26
    port_write();              // Output the higher nibble

    if (!flag2) {              // Send lower nibble if required
        temp2 = temp1 & 0x0F;
        temp2 = temp2 << 23;
        port_write();          // Output the lower nibble
    }
}

//--------------------------------------------
// Port Write Function
//--------------------------------------------
void port_write(void) {
    LPC_GPIO0->FIOPIN = temp2; // Send data to port

    if (flag1 == 0)
        LPC_GPIO0->FIOCLR = RS_CTRL; // Select command register
    else
        LPC_GPIO0->FIOSET = RS_CTRL; // Select data register

    LPC_GPIO0->FIOSET = EN_CTRL;     // Enable pulse
    delay_lcd(25);
    LPC_GPIO0->FIOCLR = EN_CTRL;
    delay_lcd(30000);
}

//--------------------------------------------
// Delay Function
//--------------------------------------------
void delay_lcd(unsigned int r1) {
    unsigned int r;
    for (r = 0; r < r1; r++);
}
