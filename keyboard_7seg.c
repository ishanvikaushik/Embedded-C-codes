#include <LPC17xx.h>
#include <stdint.h>

// ==================== CONSTANTS ====================

#define ROW0 (1 << 10)
#define ROW1 (1 << 11)
#define ROW2 (1 << 12)
#define ROW3 (1 << 13)
#define COL0 (1 << 23)
#define COL1 (1 << 24)
#define COL2 (1 << 25)
#define COL3 (1 << 26)
#define COL_MASK (COL0 | COL1 | COL2 | COL3)

// ==================== GLOBALS ====================

unsigned char keypad_map[4][4] = {
    {'0','1','2','3'},
    {'4','5','6','7'},
    {'8','9','A','B'},
    {'C','D','E','F'}
};

// 7-seg patterns (assuming common cathode, a–g mapped to P0.0–P0.6)
uint8_t seven_seg_digits[16] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // B
    0x39, // C
    0x5E, // D
    0x79, // E
    0x71  // F
};

// ==================== FUNCTION DECLARATIONS ====================

void delay_ms(uint32_t);
void keyboard_init(void);
char keyboard_getkey(void);
void sevenseg_init(void);
void sevenseg_display(char key);

// ==================== MAIN ====================

int main(void)
{
    char key;

    SystemInit();
    SystemCoreClockUpdate();

    keyboard_init();
    sevenseg_init();

    while (1)
    {
        key = keyboard_getkey();
        sevenseg_display(key);
    }
}

// ==================== KEYBOARD FUNCTIONS ====================

void keyboard_init(void)
{
    LPC_PINCON->PINSEL3 &= 0xFFC03FFF; // P1.23–P1.26 as GPIO (columns)
    LPC_PINCON->PINSEL4 &= 0xF00FFFFF; // P2.10–P2.13 as GPIO (rows)

    LPC_GPIO2->FIODIR |= (ROW0 | ROW1 | ROW2 | ROW3); // rows as output
    LPC_GPIO1->FIODIR &= ~COL_MASK;                   // columns as input
}

char keyboard_getkey(void)
{
    unsigned long temp3;
    unsigned char row, col;

    while (1)
    {
        for (row = 0; row < 4; row++)
        {
            // Clear all rows, then set one row high
            LPC_GPIO2->FIOCLR = (ROW0 | ROW1 | ROW2 | ROW3);
            LPC_GPIO2->FIOSET = (1 << (10 + row));

            temp3 = LPC_GPIO1->FIOPIN & COL_MASK;

            if (temp3 != 0)
            {
                delay_ms(300); // debounce

                if (temp3 & COL0) col = 0;
                else if (temp3 & COL1) col = 1;
                else if (temp3 & COL2) col = 2;
                else col = 3;

                while ((LPC_GPIO1->FIOPIN & COL_MASK) != 0); // wait for release
                return keypad_map[row][col];
            }
        }
    }
}

// ==================== 7-SEGMENT FUNCTIONS ====================

void sevenseg_init(void)
{
    LPC_PINCON->PINSEL0 &= 0xFFFF0000; // P0.0–P0.7 as GPIO
    LPC_GPIO0->FIODIR |= 0x7F;         // P0.0–P0.6 output
}

void sevenseg_display(char key)
{
    uint8_t index;

    if (key >= '0' && key <= '9') index = key - '0';
    else if (key >= 'A' && key <= 'F') index = 10 + (key - 'A');
    else return;

    LPC_GPIO0->FIOCLR = 0x7F; // clear all segments
    LPC_GPIO0->FIOSET = seven_seg_digits[index];
}

// ==================== DELAY ====================

void delay_ms(uint32_t count)
{
    uint32_t i, j;
    for (i = 0; i < count; i++)
        for (j = 0; j < 5000; j++);
}
