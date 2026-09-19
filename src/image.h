/*
 * Carregamento de imagens (via SDL_image) e conversão para escala de
 * cinza (Item 1 e Item 2 do descritivo do projeto). Mantém, além da imagem original
 * carregada, o buffer de intensidades em escala de cinza que serve de
 * base para as demais operações (histograma, equalização).
 */

#ifndef PROJ_VISUAL_IMAGE_H
#define PROJ_VISUAL_IMAGE_H

#include <SDL3/SDL.h>

#include "pixel_ops.h"

typedef struct {
    int width;
    int height;

    int was_color_input; // 1 se a imagem original era colorida, 0 se já era cinza

    u8 *gray_original;   // w*h bytes: escala de cinza obtida a partir do arquivo original
    u8 *gray_equalized;  // w*h bytes: versão equalizada de gray_original (calculada sob demanda)
    int equalized_ready; // 1 quando gray_equalized já foi calculado
} AppImage;

/*
 * Carrega a imagem em `path` usando SDL_image, valida o resultado e
 * preenche `out` com a versão em escala de cinza (convertendo se
 * necessário, conforme a fórmula do enunciado). Imprime no terminal:
 *   - mensagens de erro (arquivo não encontrado / formato inválido);
 *   - se a imagem de entrada é colorida ou já está em escala de cinza.
 *
 * Retorna 1 em caso de sucesso, 0 em caso de erro (nesse caso `out` não
 * deve ser usado nem liberado).
 */
int image_load_as_grayscale(const char *path, AppImage *out);

/*
 * Retorna o buffer de escala de cinza correntemente selecionado:
 * equalizado, se `use_equalized` for verdadeiro (e já tiver sido
 * calculado antes via image_equalize), ou o original caso contrário.
 */
const u8 *image_current_gray(const AppImage *img, int use_equalized);

/* Calcula (uma única vez) a versão equalizada de gray_original. */
void image_equalize(AppImage *img);

/*
 * Cria uma SDL_Surface RGBA32 (R=G=B=intensidade) a partir de um buffer
 * de escala de cinza de tamanho width*height. O chamador é responsável
 * por liberar a surface retornada com SDL_DestroySurface.
 */
SDL_Surface *image_gray_to_surface(const u8 *gray, int width, int height);

/* Libera os buffers internos de `img` (não libera a struct em si). */
void image_free(AppImage *img);

#endif
