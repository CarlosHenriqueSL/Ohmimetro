/*
 * Ohmímetro com Reconhecimento Automático do Código de Cores
 *
 * Este programa mede a resistência de um resistor conectado ao canal ADC do
 * Raspberry Pi Pico, calcula o valor padrão mais próximo da série E24 e
 * exibe no display OLED tanto o valor medido quanto o valor padrão, além de
 * mostrar as cores correspondentes às faixas do resistor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Definições de pinos e configurações
#define I2C_PORT    i2c1       
#define I2C_SDA     14         
#define I2C_SCL     15        
#define endereco    0x3C       
#define ADC_PIN     28         

int R_conhecido = 9800;
float ADC_VREF = 3.31;
int ADC_RESOLUTION = 4095;

// Valores padrão da série E24 
const float valores_e24[] = {
    1.0, 1.1, 1.2, 1.3, 1.5, 1.6, 1.8, 2.0,
    2.2, 2.4, 2.7, 3.0, 3.3, 3.6, 3.9, 4.3,
    4.7, 5.1, 5.6, 6.2, 6.8, 7.5, 8.2, 9.1
};
const int tamanho_e24 = sizeof(valores_e24) / sizeof(valores_e24[0]);

/**
 *  Encontra o valor da série E24 mais próximo de um dado número bruto.
 *  Itera sobre o vetor valores_e24 e retorna o valor com diferença mínima.
 */
float find_closest_e24(float value) {
    float min_diff = INFINITY;
    float closest = valores_e24[0];
    for (int i = 0; i < tamanho_e24; i++) {
        float diff = fabs(valores_e24[i] - value);
        if (diff < min_diff) {
            min_diff = diff;
            closest = valores_e24[i];
        }
    }
    return closest;
}

/**
 *  Converte um dígito decimal (0 a 9) para o nome da cor da faixa
 *  de resistor correspondente.
 */
const char* digit_to_color(int digit) {
    switch (digit) {
        case 0: return "PRETO";
        case 1: return "MARROM";
        case 2: return "VERMELHO";
        case 3: return "LARANJA";
        case 4: return "AMARELO";
        case 5: return "VERDE";
        case 6: return "AZUL";
        case 7: return "VIOLETA";
        case 8: return "CINZA";
        case 9: return "BRANCO";
        default: return "???";
    }
}

int main() {
    stdio_init_all(); 

    adc_init();                    
    adc_gpio_init(ADC_PIN);         
    adc_select_input(2);            

    i2c_init(I2C_PORT, 400 * 1000); 
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_t ssd;
    ssd1306_init(&ssd, 128, 64, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);

    while (true) {
        // Leitura do ADC (média de 500 amostras)
        float soma = 0.0f;
        for (int i = 0; i < 500; i++) {
            soma += adc_read();      
            sleep_ms(1);             
        }
        float media = soma / 500.0f;

        // Calcula resistência desconhecida Rx usando divisor de tensão
        // Vout = (Rx / (Rx + R_ref)) * Vref  =>  Rx = R_ref * Vout / (Vref - Vout)
        float R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);

        // Ajuste para valor padrão da série E24
        float log_rx = log10f(R_x);
        float expoente = floor(log_rx);               // Expoente (ordem de grandeza)
        float numero_bruto = R_x / powf(10.0f, expoente);
        // Seleciona número bruto E24 mais próximo
        float e24_numero_bruto = find_closest_e24(numero_bruto);
        float R_std = e24_numero_bruto * powf(10.0f, expoente);

        // Cálculo das faixas de cores
        int first_digit = (int)e24_numero_bruto;                   // Primeiro dígito
        float second_digit_float = (e24_numero_bruto - first_digit) * 10.0f;
        int second_digit = round(second_digit_float);              // Segundo dígito
        int multiplier = (int)expoente - 1;                        // Multiplicador (10^n)

        // Converte dígitos em cores
        const char *color1 = digit_to_color(first_digit);
        const char *color2 = digit_to_color(second_digit);
        const char *color3 = digit_to_color(multiplier);

        ssd1306_fill(&ssd, false); 

        // Valor real medido
        ssd1306_rect(&ssd, 0, 0, 64, 27, true, false);
        ssd1306_draw_string(&ssd, "Real:", 8, 6);
        char str_real[10];
        sprintf(str_real, "%.0fΩ", R_x);
        ssd1306_draw_string(&ssd, str_real, 8, 16);

        // Valor E24 padronizado
        ssd1306_rect(&ssd, 0, 64, 64, 27, true, false);
        ssd1306_draw_string(&ssd, "E24:", 73, 6);
        char str_e24[10];
        sprintf(str_e24, "%.0fΩ", R_std);
        ssd1306_draw_string(&ssd, str_e24, 73, 16);

        // Cores das faixas
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

    return 0;
}
