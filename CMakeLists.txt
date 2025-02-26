# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

# Configurações padrão do projeto
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define a placa como Raspberry Pi Pico W
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importa o Pico SDK
include(pico_sdk_import.cmake)

# Inicializa o projeto
project(Main C CXX ASM)

# Inicializa o Pico SDK
pico_sdk_init()

# Define o diretório de cabeçalhos (headers)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

# Adiciona o executável principal
add_executable(Main Main.c inc/ssd1306.c)

# Cria o diretório para arquivos gerados
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

# Gera o cabeçalho PIO para o controle dos LEDs WS2812
pico_generate_pio_header(Main ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# Adiciona os arquivos fonte ao projeto
target_sources(Main PRIVATE Main.c inc/ssd1306.c)

# Vincula as bibliotecas necessárias
target_link_libraries(Main PRIVATE 
    pico_stdlib 
    hardware_pio 
    hardware_i2c
    hardware_pwm 
    hardware_rtc
)

pico_add_extra_outputs(Main)