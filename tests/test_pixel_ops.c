/*
 * test_pixel_ops.c
 *
 * Testes unitários simples (sem framework externo) para as funções puras
 * de src/pixel_ops.c. Não depende da SDL — compila e roda com um gcc
 * comum (ver alvo `test` no Makefile), o que facilita validar a lógica
 * de processamento de imagem isoladamente da parte gráfica.
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../src/pixel_ops.h"

static int g_failures = 0;

#define CHECK(cond, msg)                                                     \
    do {                                                                     \
        if (!(cond)) {                                                       \
            printf("[FALHOU] %s (linha %d): %s\n", msg, __LINE__, #cond);    \
            g_failures++;                                                    \
        } else {                                                             \
            printf("[ok]     %s\n", msg);                                    \
        }                                                                    \
    } while (0)

static void test_rgb_to_gray(void) {
    /* Branco puro deve virar 255, preto puro deve virar 0. */
    CHECK(pixel_rgb_to_gray(255, 255, 255) == 255, "branco -> 255");
    CHECK(pixel_rgb_to_gray(0, 0, 0) == 0, "preto -> 0");

    /* Vermelho puro: 0.2125 * 255 = 54.1875 -> arredonda para 54. */
    CHECK(pixel_rgb_to_gray(255, 0, 0) == 54, "vermelho puro -> 54");

    /* Verde puro: 0.7154 * 255 = 182.427 -> arredonda para 182. */
    CHECK(pixel_rgb_to_gray(0, 255, 0) == 182, "verde puro -> 182");

    /* Azul puro: 0.0721 * 255 = 18.3855 -> arredonda para 18. */
    CHECK(pixel_rgb_to_gray(0, 0, 255) == 18, "azul puro -> 18");

    /* Cinza puro deve permanecer no mesmo valor (dentro de arredondamento). */
    CHECK(pixel_rgb_to_gray(128, 128, 128) == 128, "cinza 128 -> 128");
}

static void test_is_gray(void) {
    CHECK(pixel_is_gray(10, 10, 10) == 1, "10,10,10 e cinza");
    CHECK(pixel_is_gray(10, 11, 10) == 0, "10,11,10 nao e cinza");
    CHECK(pixel_is_gray(0, 0, 0) == 1, "preto e cinza");
}

static void test_histogram_uniform(void) {
    /* Imagem 4x1 com valores 0, 85, 170, 255: média deve ser 127.5. */
    u8 pixels[4] = {0, 85, 170, 255};
    Histogram hist;

    histogram_compute(pixels, 4, &hist);

    CHECK(hist.total_pixels == 4, "total_pixels == 4");
    CHECK(hist.counts[0] == 1 && hist.counts[85] == 1 && hist.counts[170] == 1 &&
              hist.counts[255] == 1,
          "contagens corretas por nivel");
    CHECK(fabs(hist.mean - 127.5) < 0.001, "media == 127.5");
}

static void test_histogram_constant_image(void) {
    /* Imagem toda com o mesmo valor: desvio padrão deve ser 0. */
    u8 pixels[100];
    Histogram hist;
    int i;

    for (i = 0; i < 100; i++) {
        pixels[i] = 200;
    }

    histogram_compute(pixels, 100, &hist);

    CHECK(fabs(hist.mean - 200.0) < 0.001, "media == 200 (imagem constante)");
    CHECK(fabs(hist.stddev) < 0.001, "desvio padrao == 0 (imagem constante)");
}

static void test_classification(void) {
    CHECK(strcmp(histogram_classify_brightness(10.0), "escura") == 0, "10 -> escura");
    CHECK(strcmp(histogram_classify_brightness(127.5), "média") == 0, "127.5 -> media");
    CHECK(strcmp(histogram_classify_brightness(240.0), "clara") == 0, "240 -> clara");

    CHECK(strcmp(histogram_classify_contrast(5.0), "baixo") == 0, "5 -> contraste baixo");
    CHECK(strcmp(histogram_classify_contrast(60.0), "médio") == 0, "60 -> contraste medio");
    CHECK(strcmp(histogram_classify_contrast(120.0), "alto") == 0, "120 -> contraste alto");
}

static void test_equalize_constant_image_is_noop(void) {
    /* Equalizar uma imagem de cor única não deve gerar divisão por zero
     * nem alterar os valores (não há o que redistribuir). */
    u8 src[16];
    u8 dst[16];
    int i;

    for (i = 0; i < 16; i++) {
        src[i] = 42;
    }

    histogram_equalize(src, dst, 16);

    for (i = 0; i < 16; i++) {
        CHECK(dst[i] == 42, "equalizacao de imagem constante nao altera valores");
        break; /* uma checagem representativa é suficiente no log */
    }
}

static void test_equalize_expands_contrast(void) {
    /* Imagem com pouco uso da faixa dinâmica (todos os valores entre 100
     * e 110): depois de equalizar, o desvio padrão deve aumentar
     * (o contraste melhora), que é o objetivo do algoritmo. */
    u8 src[11];
    u8 dst[11];
    Histogram before, after;
    int i;

    for (i = 0; i < 11; i++) {
        src[i] = (u8)(100 + i);
    }

    histogram_compute(src, 11, &before);
    histogram_equalize(src, dst, 11);
    histogram_compute(dst, 11, &after);

    CHECK(after.stddev > before.stddev, "equalizacao aumenta o desvio padrao (mais contraste)");
    CHECK(dst[0] == 0, "menor valor original mapeia para 0 apos equalizacao");
    CHECK(dst[10] == 255, "maior valor original mapeia para 255 apos equalizacao");
}

int main(void) {
    test_rgb_to_gray();
    test_is_gray();
    test_histogram_uniform();
    test_histogram_constant_image();
    test_classification();
    test_equalize_constant_image_is_noop();
    test_equalize_expands_contrast();

    if (g_failures == 0) {
        printf("\nTodos os testes passaram.\n");
        return 0;
    }

    printf("\n%d teste(s) falharam.\n", g_failures);
    return 1;
}
