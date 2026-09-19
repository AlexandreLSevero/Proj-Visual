#include "button.h"

#include <string.h>

/* Cores por estado, conforme sugerido no enunciado:
 * azul (neutro), azul claro (hover), azul escuro (clicado). */
static const SDL_Color COLOR_NEUTRO = {40, 90, 200, 255};
static const SDL_Color COLOR_HOVER = {90, 140, 240, 255};
static const SDL_Color COLOR_CLICADO = {20, 50, 130, 255};
static const SDL_Color COLOR_BORDA = {10, 20, 60, 255};
static const SDL_Color COLOR_TEXTO = {255, 255, 255, 255};

void button_init(Button *btn, float x, float y, float w, float h, const char *label) {
    btn->rect.x = x;
    btn->rect.y = y;
    btn->rect.w = w;
    btn->rect.h = h;
    btn->state = BUTTON_STATE_NEUTRO;
    btn->label_texture = NULL;
    btn->label_texture_w = 0;
    btn->label_texture_h = 0;
    btn->label_texture_source[0] = '\0';
    button_set_label(btn, label);
}

void button_set_label(Button *btn, const char *label) {
    strncpy(btn->label, label, sizeof(btn->label) - 1);
    btn->label[sizeof(btn->label) - 1] = '\0';
    /* A textura é recriada de forma preguiçosa em button_render, apenas
     * quando o texto realmente muda (ver comparação com
     * label_texture_source). */
}

int button_contains(const Button *btn, float x, float y) {
    return x >= btn->rect.x && x <= (btn->rect.x + btn->rect.w) && y >= btn->rect.y &&
           y <= (btn->rect.y + btn->rect.h);
}

void button_update_state(Button *btn, float mouse_x, float mouse_y, int mouse_down) {
    int inside = button_contains(btn, mouse_x, mouse_y);

    if (inside && mouse_down) {
        btn->state = BUTTON_STATE_CLICADO;
    } else if (inside) {
        btn->state = BUTTON_STATE_HOVER;
    } else {
        btn->state = BUTTON_STATE_NEUTRO;
    }
}

static SDL_Color button_color_for_state(ButtonState state) {
    switch (state) {
        case BUTTON_STATE_HOVER:
            return COLOR_HOVER;
        case BUTTON_STATE_CLICADO:
            return COLOR_CLICADO;
        case BUTTON_STATE_NEUTRO:
        default:
            return COLOR_NEUTRO;
    }
}

static void button_rebuild_texture_if_needed(Button *btn, SDL_Renderer *renderer,
                                              TTF_Font *font) {
    SDL_Surface *text_surface;

    if (font == NULL) {
        return;
    }

    if (strcmp(btn->label, btn->label_texture_source) == 0 && btn->label_texture != NULL) {
        return; /* texto não mudou: reaproveita a textura já criada */
    }

    if (btn->label_texture != NULL) {
        SDL_DestroyTexture(btn->label_texture);
        btn->label_texture = NULL;
    }

    text_surface = TTF_RenderText_Blended(font, btn->label, 0, COLOR_TEXTO);
    if (text_surface == NULL) {
        SDL_Log("Aviso: falha ao renderizar texto do botão \"%s\": %s", btn->label,
                SDL_GetError());
        return;
    }

    btn->label_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    btn->label_texture_w = text_surface->w;
    btn->label_texture_h = text_surface->h;
    SDL_DestroySurface(text_surface);

    strncpy(btn->label_texture_source, btn->label, sizeof(btn->label_texture_source) - 1);
    btn->label_texture_source[sizeof(btn->label_texture_source) - 1] = '\0';
}

void button_render(Button *btn, SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Color color = button_color_for_state(btn->state);

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &btn->rect);

    SDL_SetRenderDrawColor(renderer, COLOR_BORDA.r, COLOR_BORDA.g, COLOR_BORDA.b, COLOR_BORDA.a);
    SDL_RenderRect(renderer, &btn->rect);

    button_rebuild_texture_if_needed(btn, renderer, font);

    if (btn->label_texture != NULL) {
        SDL_FRect dst;
        dst.w = (float)btn->label_texture_w;
        dst.h = (float)btn->label_texture_h;
        dst.x = btn->rect.x + (btn->rect.w - dst.w) / 2.0f;
        dst.y = btn->rect.y + (btn->rect.h - dst.h) / 2.0f;
        SDL_RenderTexture(renderer, btn->label_texture, NULL, &dst);
    }
}

void button_destroy(Button *btn) {
    if (btn->label_texture != NULL) {
        SDL_DestroyTexture(btn->label_texture);
        btn->label_texture = NULL;
    }
}
