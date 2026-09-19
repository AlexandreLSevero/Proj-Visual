#include "gui.h"

#include <stdio.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "button.h"
#include "pixel_ops.h"
#include "text.h"

#define MAIN_WINDOW_DEFAULT_W 1024
#define MAIN_WINDOW_DEFAULT_H 768

#define SECONDARY_WINDOW_W 380
#define SECONDARY_WINDOW_H 480

#define HISTOGRAM_RECT_X 20.0f
#define HISTOGRAM_RECT_Y 20.0f
#define HISTOGRAM_RECT_W 340.0f
#define HISTOGRAM_RECT_H 160.0f

#define OUTPUT_FILE_NAME "output_image.png"

typedef enum { RES_MODE_FIXED_1024x768 = 0, RES_MODE_ORIGINAL } ResolutionMode;

typedef struct {
    AppImage *image;

    SDL_Window *main_window;
    SDL_Renderer *main_renderer;
    SDL_Window *secondary_window;
    SDL_Renderer *secondary_renderer;

    TTF_Font *font;

    SDL_Texture *main_texture; /* imagem atual (cinza ou equalizada), resolução nativa */

    int use_equalized;
    ResolutionMode res_mode;

    Histogram hist;
    SDL_Texture *text_mean_texture;
    SDL_Texture *text_stddev_texture;
    int text_mean_w, text_mean_h;
    int text_stddev_w, text_stddev_h;

    Button btn_equalize;
    Button btn_resolution;

    int mouse_down;
    Button *pressed_button; /* botão sob o mouse no momento do clique inicial */

    int running;
} AppState;

/* ------------------------------------------------------------------ */
/* Utilidades internas                                                 */
/* ------------------------------------------------------------------ */

static void refresh_main_texture(AppState *app) {
    const u8 *gray = image_current_gray(app->image, app->use_equalized);
    SDL_Surface *surface = image_gray_to_surface(gray, app->image->width, app->image->height);

    if (app->main_texture != NULL) {
        SDL_DestroyTexture(app->main_texture);
        app->main_texture = NULL;
    }

    if (surface == NULL) {
        return;
    }

    app->main_texture = SDL_CreateTextureFromSurface(app->main_renderer, surface);
    SDL_DestroySurface(surface);

    if (app->main_texture == NULL) {
        fprintf(stderr, "Erro ao criar textura da imagem: %s\n", SDL_GetError());
    }
}

static void make_text_texture(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                               SDL_Texture **out_tex, int *out_w, int *out_h) {
    SDL_Surface *surface;
    static const SDL_Color COLOR_TEXTO_ESCURO = {20, 20, 20, 255};

    if (*out_tex != NULL) {
        SDL_DestroyTexture(*out_tex);
        *out_tex = NULL;
    }

    if (font == NULL) {
        return;
    }

    surface = TTF_RenderText_Blended(font, text, 0, COLOR_TEXTO_ESCURO);
    if (surface == NULL) {
        fprintf(stderr, "Aviso: falha ao renderizar texto \"%s\": %s\n", text, SDL_GetError());
        return;
    }

    *out_tex = SDL_CreateTextureFromSurface(renderer, surface);
    *out_w = surface->w;
    *out_h = surface->h;
    SDL_DestroySurface(surface);
}

static void refresh_histogram_and_texts(AppState *app) {
    char buf[128];
    const u8 *gray = image_current_gray(app->image, app->use_equalized);
    size_t count = (size_t)app->image->width * (size_t)app->image->height;

    histogram_compute(gray, count, &app->hist);

    SDL_snprintf(buf, sizeof(buf), "Media de intensidade: %.1f (%s)", app->hist.mean,
                 histogram_classify_brightness(app->hist.mean));
    make_text_texture(app->secondary_renderer, app->font, buf, &app->text_mean_texture,
                       &app->text_mean_w, &app->text_mean_h);

    SDL_snprintf(buf, sizeof(buf), "Desvio padrao: %.1f (contraste %s)", app->hist.stddev,
                 histogram_classify_contrast(app->hist.stddev));
    make_text_texture(app->secondary_renderer, app->font, buf, &app->text_stddev_texture,
                       &app->text_stddev_w, &app->text_stddev_h);
}

