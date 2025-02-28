
# SmartMedBox

Sistema de lembrete de medicação com Raspberry Pi Pico, display OLED e alarme sonoro.

## Funcionalidades
<!-- - Alarme programável para horários de medicação (3 horários/dia)
- Notificação visual via LED RGB (WS2812) e display OLED
- Alerta sonoro com buzzer PWM -->
- Confirmação de ingestão via botões físicos
<!-- - Interface em tempo real com RTC simulado -->

## Hardware Necessário
- Raspberry Pi Pico
- Display OLED SSD1306 (I2C)
- LED WS2812 (25 pixels)
- Buzzer passivo
- 2 botões táteis
- Resistores de pull-up (para botões)

## Conexões
| Componente       | Pino Pico |
|------------------|-----------|
| LEDs WS2812       | GP7       |
| Botão A          | GP5       |
| Botão B          | GP6       |


## Dependências
- SDK Raspberry Pi Pico
- Biblioteca SSD1306 (I2C)
- Driver PIO para WS2812
- Fonte de caracteres (incluída)

## Como Executar
1. **Clonar o repositório**:
   ```bash
   git clone https://github.com/coelhoisaque/projetoFinal.git
   ```
2. **Configurar o ambiente Pico SDK**:
   - Certifique-se de que o Raspberry Pi Pico SDK está corretamente instalado e configurado no seu ambiente de desenvolvimento.
3. **Compile e carregue o código no microcontrolador.**:
4. **Conectar a placa BitDogLab** ao seu computador.
5. **Enviar o arquivo `.uf2`** gerado para a placa.

## Estrutura do Projeto

```bash
C:.
│   CMakeLists.txt
│   diagram.json
│   Main.c
│   pico_sdk_import.cmake
│   README.md
│   wokwi.toml
│   ws2812.pio
├───generated
│       ws2812.pio.h
│
└───inc
        font.h
        ssd1306.c
        ssd1306.h
```
---
Licença MIT - Código aberto para modificação e uso comercial
