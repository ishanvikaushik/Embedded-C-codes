#include <LPC17xx.h>
#include <stdlib.h>

#define RS_CTRL (1 << 27)     // RS -> P0.27
#define EN_CTRL (1 << 28)     // EN -> P0.28
#define DT_CTRL (0xF << 23)   // Data lines -> P0.23–P0.26 (4-bit mode)

unsigned long int temp1 = 0, temp2 = 0, i, j;
unsigned char flag1 = 0, flag2 = 0;
unsigned char msg[] = "Dice: ";

void lcd_write(void);
void port_write(void);
void delay_lcd(unsigned long);

unsigned long int init_command[] = {
    0x30, 0x30, 0x30, 0x20, 0x28, 0x0C, 0x01, 0x80
};

int main(void) {
    // Configure pins P0.23–P0.28 as output
    LPC_GPIO0->FIODIR = DT_CTRL | RS_CTRL | EN_CTRL;

    // LCD initialization commands
    for (i = 0; i < 8; i++) {
        temp1 = init_command[i];
        flag1 = 0;           // Command mode
        lcd_write();
    }

    // Configure P0.21 as input (button)
    LPC_PINCON->PINSEL3 = 0;
    LPC_GPIO0->FIODIR &= ~(1 << 21);

    while (1) {
        // Polling: wait for button press (active low)
        while ((LPC_GPIO0->FIOPIN & (1 << 21)) != 0);

        // Clear display and set cursor to start
        flag1 = 0; temp1 = 0x01; lcd_write();
        temp1 = 0x80; lcd_write();

        // Display "Dice: "
        flag1 = 1;
        for (i = 0; msg[i] != '\0'; i++) {
            temp1 = msg[i];
            lcd_write();
        }

        // Generate random number 1–6 and display
        j = (rand() % 6) + 1;
        temp1 = '0' + j;
        lcd_write();

        // Wait for button release
        while ((LPC_GPIO0->FIOPIN & (1 << 21)) == 0);
    }
}

void lcd_write(void) {
    // Determine if upper/lower nibble should be sent
    flag2 = (flag1 == 1) ? 0 : (((temp1 == 0x30) || (temp1 == 0x20)) ? 1 : 0);

    // Send upper nibble
    temp2 = (temp1 & 0xF0) << 19;
    port_write();

    // Send lower nibble only for data or full commands
    if (!flag2) {
        temp2 = (temp1 & 0x0F) << 23;
        port_write();
    }
}

void port_write(void) {
    LPC_GPIO0->FIOPIN = temp2;

    if (flag1 == 0)
        LPC_GPIO0->FIOCLR = RS_CTRL;  // Command
    else
        LPC_GPIO0->FIOSET = RS_CTRL;  // Data

    // Pulse enable
    LPC_GPIO0->FIOSET = EN_CTRL;
    delay_lcd(100);
    LPC_GPIO0->FIOCLR = EN_CTRL;

    delay_lcd(500000);
}

void delay_lcd(unsigned long r1) {
    unsigned long r;
    for (r = 0; r < r1; r++);
}
