/*
 * gui.h
 *
 * Orquestra as duas janelas (Item 3), o histograma (Item 4), os botões
 * de equalização (Item 5) e de resolução (Item 6), o salvamento por
 * tecla (Item 7) e o carregamento de texto (Item 8). É o "coração" do
 * programa: gui_run() é chamada por main() depois que a imagem já foi
 * carregada e convertida para escala de cinza.
 */
#ifndef PROJ_VISUAL_GUI_H
#define PROJ_VISUAL_GUI_H

#include "image.h"

/*
 * Executa a aplicação gráfica completa para a imagem já carregada em
 * `image` (ver image_load_as_grayscale). Bloqueia até o usuário fechar
 * a janela. Retorna 0 em caso de sucesso, ou um código de erro != 0 se
 * a inicialização de vídeo/janelas/fonte falhar.
 */
int gui_run(AppImage *image);

#endif /* PROJ_VISUAL_GUI_H */
