#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "font.h"

// Configurações do WS2812
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define BRIGHTNESS 32

// Configurações do Display OLED
#define I2C_PORT i2c0
#define I2C_SDA 14
#define I2C_SCL 15
#define OLED_ADDR 0x3C

// Configurações dos botões
#define BUTTON_A 5
#define BUTTON_B 6
#define DEBOUNCE_TIME 200000 // 200ms em microssegundos
// LED interno do Pico W
#define LED_PIN 25

// Variáveis globais
static volatile uint8_t medication_count = 0;
static uint32_t last_interrupt_time = 0;
uint32_t led_buffer[NUM_PIXELS] = {0};
ssd1306_t ssd;

// Função para converter RGB para formato GRB
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

// Atualiza o buffer de LEDs com base na contagem de medicamentos
void update_led_buffer() {
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            int idx = y * 5 + (y % 2 == 0 ? x : 4 - x); // Zig-zag
            if (idx < medication_count) {
                led_buffer[idx] = urgb_u32(BRIGHTNESS, BRIGHTNESS, BRIGHTNESS); // Branco
            } else {
                led_buffer[idx] = 0; // Desligado
            }
        }
    }
}

// Envia o buffer para a matriz de LEDs
void set_leds_from_buffer(PIO pio, uint sm) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        pio_sm_put_blocking(pio, sm, led_buffer[i] << 8u);
    }
}

// Handler de interrupção para os botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    if (current_time - last_interrupt_time < DEBOUNCE_TIME) {
        return;
    }
    
    last_interrupt_time = current_time;
    
    if (gpio == BUTTON_A && medication_count > 0) {
        medication_count--;
    } else if (gpio == BUTTON_B && medication_count < 25) {
        medication_count++;
    }
    
    update_led_buffer();
}

// Configuração inicial do display OLED
void setup_display() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_init(&ssd, 128, 64, false, OLED_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

// Atualiza o display com informações atuais
void update_display() {
    char buffer[32];
    ssd1306_fill(&ssd, false);
    
    ssd1306_draw_string(&ssd, "Medicamentos:", 10, 10);
    sprintf(buffer, "Quantidade: %02d", medication_count);
    ssd1306_draw_string(&ssd, buffer, 10, 30);
    
    if (medication_count == 0) {
        ssd1306_draw_string(&ssd, "ALERTA: Reposicao", 10, 50);
    }
    
    ssd1306_send_data(&ssd);
}

int main() {
    stdio_init_all();
    printf("Iniciando SmartMedBox...\n");
    
    // Inicialização do LED interno
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1); // Liga o LED para indicar inicialização
    printf("[OK] LED interno inicializado\n");
    sleep_ms(2000);
    gpio_put(LED_PIN, 0); // Desliga o LED
    
    // Inicialização do WS2812
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    if(offset == 0xFFFFFFFF) {
        printf("[ERRO] Falha ao carregar programa PIO!\n");
        while(1);
    }
    printf("[OK] Programa PIO carregado (offset: %d)\n", offset);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    // Teste direto dos LEDs
    pio_sm_put_blocking(pio, sm, urgb_u32(32,0,0)); // Vermelho
    printf("[TESTE] LEDs devem estar VERMELHOS\n");
    sleep_ms(2000);
    
    // Configuração dos botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_RISE, true);
    
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_RISE, true);
    
    // Configuração das interrupções
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_RISE, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_RISE, true, &gpio_irq_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);
    
    // Inicialização do display
    setup_display();

    ssd1306_draw_string(&ssd, "TESTE OLED", 10, 10);
    ssd1306_send_data(&ssd);
    printf("[TESTE] Display deve mostrar 'TESTE OLED'\n");
    sleep_ms(2000);


    // Teste inicial dos LEDs
    for (int i = 0; i < NUM_PIXELS; i++) {
        led_buffer[i] = urgb_u32(32, 0, 0); // Vermelho
    }
    set_leds_from_buffer(pio, sm);
    sleep_ms(1000); // LEDs devem acender vermelhos
    
    while (true) {
        // Atualiza a matriz de LEDs
        set_leds_from_buffer(pio, sm);
        
        // Atualiza o display a cada 500ms
        static uint32_t last_display_update = 0;
        if (to_ms_since_boot(get_absolute_time()) - last_display_update > 500) {
            update_display();
            last_display_update = to_ms_since_boot(get_absolute_time());
        }

         // Pisca o LED interno para indicar atividade
         gpio_put(LED_PIN, 1);
         sleep_ms(100);
         gpio_put(LED_PIN, 0);
         sleep_ms(100);
    }
    
    return 0;
}