static void main_window_apply_resolution(AppState *app) {
    int target_w, target_h;
    SDL_DisplayID display_id;
    const SDL_DisplayMode *mode;

    if (app->res_mode == RES_MODE_FIXED_1024x768) {
        target_w = MAIN_WINDOW_DEFAULT_W;
        target_h = MAIN_WINDOW_DEFAULT_H;
    } else {
        target_w = app->image->width;
        target_h = app->image->height;
    }

    SDL_SetWindowSize(app->main_window, target_w, target_h);

    display_id = SDL_GetPrimaryDisplay();
    mode = SDL_GetCurrentDisplayMode(display_id);

    if (mode != NULL && (target_w > mode->w || target_h > mode->h)) {
        /* Excede a resolução do sistema: canto superior esquerdo em (0,0). */
        SDL_SetWindowPosition(app->main_window, 0, 0);
    } else {
        SDL_SetWindowPosition(app->main_window, SDL_WINDOWPOS_CENTERED_DISPLAY(display_id),
                               SDL_WINDOWPOS_CENTERED_DISPLAY(display_id));
    }
}

static void save_currently_displayed_image(AppState *app) {
    const u8 *gray = image_current_gray(app->image, app->use_equalized);
    SDL_Surface *native_surface =
        image_gray_to_surface(gray, app->image->width, app->image->height);
    SDL_Surface *to_save = native_surface;
    SDL_Surface *scaled = NULL;
    int overwritten;

    if (native_surface == NULL) {
        fprintf(stderr, "Erro: não foi possível preparar a imagem para salvar.\n");
        return;
    }

    if (app->res_mode == RES_MODE_FIXED_1024x768) {
        scaled = SDL_ScaleSurface(native_surface, MAIN_WINDOW_DEFAULT_W, MAIN_WINDOW_DEFAULT_H,
                                   SDL_SCALEMODE_LINEAR);
        if (scaled != NULL) {
            to_save = scaled;
        } else {
            fprintf(stderr,
                    "Aviso: falha ao redimensionar para salvar em 1024x768 (%s); salvando na "
                    "resolução original.\n",
                    SDL_GetError());
        }
    }

    overwritten = (SDL_GetPathInfo(OUTPUT_FILE_NAME, NULL));

    if (IMG_SavePNG(to_save, OUTPUT_FILE_NAME)) {
        if (overwritten) {
            printf("Arquivo %s sobrescrito.\n", OUTPUT_FILE_NAME);
        } else {
            printf("Arquivo %s criado.\n", OUTPUT_FILE_NAME);
        }
    } else {
        fprintf(stderr, "Erro ao salvar %s: %s\n", OUTPUT_FILE_NAME, SDL_GetError());
    }

    if (scaled != NULL) {
        SDL_DestroySurface(scaled);
    }
    SDL_DestroySurface(native_surface);
}

/* ------------------------------------------------------------------ */
/* Ações dos botões                                                     */
/* ------------------------------------------------------------------ */

static void on_click_equalize(AppState *app) {
    if (!app->use_equalized) {
        image_equalize(app->image);
        app->use_equalized = 1;
        button_set_label(&app->btn_equalize, "Ver original");
    } else {
        app->use_equalized = 0;
        button_set_label(&app->btn_equalize, "Equalizar");
    }

    refresh_main_texture(app);
    refresh_histogram_and_texts(app);
}

static void on_click_resolution(AppState *app) {
    if (app->res_mode == RES_MODE_FIXED_1024x768) {
        app->res_mode = RES_MODE_ORIGINAL;
        button_set_label(&app->btn_resolution, "1024x768");
    } else {
        app->res_mode = RES_MODE_FIXED_1024x768;
        button_set_label(&app->btn_resolution, "Resolucao original");
    }

    main_window_apply_resolution(app);
}

/* ------------------------------------------------------------------ */
/* Inicialização e limpeza                                             */
/* ------------------------------------------------------------------ */

