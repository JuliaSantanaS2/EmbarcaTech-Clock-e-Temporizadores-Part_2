#include "hardware/timer.h"
#include "pico/stdlib.h"

const uint LED_RED = 13;
const uint LED_YELLOW = 12;
const uint LED_GREEN = 11;
const uint BUTTON = 1;

typedef enum {
    RED,
    YELLOW,
    GREEN,
    FINISHED
} state_leds_t;

state_leds_t state_now = GREEN;  
bool processo_em_andamento = false;  // Controle para garantir que o botão só funcione quando o último LED estiver apagado

// Função de callback chamada para alternar o estado dos LEDs
bool leds_callback(struct repeating_timer *t) {

    switch (state_now) {
        case GREEN:
            gpio_put(LED_GREEN, 0);  
            state_now = YELLOW;
            break;
        case YELLOW:
            gpio_put(LED_YELLOW, 0);  
            state_now = RED;
            break;
        case RED:
            gpio_put(LED_RED, 0);  
            state_now = FINISHED;  // Todos os LEDs apagados, processo concluído
            break;
        case FINISHED:
            gpio_put(LED_RED, 1);
            gpio_put(LED_YELLOW, 1);
            gpio_put(LED_GREEN, 1);
            state_now = GREEN;    
            return false;  
    }

    return true;  
}

// Função para debouncing do botão
bool debounce_botao() {
    static uint32_t tempo_ultimo_clique = 0;
    uint32_t tempo_atual = time_us_32();

    if (tempo_atual - tempo_ultimo_clique > 50000) {  
        tempo_ultimo_clique = tempo_atual;
        return true;  
    }
    return false;  
}

int main() {
    stdio_init_all();

    // Inicialização dos pinos dos LEDs
    gpio_init(LED_RED);
    gpio_init(LED_YELLOW);
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_set_dir(LED_YELLOW, GPIO_OUT);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    // Configuração do pino do botão como entrada com resistor pull-up
    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);  

    // Acende os LEDs inicialmente
    gpio_put(LED_RED, 1);
    gpio_put(LED_YELLOW, 1);
    gpio_put(LED_GREEN, 1);

    struct repeating_timer timer;

    while (1) {
        // Verifica se o botão foi pressionado e o processo não está em andamento
        if (!processo_em_andamento && debounce_botao() && gpio_get(BUTTON) == 0) {
            processo_em_andamento = true;  // Bloqueia novo pressionamento durante o processo

            // Inicia o alarme para alternar os LEDs a cada 3000ms
            add_repeating_timer_ms(3000, leds_callback, NULL, &timer);
            sleep_ms(9000);
            
            processo_em_andamento = false;
        }
    }
}
