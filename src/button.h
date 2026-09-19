#ifndef PROJ_VISUAL_BUTTON_H
#define PROJ_VISUAL_BUTTON_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef enum {
    BUTTON_STATE_NEUTRO = 0,
    BUTTON_STATE_HOVER,
    BUTTON_STATE_CLICADO
} ButtonState;

typedef struct {
    SDL_FRect rect;
    char label[64];
    ButtonState state;

    // Textura de texto cacheada, para não recriar a textura a cada frame
    SDL_Texture *label_texture;
    int label_texture_w;
    int label_texture_h;
    char label_texture_source[64];
} Button;

// Inicializa um botão na posição/tamanho dados, com o texto inicial
void button_init(Button *btn, float x, float y, float w, float h, const char *label);

// Atualiza o texto do botão
void button_set_label(Button *btn, const char *label);

/*
 * Atualiza o estado visual do botão a partir da posição do mouse e se o
 * botão do mouse está pressionado sobre ele. Deve ser chamado a cada
 * evento de mouse relevante (movimento, botão pressionado/solto)
 */
void button_update_state(Button *btn, float mouse_x, float mouse_y, int mouse_down);

// Retorna 1 se o ponto (x, y) está dentro da área do botão
int button_contains(const Button *btn, float x, float y);

/*
 * Desenha o botão no renderer informado, usando primitivas SDL para o
 * retângulo (cor conforme o estado) e SDL_ttf (via `font`) para o texto,
 * centralizado. `font` pode ser NULL, caso em que apenas o retângulo é
 * desenhado (fallback caso a fonte não tenha carregado)
 */
void button_render(Button *btn, SDL_Renderer *renderer, TTF_Font *font);

// Libera a textura de texto cacheada, se houver
void button_destroy(Button *btn);

#endif