static int app_init(AppState *app, AppImage *image) {
    memset(app, 0, sizeof(*app));
    app->image = image;
    app->running = 1;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Erro ao inicializar a SDL: %s\n", SDL_GetError());
        return 0;
    }

    app->main_window =
        SDL_CreateWindow("Proj1 - Processamento de Imagens", MAIN_WINDOW_DEFAULT_W,
                          MAIN_WINDOW_DEFAULT_H, SDL_WINDOW_RESIZABLE);
    if (app->main_window == NULL) {
        fprintf(stderr, "Erro ao criar a janela principal: %s\n", SDL_GetError());
        return 0;
    }
    SDL_SetWindowPosition(app->main_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    app->main_renderer = SDL_CreateRenderer(app->main_window, NULL);
    if (app->main_renderer == NULL) {
        fprintf(stderr, "Erro ao criar o renderer da janela principal: %s\n", SDL_GetError());
        return 0;
    }

    app->secondary_window =
        SDL_CreateWindow("Histograma e controles", SECONDARY_WINDOW_W, SECONDARY_WINDOW_H, 0);
    if (app->secondary_window == NULL) {
        fprintf(stderr, "Erro ao criar a janela secundária: %s\n", SDL_GetError());
        return 0;
    }
    SDL_SetWindowPosition(app->secondary_window, 0, 0);
    /* Torna a janela secundária "filha" da janela principal, conforme
     * pede o Item 3 do enunciado. */
    SDL_SetWindowParent(app->secondary_window, app->main_window);

    app->secondary_renderer = SDL_CreateRenderer(app->secondary_window, NULL);
    if (app->secondary_renderer == NULL) {
        fprintf(stderr, "Erro ao criar o renderer da janela secundária: %s\n", SDL_GetError());
        return 0;
    }

    app->font = text_load_app_font();
    /* Mesmo se a fonte falhar ao carregar, o programa continua: os
     * textos simplesmente não serão desenhados (ver make_text_texture e
     * button_render), mas todo o resto (histograma, botões, imagem)
     * continua funcionando. */

    button_init(&app->btn_equalize, HISTOGRAM_RECT_X, 260.0f, HISTOGRAM_RECT_W, 50.0f,
                "Equalizar");
    button_init(&app->btn_resolution, HISTOGRAM_RECT_X, 325.0f, HISTOGRAM_RECT_W, 50.0f,
                "Resolucao original");

    app->use_equalized = 0;
    app->res_mode = RES_MODE_FIXED_1024x768;

    refresh_main_texture(app);
    refresh_histogram_and_texts(app);

    return 1;
}

static void app_shutdown(AppState *app) {
    button_destroy(&app->btn_equalize);
    button_destroy(&app->btn_resolution);

    if (app->text_mean_texture != NULL) {
        SDL_DestroyTexture(app->text_mean_texture);
    }
    if (app->text_stddev_texture != NULL) {
        SDL_DestroyTexture(app->text_stddev_texture);
    }
    if (app->main_texture != NULL) {
        SDL_DestroyTexture(app->main_texture);
    }

    if (app->font != NULL) {
        TTF_CloseFont(app->font);
    }
    text_quit();

    if (app->secondary_renderer != NULL) {
        SDL_DestroyRenderer(app->secondary_renderer);
    }
    if (app->secondary_window != NULL) {
        SDL_DestroyWindow(app->secondary_window);
    }
    if (app->main_renderer != NULL) {
        SDL_DestroyRenderer(app->main_renderer);
    }
    if (app->main_window != NULL) {
        SDL_DestroyWindow(app->main_window);
    }

    SDL_Quit();
}

/* ------------------------------------------------------------------ */
/* Eventos                                                              */
/* ------------------------------------------------------------------ */

static void handle_secondary_mouse_motion(AppState *app, float x, float y) {
    button_update_state(&app->btn_equalize, x, y, app->mouse_down && app->pressed_button == &app->btn_equalize);
    button_update_state(&app->btn_resolution, x, y,
                         app->mouse_down && app->pressed_button == &app->btn_resolution);
}

static void handle_secondary_mouse_down(AppState *app, float x, float y) {
    app->mouse_down = 1;

    if (button_contains(&app->btn_equalize, x, y)) {
        app->pressed_button = &app->btn_equalize;
    } else if (button_contains(&app->btn_resolution, x, y)) {
        app->pressed_button = &app->btn_resolution;
    } else {
        app->pressed_button = NULL;
    }

    handle_secondary_mouse_motion(app, x, y);
}

static void handle_secondary_mouse_up(AppState *app, float x, float y) {
    app->mouse_down = 0;

    if (app->pressed_button == &app->btn_equalize && button_contains(&app->btn_equalize, x, y)) {
        on_click_equalize(app);
    } else if (app->pressed_button == &app->btn_resolution &&
               button_contains(&app->btn_resolution, x, y)) {
        on_click_resolution(app);
    }

    app->pressed_button = NULL;
    handle_secondary_mouse_motion(app, x, y);
}

