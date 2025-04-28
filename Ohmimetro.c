/*
  Ohmímetro com Reconhecimento Automático do Código de Cores
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28
#define Botao_A 5

int R_conhecido = 9800;
float ADC_VREF = 3.31;
int ADC_RESOLUTION = 4095;

// E24 series (5%)
const float e24_values[] = {1.0, 1.1, 1.2, 1.3, 1.5, 1.6, 1.8, 2.0, 2.2, 2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.3, 4.7, 5.1, 5.6, 6.2, 6.8, 7.5, 8.2, 9.1};
const int e24_size = sizeof(e24_values) / sizeof(e24_values[0]);

// Funções auxiliares
float find_closest_e24(float value) {
    float min_diff = INFINITY;
    float closest = e24_values[0];
    for (int i = 0; i < e24_size; i++) {
        float diff = fabs(e24_values[i] - value);
        if (diff < min_diff) {
            min_diff = diff;
            closest = e24_values[i];
        }
    }
    return closest;
}

const char* digit_to_color(int digit) {
    switch (digit) {
        case 0: return "PRETO"; // Preto
        case 1: return "MARROM"; // Marrom
        case 2: return "VERMELHO"; // Vermelho
        case 3: return "LARANJA"; // Laranja
        case 4: return "AMARELO"; // Amarelo
        case 5: return "VERDE"; // Verde
        case 6: return "AZUL"; // Azul
        case 7: return "VIOLETA"; // Violeta
        case 8: return "CINZA"; // Cinza
        case 9: return "BRANCO"; // Branco
        default: return "???";
    }
}

int main() {
    stdio_init_all();

    // Inicialização do ADC
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(2);

    // Inicialização do display
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_t ssd;
    ssd1306_init(&ssd, 128, 64, false, 0x3C, I2C_PORT);
    ssd1306_config(&ssd);

    while (true) {
        // Leitura do ADC
        float soma = 0.0f;
        for (int i = 0; i < 500; i++) {
            soma += adc_read();
            sleep_ms(1);
        }
        float media = soma / 500.0f;
        float R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);

        // Encontrar valor E24 mais próximo
        float log_rx = log10f(R_x);
        float exp_floor = floor(log_rx);
        float mantissa = R_x / powf(10.0f, exp_floor);
        float e24_mantissa = find_closest_e24(mantissa);
        float R_std = e24_mantissa * powf(10.0f, exp_floor);

        // Determinar cores
        int first_digit = (int)e24_mantissa;
        float second_digit_float = (e24_mantissa - first_digit) * 10.0f;
        int second_digit = round(second_digit_float);
        int multiplier = (int)exp_floor - 1;

        const char *color1 = digit_to_color(first_digit);
        const char *color2 = digit_to_color(second_digit);
        const char *color3 = digit_to_color(multiplier);

        // Atualizar display
        ssd1306_fill(&ssd, false);

        // Quadro de leitura real
        ssd1306_rect(&ssd, 0, 0, 64, 27, true, false);
        ssd1306_draw_string(&ssd, "Real:", 8, 6);
        char str_real[10];
        sprintf(str_real, "%.0fΩ", R_x);
        ssd1306_draw_string(&ssd, str_real, 8, 16);

        // Quadro de valor E24
        ssd1306_rect(&ssd, 0, 64, 64, 27, true, false);
        ssd1306_draw_string(&ssd, "E24:", 73, 6);
        char str_e24[10];
        sprintf(str_e24, "%.0fΩ", R_std);
        ssd1306_draw_string(&ssd, str_e24, 73, 16);

        // Quadro de cores
        ssd1306_rect(&ssd, 27, 0, 128, 37, true, false);
        char band1[16], band2[16], band3[16];
        sprintf(band1, "1a: %s", color1);
        sprintf(band2, "2a: %s", color2);
        sprintf(band3, "Mult: %s", color3);
        ssd1306_draw_string(&ssd, band1, 8, 30);
        ssd1306_draw_string(&ssd, band2, 8, 40);
        ssd1306_draw_string(&ssd, band3, 8, 50);

        ssd1306_send_data(&ssd);
        sleep_ms(500);
    }
}