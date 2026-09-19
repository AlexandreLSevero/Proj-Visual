#include "image.h"

#include <stdio.h>
#include <stdlib.h>

#include <SDL3_image/SDL_image.h>

int image_load_as_grayscale(const char *path, AppImage *out) {
    SDL_Surface *loaded = NULL;
    SDL_Surface *converted = NULL;
    int w, h;
    int is_color = 0;
    size_t count;
    size_t i;
    u8 *gray = NULL;

    if (path == NULL || path[0] == '\0') {
        fprintf(stderr, "Erro: nenhum caminho de imagem foi informado.\n");
        return 0;
    }

    /* IMG_Load já trata "arquivo não encontrado" e "formato inválido"
     * internamente: em ambos os casos retorna NULL e SDL_GetError()
     * traz o motivo */
    loaded = IMG_Load(path);
    if (loaded == NULL) {
        fprintf(stderr, "Erro ao carregar a imagem \"%s\": %s\n", path, SDL_GetError());
        return 0;
    }

    /* Normaliza para um formato de pixel conhecido (8 bits por canal,
     * com alfa) para simplificar a leitura pixel a pixel abaixo,
     * independentemente do formato original do arquivo */
    converted = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(loaded);
    if (converted == NULL) {
        fprintf(stderr, "Erro ao converter a imagem \"%s\" para RGBA32: %s\n", path,
                SDL_GetError());
        return 0;
    }

    w = converted->w;
    h = converted->h;
    count = (size_t)w * (size_t)h;

    gray = (u8 *)malloc(count);
    if (gray == NULL) {
        fprintf(stderr, "Erro: memória insuficiente para processar a imagem \"%s\".\n", path);
        SDL_DestroySurface(converted);
        return 0;
    }

    if (SDL_MUSTLOCK(converted)) {
        if (!SDL_LockSurface(converted)) {
            fprintf(stderr, "Erro ao acessar os pixels da imagem \"%s\": %s\n", path,
                    SDL_GetError());
            free(gray);
            SDL_DestroySurface(converted);
            return 0;
        }
    }

    {
        const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(converted->format);
        const Uint8 *pixels = (const Uint8 *)converted->pixels;

        for (i = 0; i < count; i++) {
            int row = (int)(i / (size_t)w);
            int col = (int)(i % (size_t)w);
            const Uint32 *pixel_ptr =
                (const Uint32 *)(pixels + (size_t)row * converted->pitch + (size_t)col * 4);
            Uint8 r, g, b, a;

            SDL_GetRGBA(*pixel_ptr, fmt, NULL, &r, &g, &b, &a);
            (void)a;

            if (!pixel_is_gray(r, g, b)) {
                is_color = 1;
                gray[i] = pixel_rgb_to_gray(r, g, b);
            } else {
                gray[i] = r; /* já é cinza: R == G == B */
            }
        }
    }

    if (SDL_MUSTLOCK(converted)) {
        SDL_UnlockSurface(converted);
    }
    SDL_DestroySurface(converted);

    if (is_color) {
        printf("Imagem de entrada: COLORIDA (convertida para escala de cinza).\n");
    } else {
        printf("Imagem de entrada: já está em escala de cinza.\n");
    }

    out->width = w;
    out->height = h;
    out->was_color_input = is_color;
    out->gray_original = gray;
    out->gray_equalized = NULL;
    out->equalized_ready = 0;

    return 1;
}

const u8 *image_current_gray(const AppImage *img, int use_equalized) {
    if (use_equalized && img->equalized_ready) {
        return img->gray_equalized;
    }
    return img->gray_original;
}

void image_equalize(AppImage *img) {
    size_t count = (size_t)img->width * (size_t)img->height;

    if (img->equalized_ready) {
        return; // evita refazer o trabalho a cada clique 
    }

    if (img->gray_equalized == NULL) {
        img->gray_equalized = (u8 *)malloc(count);
        if (img->gray_equalized == NULL) {
            fprintf(stderr, "Erro: memória insuficiente para equalizar o histograma.\n");
            return;
        }
    }

    histogram_equalize(img->gray_original, img->gray_equalized, count);
    img->equalized_ready = 1;
}

SDL_Surface *image_gray_to_surface(const u8 *gray, int width, int height) {
    SDL_Surface *surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    int x, y;

    if (surface == NULL) {
        fprintf(stderr, "Erro ao criar surface %dx%d: %s\n", width, height, SDL_GetError());
        return NULL;
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }

    {
        const SDL_PixelFormatDetails *fmt = SDL_GetPixelFormatDetails(surface->format);
        Uint8 *pixels = (Uint8 *)surface->pixels;

        for (y = 0; y < height; y++) {
            Uint32 *row = (Uint32 *)(pixels + (size_t)y * surface->pitch);
            for (x = 0; x < width; x++) {
                u8 value = gray[(size_t)y * (size_t)width + (size_t)x];
                row[x] = SDL_MapRGBA(fmt, NULL, value, value, value, 255);
            }
        }
    }

    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }

    return surface;
}

void image_free(AppImage *img) {
    free(img->gray_original);
    free(img->gray_equalized);
    img->gray_original = NULL;
    img->gray_equalized = NULL;
    img->equalized_ready = 0;
}
