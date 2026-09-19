/*
 * Funções puras de processamento de imagem (escala de cinza, histograma,
 * estatísticas e equalização). Não dependem da SDL: trabalham apenas com
 * arrays de bytes (Uint8), o que permite testá-las isoladamente (ver
 * tests/test_pixel_ops.c) sem precisar inicializar vídeo/janelas.
 */
#ifndef PROJ_VISUAL_PIXEL_OPS_H
#define PROJ_VISUAL_PIXEL_OPS_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;

// Número de níveis de intensidade (0..255) usados no histograma
#define HISTOGRAM_LEVELS 256

typedef struct {
    int counts[HISTOGRAM_LEVELS]; // contagem de pixels por nível de cinza
    size_t total_pixels;
    double mean;   // média de intensidade (0..255)
    double stddev; // desvio padrão da intensidade
} Histogram;

/*
 * Converte um pixel RGB para escala de cinza usando a fórmula ponderada
 * pedida pelo projeto: Y = 0.2125*R + 0.7154*G + 0.0721*B.
 * O resultado é arredondado e limitado ao intervalo [0, 255].
 */
u8 pixel_rgb_to_gray(u8 r, u8 g, u8 b);

/*
 * Verifica se um pixel RGB corresponde a um tom de cinza "puro"
 * (R == G == B). Usado para detectar se a imagem de entrada já é
 * monocromática antes de decidir se a conversão é necessária.
 */
int pixel_is_gray(u8 r, u8 g, u8 b);

/*
 * Calcula o histograma (contagem por nível de cinza) e as estatísticas
 * (média e desvio padrão) de um buffer de `count` pixels em escala de
 * cinza (um byte por pixel).
 */
void histogram_compute(const u8 *gray_pixels, size_t count, Histogram *out);

/*
 * Classifica o brilho médio da imagem em "clara", "média" ou "escura".
 * Faixas (sobre 0..255): [0,85) escura, [85,170] média, (170,255] clara.
 */
const char *histogram_classify_brightness(double mean);

/*
 * Classifica o contraste (desvio padrão) da imagem em "alto", "médio" ou
 * "baixo". Faixas escolhidas pelo grupo, sobre o desvio padrão máximo
 * teórico de uma imagem 0..255 (~127.5): [0,42) baixo, [42,85] médio,
 * (85,255] alto.
 */
const char *histogram_classify_contrast(double stddev);

/*
 * Aplica equalização de histograma clássica (via CDF) sobre `src`
 * (buffer de `count` pixels em escala de cinza) e escreve o resultado em
 * `dst` (buffer do mesmo tamanho, já alocado pelo chamador). `src` e
 * `dst` podem ser o mesmo ponteiro? Não: para evitar corromper a leitura
 * durante a escrita, `dst` deve ser um buffer distinto de `src`.
 */
void histogram_equalize(const u8 *src, u8 *dst, size_t count);

#endif