static void process_events(AppState *app) {
    SDL_Event event;
    SDL_WindowID secondary_id = SDL_GetWindowID(app->secondary_window);

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                app->running = 0;
                break;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                /* Fechar qualquer uma das janelas encerra o programa. */
                app->running = 0;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_S) {
                    save_currently_displayed_image(app);
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (event.motion.windowID == secondary_id) {
                    handle_secondary_mouse_motion(app, event.motion.x, event.motion.y);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.windowID == secondary_id && event.button.button == SDL_BUTTON_LEFT) {
                    handle_secondary_mouse_down(app, event.button.x, event.button.y);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.windowID == secondary_id && event.button.button == SDL_BUTTON_LEFT) {
                    handle_secondary_mouse_up(app, event.button.x, event.button.y);
                }
                break;

            default:
                break;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Renderização                                                        */
/* ------------------------------------------------------------------ */

static void render_main_window(AppState *app) {
    int win_w, win_h;
    SDL_FRect dst;

    SDL_GetWindowSizeInPixels(app->main_window, &win_w, &win_h);

    SDL_SetRenderDrawColor(app->main_renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->main_renderer);

    if (app->main_texture != NULL) {
        dst.x = 0.0f;
        dst.y = 0.0f;
        dst.w = (float)win_w;
        dst.h = (float)win_h;
        SDL_RenderTexture(app->main_renderer, app->main_texture, NULL, &dst);
    }

    SDL_RenderPresent(app->main_renderer);
}

static void render_histogram(AppState *app) {
    int i;
    int max_count = 1;
    float bar_width = HISTOGRAM_RECT_W / (float)HISTOGRAM_LEVELS;

    for (i = 0; i < HISTOGRAM_LEVELS; i++) {
        if (app->hist.counts[i] > max_count) {
            max_count = app->hist.counts[i];
        }
    }

    /* Moldura do histograma. */
    SDL_SetRenderDrawColor(app->secondary_renderer, 255, 255, 255, 255);
    {
        SDL_FRect frame = {HISTOGRAM_RECT_X, HISTOGRAM_RECT_Y, HISTOGRAM_RECT_W, HISTOGRAM_RECT_H};
        SDL_RenderFillRect(app->secondary_renderer, &frame);
    }
    SDL_SetRenderDrawColor(app->secondary_renderer, 120, 120, 120, 255);
    {
        SDL_FRect frame = {HISTOGRAM_RECT_X, HISTOGRAM_RECT_Y, HISTOGRAM_RECT_W, HISTOGRAM_RECT_H};
        SDL_RenderRect(app->secondary_renderer, &frame);
    }

    SDL_SetRenderDrawColor(app->secondary_renderer, 40, 40, 40, 255);
    for (i = 0; i < HISTOGRAM_LEVELS; i++) {
        float bar_height = (float)app->hist.counts[i] / (float)max_count * HISTOGRAM_RECT_H;
        SDL_FRect bar;
        bar.x = HISTOGRAM_RECT_X + (float)i * bar_width;
        bar.w = bar_width > 1.0f ? bar_width : 1.0f;
        bar.h = bar_height;
        bar.y = HISTOGRAM_RECT_Y + (HISTOGRAM_RECT_H - bar_height);
        SDL_RenderFillRect(app->secondary_renderer, &bar);
    }
}

static void render_secondary_window(AppState *app) {
    SDL_SetRenderDrawColor(app->secondary_renderer, 230, 230, 230, 255);
    SDL_RenderClear(app->secondary_renderer);

    render_histogram(app);

    if (app->text_mean_texture != NULL) {
        SDL_FRect dst = {HISTOGRAM_RECT_X, 195.0f, (float)app->text_mean_w, (float)app->text_mean_h};
        SDL_RenderTexture(app->secondary_renderer, app->text_mean_texture, NULL, &dst);
    }
    if (app->text_stddev_texture != NULL) {
        SDL_FRect dst = {HISTOGRAM_RECT_X, 220.0f, (float)app->text_stddev_w,
                          (float)app->text_stddev_h};
        SDL_RenderTexture(app->secondary_renderer, app->text_stddev_texture, NULL, &dst);
    }

    button_render(&app->btn_equalize, app->secondary_renderer, app->font);
    button_render(&app->btn_resolution, app->secondary_renderer, app->font);

    SDL_RenderPresent(app->secondary_renderer);
}

/* ------------------------------------------------------------------ */
/* Laço principal                                                       */
/* ------------------------------------------------------------------ */

int gui_run(AppImage *image) {
    AppState app;

    if (!app_init(&app, image)) {
        app_shutdown(&app);
        return 1;
    }

    while (app.running) {
        process_events(&app);
        render_main_window(&app);
        render_secondary_window(&app);
        SDL_Delay(10); /* ~100 fps de teto: suficiente para uma UI simples */
    }

    app_shutdown(&app);
    return 0;
}
