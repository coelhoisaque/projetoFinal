#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "generated/ws2812.pio.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"

// Configurações do sistema
#define LED_PIN     13 // 11 12 e 13
#define NUM_PIXELS  25
#define WS2812_PIN  7
#define BRIGHTNESS  32
#define I2C_PORT    i2c0
#define I2C_SDA     14
#define I2C_SCL     15
#define OLED_ADDR   0x3C
#define BUTTON_A    5
#define BUTTON_B    6
#define DEBOUNCE_MS 250

// Mapeamento físico da matriz 5x5 em zig-zag
const uint8_t LED_MAP[NUM_PIXELS] = {
    0,  1,  2,  3,  4,
    9,  8,  7,  6,  5,
    10, 11, 12, 13, 14,
    19, 18, 17, 16, 15,
    20, 21, 22, 23, 24
};

// Padrões pré-definidos para números (0-24)
const uint8_t num_0[NUM_PIXELS] = {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_1[NUM_PIXELS] = {1,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_2[NUM_PIXELS] = {1,1,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_3[NUM_PIXELS] = {1,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_4[NUM_PIXELS] = {1,1,1,1,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_5[NUM_PIXELS] = {1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_6[NUM_PIXELS] = {1,1,1,1,1, 1,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_7[NUM_PIXELS] = {1,1,1,1,1, 1,1,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_8[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_9[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_10[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_11[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_12[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_13[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_14[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_15[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0, 0,0,0,0,0};
const uint8_t num_16[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,0,0,0,0, 0,0,0,0,0};
const uint8_t num_17[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0, 0,0,0,0,0};
const uint8_t num_18[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0, 0,0,0,0,0};
const uint8_t num_19[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,0, 0,0,0,0,0};
const uint8_t num_20[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0};
const uint8_t num_21[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,0,0,0,0};
const uint8_t num_22[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,0,0,0};
const uint8_t num_23[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,0,0};
const uint8_t num_24[NUM_PIXELS] = {1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 1,1,1,1,1};

// Array de padrões
const uint8_t *number_patterns[] = {
    num_0, num_1, num_2, num_3, num_4,
    num_5, num_6, num_7, num_8, num_9,
    num_10, num_11, num_12, num_13, num_14,
    num_15, num_16, num_17, num_18, num_19,
    num_20, num_21, num_22, num_23, num_24
};

// Variáveis globais
static volatile uint8_t med_count = 0;
static volatile uint32_t last_isr_time = 0;
uint32_t led_buffer[NUM_PIXELS] = {0};
ssd1306_t oled;
PIO ws_pio = pio0;
uint ws_sm = 0;

// Conversão RGB para GRB
static inline uint32_t rgb_to_grb(uint8_t r, uint8_t g, uint8_t b) {
    return (g << 24) | (r << 16) | (b << 8);
}

// Atualiza buffer de LEDs com padrão atual
void update_leds() {
    const uint8_t *current_pattern = number_patterns[med_count];
    for(uint8_t logical_i = 0; logical_i < NUM_PIXELS; logical_i++) {
        uint8_t physical_i = LED_MAP[logical_i];
        led_buffer[physical_i] = current_pattern[logical_i] ? 
            rgb_to_grb(BRIGHTNESS, BRIGHTNESS, BRIGHTNESS) : 0;
    }
}

// Envia dados para os LEDs
void send_leds() {
    for(uint8_t i = 0; i < NUM_PIXELS; i++) {
        pio_sm_put_blocking(ws_pio, ws_sm, led_buffer[i]);
        sleep_us(35);  // Timing crítico para WS2812
    }
}

// Handler de interrupção para botões
void button_isr(uint gpio, uint32_t events) {
    uint32_t now = time_us_32();
    if((now - last_isr_time) < DEBOUNCE_MS * 1000) return;
    
    last_isr_time = now;
    
    if(gpio == BUTTON_A && med_count > 0) med_count--;
    if(gpio == BUTTON_B && med_count < 24) med_count++;  // 25 padrões (0-24)
    
    update_leds();
}

// Inicialização do display OLED
void init_display() {
    i2c_init(I2C_PORT, 400000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_init(&oled, 128, 64, false, OLED_ADDR, I2C_PORT);
    ssd1306_fill(&oled, false);
    ssd1306_draw_string(&oled, "Sistema Pronto", 0, 0);
    ssd1306_send_data(&oled);
}

int main() {
    stdio_init_all();
    
    // LED de debug
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
    
    // Inicializa WS2812
    uint offset = pio_add_program(ws_pio, &ws2812_program);
    ws2812_program_init(ws_pio, ws_sm, offset, WS2812_PIN, 800000, false);
    
    // Configura botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_FALL, true);
    
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);
    
    gpio_set_irq_callback(button_isr);
    irq_set_enabled(IO_IRQ_BANK0, true);
    
    // Inicializa display
    init_display();
    
    // Teste inicial de LEDs
    for(uint8_t i = 0; i < NUM_PIXELS; i++) {
        led_buffer[i] = rgb_to_grb(32, 0, 0);
        send_leds();
        sleep_ms(50);
        led_buffer[i] = 0;
    }
    
    // Teste de display
    ssd1306_fill(&oled, false);
    ssd1306_draw_string(&oled, "Teste OK!", 0, 30);
    ssd1306_send_data(&oled);
    
    gpio_put(LED_PIN, 0);
    printf("=== OPERACIONAL ===\n\n");
    
    while(true) {
        send_leds();
        
        // Atualiza display a cada 500ms
        static uint32_t last_update = 0;
        if(absolute_time_diff_us(last_update, get_absolute_time()) > 500000) {
            char status[32];
            snprintf(status, sizeof(status), "Medicamentos: %02d", med_count + 1);  // Mostra 1-25
            
            ssd1306_fill(&oled, false);
            ssd1306_draw_string(&oled, "SmartMedBox", 0, 0);
            ssd1306_draw_string(&oled, status, 0, 20);
            if(med_count == 0) {
                ssd1306_draw_string(&oled, "REPOR ESTOQUE!", 0, 40);
            }
            ssd1306_send_data(&oled);
            
            last_update = get_absolute_time();
        }
        
        sleep_ms(10);
    }
    
    return 0;
}