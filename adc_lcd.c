#include <LPC17xx.h>
#include <stdio.h>
#include <stdlib.h>

// ================= LCD PIN DEFINITIONS =================
#define RS_CTRL (1 << 27)      // P0.27 RS
#define EN_CTRL (1 << 28)      // P0.28 EN
#define DT_CTRL (0xF << 23)    // P0.23–P0.26 Data lines

// ================= CONSTANTS =================
#define Ref_Vtg     3.300
#define Full_Scale  0xFFF  // 12-bit ADC

// ================= FUNCTION DECLARATIONS =================
void delay_lcd(unsigned int);
void lcd_write(void);
void port_write(void);
void lcd_cmd(unsigned char);
void lcd_data(unsigned char);
void lcd_init(void);
void lcd_puts(unsigned char *);
void lcd_goto_xy(unsigned int, unsigned int);

void adc_init(void);
uint16_t read_adc(uint8_t channel);

// ================= GLOBALS =================
unsigned long int temp1, temp2;
unsigned char flag1 = 0;
unsigned int i;

// ================= MAIN =================
int main(void)
{
    uint16_t adc4_val, adc5_val, diff_val;
    float in_vtg_diff;
    unsigned char msg1[] = "ADC4-ADC5 Diff:";
    unsigned char buffer[20];

    SystemInit();
    SystemCoreClockUpdate();

    // LCD and ADC setup
    LPC_GPIO0->FIODIR |= RS_CTRL | EN_CTRL | DT_CTRL;  // LCD pins as output
    lcd_init();
    adc_init();

    lcd_cmd(0x01);  // clear LCD
    lcd_puts(msg1);

    while (1)
    {
        // Read ADC channels
        adc4_val = read_adc(4);
        adc5_val = read_adc(5);

        // Find digital difference
        diff_val = abs(adc4_val - adc5_val);

        // Convert to voltage difference
        in_vtg_diff = ((float)diff_val * Ref_Vtg) / Full_Scale;

        // Move to next line
        lcd_cmd(0xC0);

        // Display values
        sprintf(buffer, "Diff:%04d (%.2fV)", diff_val, in_vtg_diff);
        lcd_puts((unsigned char *)buffer);

        delay_lcd(5000);
    }
}

// ================= LCD FUNCTIONS =================

void lcd_init(void)
{
    unsigned char cmds[] = {0x33, 0x32, 0x28, 0x0C, 0x01, 0x80};
    for (i = 0; i < 6; i++) {
        lcd_cmd(cmds[i]);
        delay_lcd(3000);
    }
}

void lcd_cmd(unsigned char cmd)
{
    flag1 = 0;
    temp1 = cmd;
    lcd_write();
}

void lcd_data(unsigned char data)
{
    flag1 = 1;
    temp1 = data;
    lcd_write();
}

void lcd_write(void)
{
    // Upper nibble
    temp2 = (temp1 & 0xF0) << 19;
    port_write();

    // Lower nibble
    temp2 = (temp1 & 0x0F) << 23;
    port_write();
}

void port_write(void)
{
    LPC_GPIO0->FIOCLR = DT_CTRL;  // Clear data bits
    LPC_GPIO0->FIOSET = temp2;    // Send bits

    if (flag1 == 0)
        LPC_GPIO0->FIOCLR = RS_CTRL; // Command
    else
        LPC_GPIO0->FIOSET = RS_CTRL; // Data

    LPC_GPIO0->FIOSET = EN_CTRL;
    delay_lcd(100);
    LPC_GPIO0->FIOCLR = EN_CTRL;
    delay_lcd(100);
}

void lcd_puts(unsigned char *msg)
{
    while (*msg)
        lcd_data(*msg++);
}

void lcd_goto_xy(unsigned int row, unsigned int col)
{
    unsigned char addr;
    switch (row) {
        case 1: addr = 0x80 + (col - 1); break;
        case 2: addr = 0xC0 + (col - 1); break;
        default: addr = 0x80; break;
    }
    lcd_cmd(addr);
}

void delay_lcd(unsigned int r)
{
    unsigned int t;
    for (t = 0; t < r * 1000; t++);
}

// ================= ADC FUNCTIONS =================

void adc_init(void)
{
    LPC_PINCON->PINSEL3 |= (3 << 28); // P1.30 as AD0.4
    LPC_PINCON->PINSEL3 |= (3 << 30); // P1.31 as AD0.5
    LPC_SC->PCONP |= (1 << 12);       // Power up ADC block
    LPC_ADC->ADCR = (1 << 21) | (1 << 24); // Enable ADC and operational mode
}

uint16_t read_adc(uint8_t channel)
{
    uint32_t result;
    LPC_ADC->ADCR &= 0xFFFFFF00;     // Clear old channel select
    LPC_ADC->ADCR |= (1 << channel); // Select channel
    LPC_ADC->ADCR |= (1 << 24);      // Start conversion

    // Wait until done
    do {
        result = LPC_ADC->ADGDR;
    } while ((result & (1 << 31)) == 0);

    result = (result >> 4) & 0xFFF; // Extract 12-bit result
    return (uint16_t)result;
}
