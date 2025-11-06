#include <LPC17xx.h>
#include <stdio.h>
#include <stdlib.h>

#define Ref_Vtg     3.300
#define Full_Scale  0xFFF // 12-bit ADC max value

// ====================== FUNCTION DECLARATIONS ======================
void delay_ms(uint32_t);
void adc_init(void);
uint16_t read_adc(uint8_t channel);
void sevenseg_init(void);
void sevenseg_display(uint16_t value);

// ====================== MAIN ======================
int main(void)
{
    uint16_t adc_val4, adc_val5, diff_val;
    
    SystemInit();
    SystemCoreClockUpdate();

    adc_init();
    sevenseg_init();

    while (1)
    {
        adc_val4 = read_adc(4);   // Read AD0.4
        adc_val5 = read_adc(5);   // Read AD0.5
        diff_val = abs(adc_val4 - adc_val5);  // Get digital difference

        sevenseg_display(diff_val); // Display on 7-seg
        delay_ms(500);
    }
}

// ====================== ADC FUNCTIONS ======================

void adc_init(void)
{
    LPC_PINCON->PINSEL3 |= (3 << 28); // P1.30 as AD0.4
    LPC_PINCON->PINSEL3 |= (3 << 30); // P1.31 as AD0.5
    LPC_SC->PCONP |= (1 << 12);       // Power up ADC block
    LPC_ADC->ADCR = (1 << 21) | (1 << 24); // Enable ADC & start mode config
}

uint16_t read_adc(uint8_t channel)
{
    uint32_t result;
    LPC_ADC->ADCR &= 0xFFFFFF00;        // Clear channel selection bits
    LPC_ADC->ADCR |= (1 << channel);    // Select ADC channel (4 or 5)
    LPC_ADC->ADCR |= (1 << 24);         // Start conversion

    // Wait until DONE bit (bit 31) is set
    do {
        result = LPC_ADC->ADGDR;
    } while ((result & (1 << 31)) == 0);

    // Extract 12-bit result
    result = (result >> 4) & 0xFFF;
    return (uint16_t)result;
}

// ====================== 7-SEGMENT FUNCTIONS ======================

void sevenseg_init(void)
{
    // Data lines P0.4–P0.11
    LPC_PINCON->PINSEL0 &= 0xFF00000F;
    LPC_GPIO0->FIODIR |= (0xFF << 4); // P0.4–P0.11 output

    // Decoder select lines P1.23–P1.26
    LPC_PINCON->PINSEL3 &= 0xF00FFFFF;
    LPC_GPIO1->FIODIR |= (0xF << 23); // P1.23–P1.26 output
}

void sevenseg_display(uint16_t value)
{
    uint8_t digits[4];
    uint16_t temp = value;
    int i;

    // Break the value into 4 decimal digits
    for (i = 0; i < 4; i++) {
        digits[i] = temp % 10;
        temp /= 10;
    }

    // Display each digit via decoder line
    for (i = 0; i < 4; i++) {
        LPC_GPIO1->FIOCLR = (0xF << 23);
        LPC_GPIO1->FIOSET = (i << 23);       // Activate decoder line i

        LPC_GPIO0->FIOCLR = (0xFF << 4);
        LPC_GPIO0->FIOSET = (digits[i] << 4); // Send BCD data

        delay_ms(5); // Short delay for persistence
    }
}

// ====================== DELAY FUNCTION ======================

void delay_ms(uint32_t count)
{
    uint32_t i, j;
    for (i = 0; i < count; i++)
        for (j = 0; j < 5000; j++);
}
