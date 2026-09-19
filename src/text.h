/*
 * Inicialização da SDL_ttf e carregamento da fonte do projeto de forma
 * independente de sistema operacional (Item 8 do enunciado): o caminho
 * da fonte é resolvido a partir do diretório do executável
 * (SDL_GetBasePath), nunca de um caminho absoluto fixo.
 */
#ifndef PROJ_VISUAL_TEXT_H
#define PROJ_VISUAL_TEXT_H

#include <SDL3_ttf/SDL_ttf.h>

// Nome do arquivo de fonte, relativo à pasta assets/fonts/ do projeto
#define APP_FONT_FILENAME "DejaVuSans.ttf"
#define APP_FONT_POINT_SIZE 16.0f

/*
 * Inicializa a SDL_ttf (TTF_Init) e abre a fonte do projeto, procurando
 * em "<diretório do executável>/assets/fonts/DejaVuSans.ttf" e, como
 * alternativa, em "<diretório do executável>/../assets/fonts/..." (útil
 * quando o binário fica em uma subpasta como build/ durante o
 * desenvolvimento). Retorna a fonte aberta, ou NULL em caso de erro
 * (mensagem impressa no terminal).
 */
TTF_Font *text_load_app_font(void);

// Encerra a SDL_ttf. Deve ser chamado uma única vez, ao fim do programa.
void text_quit(void);

#endif
