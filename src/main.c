/*
 * main.c
 *
 * Proj1 - Processamento de imagens (Computação Visual, Mackenzie).
 * Uso: programa caminho_da_imagem.ext
 */
#include <stdio.h>

#include "gui.h"
#include "image.h"

int main(int argc, char *argv[]) {
    AppImage image;
    int result;

    /* Força stdout a ser line-buffered mesmo quando a saída é
     * redirecionada para um arquivo/pipe (por padrão, nesse caso o C
     * usa buffer completo, atrasando as mensagens de status até o
     * programa encerrar). Garante que as mensagens pedidas pelo
     * enunciado - cor/escala de cinza, criado/sobrescrito - apareçam
     * imediatamente em qualquer cenário de execução. */
    setvbuf(stdout, NULL, _IOLBF, 0);

    if (argc != 2) {
        fprintf(stderr, "Uso: %s caminho_da_imagem.ext\n", argv[0]);
        return 1;
    }

    if (!image_load_as_grayscale(argv[1], &image)) {
        /* image_load_as_grayscale já imprime a mensagem de erro pertinente. */
        return 1;
    }

    result = gui_run(&image);

    image_free(&image);

    return result;
}
