#include "text.h"

#include <stdio.h>
#include <string.h>

#include <SDL3/SDL.h>

static TTF_Font *try_open_font_at(const char *base_path, const char *relative) {
    char full_path[1024];
    TTF_Font *font;

    if (base_path == NULL) {
        return NULL;
    }

    SDL_snprintf(full_path, sizeof(full_path), "%s%s", base_path, relative);

    font = TTF_OpenFont(full_path, APP_FONT_POINT_SIZE);
    if (font != NULL) {
        printf("Fonte carregada: %s\n", full_path);
    }
    return font;
}

TTF_Font *text_load_app_font(void) {
    const char *base_path;
    TTF_Font *font;

    if (!TTF_Init()) {
        fprintf(stderr, "Erro ao inicializar SDL_ttf: %s\n", SDL_GetError());
        return NULL;
    }

    /* SDL_GetBasePath() devolve o diretório do executável com a barra
     * apropriada ao sistema operacional já incluída no final, então o
     * caminho da fonte nunca depende de regras fixas de uma plataforma
     * específica (requisito do Item 8). */
    base_path = SDL_GetBasePath();

    /* Layout esperado quando o executável já está instalado ao lado da
     * pasta assets/ (uso normal). */
    font = try_open_font_at(base_path, "assets/fonts/" APP_FONT_FILENAME);

    /* Layout alternativo comum durante o desenvolvimento, quando o
     * binário é gerado dentro de build/ e assets/ fica um nível acima. */
    if (font == NULL) {
        font = try_open_font_at(base_path, "../assets/fonts/" APP_FONT_FILENAME);
    }

    if (font == NULL) {
        fprintf(stderr,
                "Erro: não foi possível carregar a fonte \"%s\" (procurada a partir de \"%s\"). "
                "Verifique se a pasta assets/fonts/ está ao lado do executável. Detalhe: %s\n",
                APP_FONT_FILENAME, base_path != NULL ? base_path : "(desconhecido)",
                SDL_GetError());
    }

    return font;
}

void text_quit(void) {
    TTF_Quit();
}
