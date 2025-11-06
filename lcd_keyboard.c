#include <LPC17xx.h>

// ================= LCD PIN DEFINITIONS =================
#define RS_CTRL (1 << 27)      // P0.27 RS
#define EN_CTRL (1 << 28)      // P0.28 EN
#define DT_CTRL (0xF << 23)    // P0.23–P0.26 Data lines

// ================= KEYPAD PIN DEFINITIONS =================
#define ROW0 (1 << 10)  // P2.10
#define ROW1 (1 << 11)  // P2.11
#define ROW2 (1 << 12)  // P2.12
#define ROW3 (1 << 13)  // P2.13
#define ROW_PINS (ROW0 | ROW1 | ROW2 | ROW3)

#define COL0 (1 << 23)  // P1.23
#define COL1 (1 << 24)  // P1.24
#define COL2 (1 << 25)  // P1.25
#define COL3 (1 << 26)  // P1.26
#define COL_MASK (COL0 | COL1 | COL2 | COL3)

// ================= GLOBALS =================
unsigned long int temp1, temp2;
unsigned char flag1 = 0, flag2 = 0;
unsigned int i;

// ================= FUNCTION DECLARATIONS =================
void delay_lcd(unsigned int);
void lcd_write(void);
void port_write(void);
void lcd_cmd(unsigned char);
void lcd_data(unsigned char);
void lcd_init(void);
void lcd_puts(unsigned char *);
void lcd_goto_xy(unsigned int, unsigned int);

void keyboard_init(void);
char keyboard_getkey(void);

// ================= KEYPAD CHARACTER MAP =================
unsigned char keypad_map[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

// ================= MAIN =================
int main(void)
{
    unsigned char key_i, key_j;
    unsigned int row_i, col_j;

    SystemInit();
    SystemCoreClockUpdate();

    // Set LCD pins as output
    LPC_GPIO0->FIODIR |= RS_CTRL | EN_CTRL | DT_CTRL;

    lcd_init();
    keyboard_init();

    lcd_cmd(0x01);
    lcd_puts((unsigned char *)"Select Row (1-4):");

    // Get row number
    while (1) {
        key_i = keyboard_getkey();
        if (key_i >= '1' && key_i <= '4') {
            row_i = key_i - '0';
            break;
        }
    }

    lcd_cmd(0x01);
    lcd_puts((unsigned char *)"Select Col (1-4):");

    // Get column number
    while (1) {
        key_j = keyboard_getkey();
        if (key_j >= '1' && key_j <= '4') {
            col_j = key_j - '0';
            break;
        }
    }

    // Display text at chosen position
    lcd_cmd(0x01);
    lcd_goto_xy(row_i, col_j);
    lcd_puts((unsigned char *)"ESD LAB EXAM");

    while (1);
}

// ================= LCD FUNCTIONS =================

void lcd_init(void)
{
    // New initialization command sequence (8-bit mode to 4-bit)
    unsigned char cmds[] = {0x30, 0x30, 0x30, 0x20, 0x0C, 0x06, 0x01, 0x80};

    for (i = 0; i < 8; i++) {
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
    // Send upper nibble
    temp2 = (temp1 & 0xF0) << 19; // shift upper nibble to P0.23–26
    port_write();

    // Send lower nibble
    temp2 = (temp1 & 0x0F) << 23; // shift lower nibble to P0.23–26
    port_write();
}

void port_write(void)
{
    LPC_GPIO0->FIOCLR = DT_CTRL;   // clear data lines
    LPC_GPIO0->FIOSET = temp2;     // send data

    if (flag1 == 0)
        LPC_GPIO0->FIOCLR = RS_CTRL; // command mode
    else
        LPC_GPIO0->FIOSET = RS_CTRL; // data mode

    LPC_GPIO0->FIOSET = EN_CTRL;
    delay_lcd(100);
    LPC_GPIO0->FIOCLR = EN_CTRL;
    delay_lcd(100);
}

void lcd_puts(unsigned char *msg)
{
    while (*msg) {
        lcd_data(*msg);
        msg++;
    }
}

void lcd_goto_xy(unsigned int row, unsigned int col)
{
    unsigned char addr;
    switch (row) {
        case 1: addr = 0x80 + (col - 1); break;
        case 2: addr = 0xC0 + (col - 1); break;
        case 3: addr = 0x94 + (col - 1); break;
        case 4: addr = 0xD4 + (col - 1); break;
        default: addr = 0x80; break;
    }
    lcd_cmd(addr);
}

void delay_lcd(unsigned int r)
{
    unsigned int t;
    for (t = 0; t < r * 100; t++);
}

// ================= KEYBOARD FUNCTIONS =================

void keyboard_init(void)
{
    LPC_PINCON->PINSEL3 &= 0xFFC03FFF;  // P1.23–P1.26 as GPIO
    LPC_PINCON->PINSEL4 &= 0xF00FFFFF;  // P2.10–P2.13 as GPIO
    LPC_GPIO2->FIODIR |= ROW_PINS;      // Rows output
    LPC_GPIO1->FIODIR &= ~COL_MASK;     // Cols input
}

char keyboard_getkey(void)
{
    unsigned long col_val;
    int row, col;

    while (1) {
        for (row = 0; row < 4; row++) {
            LPC_GPIO2->FIOCLR = ROW_PINS;              // clear all rows
            LPC_GPIO2->FIOSET = (1 << (10 + row));     // activate one row
            col_val = LPC_GPIO1->FIOPIN & COL_MASK;    // read columns

            if (col_val) {
                delay_lcd(500); // debounce delay

                if (col_val & COL0) col = 0;
                else if (col_val & COL1) col = 1;
                else if (col_val & COL2) col = 2;
                else if (col_val & COL3) col = 3;
                else continue;

                // wait for key release
                while ((LPC_GPIO1->FIOPIN & COL_MASK) != 0);

                return keypad_map[row][col];
            }
        }
    }
}
