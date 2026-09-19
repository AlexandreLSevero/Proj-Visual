# Proj1 — Processamento de Imagens (Computação Visual)
## O que é o projeto

Um programa de linha de comando com interface gráfica (SDL3) que:

1. Carrega uma imagem (`SDL_image`), detectando se ela é colorida ou já está em escala de cinza.
2. Converte imagens coloridas para escala de cinza usando `Y = 0.2125*R + 0.7154*G + 0.0721*B`.
3. Exibe a imagem em uma janela principal e, em uma janela secundária (filha da principal), o
   histograma da imagem com sua média de intensidade (clara/média/escura) e desvio padrão
   (contraste alto/médio/baixo).
4. Permite equalizar o histograma (e reverter) através de um botão.
5. Permite alternar a exibição entre a resolução original da imagem e 1024x768 através de outro
   botão.
6. Salva a imagem atualmente exibida em `output_image.png` ao pressionar a tecla **S**.

## Como funciona (visão geral do código)

```
src/
  pixel_ops.c/.h  — funções puras de processamento (RGB→cinza, histograma, estatísticas,
                     equalização). Não dependem da SDL, o que permite testá-las isoladamente
                     (ver tests/test_pixel_ops.c).
  image.c/.h      — carregamento da imagem via SDL_image, conversão para escala de cinza,
                     conversão de volta para SDL_Surface para exibição/salvamento.
  button.c/.h     — botão desenhado com primitivas SDL, com estados neutro/hover/clicado.
  text.c/.h       — inicialização da SDL_ttf e carregamento da fonte do projeto de forma
                     independente do sistema operacional (via SDL_GetBasePath()).
  gui.c/.h        — orquestra as duas janelas, o histograma, os botões, os eventos de
                     mouse/teclado e o laço principal de renderização.
  main.c          — ponto de entrada: lê o argumento de linha de comando e chama os módulos
                     acima.
tests/
  test_pixel_ops.c — testes unitários das funções de src/pixel_ops.c (sem depender da SDL).
assets/
  fonts/DejaVuSans.ttf — fonte usada nos textos (ver seção "Fonte utilizada").
  sample_images/       — imagens de exemplo para testar o programa localmente.
```

### Uso

```
proj1 caminho_da_imagem.ext
```

Controles:

- Botão **Equalizar / Ver original** (janela secundária): alterna entre a imagem equalizada e a
  versão original em escala de cinza.
- Botão **Resolução original / 1024x768** (janela secundária): alterna a resolução de exibição na
  janela principal.
- Tecla **S**: salva a imagem atualmente exibida como `output_image.png` (sobrescreve se já
  existir).
- Fechar qualquer uma das janelas encerra o programa.

## Fonte utilizada

O projeto usa **DejaVu Sans** (`assets/fonts/DejaVuSans.ttf`) para todos os textos renderizados
com `SDL_ttf` (informações do histograma e rótulos dos botões). Foi escolhida por ser uma fonte
livre para redistribuição (licença Bitstream Vera — ver `assets/fonts/DejaVuSans-LICENSE.txt`),
com bom suporte a acentuação do português e por já estar embutida no repositório, garantindo que
o texto seja carregado da mesma forma em qualquer sistema operacional (o caminho é resolvido em
tempo de execução a partir do diretório do executável via `SDL_GetBasePath()`, nunca por um
caminho absoluto fixo).

*(Se o grupo decidir trocar a fonte, atualizar esta seção e o arquivo em `assets/fonts/`.)*

## Como compilar e executar

### Dependências

- Compilador **GCC** com suporte a C99 ou mais recente.
- **SDL3**, **SDL3_image** e **SDL3_ttf**.

> Estas versões da SDL são recentes e ainda não estão disponíveis via `apt` nas distribuições Linux
> mais comuns (nem via instaladores tradicionais no Windows), então normalmente é preciso compilar
> a partir do código-fonte oficial uma única vez (ver abaixo). As versões usadas para desenvolver e
> testar este projeto foram:
>
> | Biblioteca   | Versão  |
> |--------------|---------|
> | SDL3         | 3.4.16  |
> | SDL3_image   | 3.4.6   |
> | SDL3_ttf     | 3.2.2   |

### Linux / WSL (Ubuntu)

```bash
# Dependências de sistema para compilar a SDL a partir do código-fonte
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build git pkg-config \
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxfixes-dev \
    libxss-dev libxtst-dev libxkbcommon-dev libwayland-dev wayland-protocols \
    libdrm-dev libgbm-dev libgl1-mesa-dev libegl1-mesa-dev \
    libasound2-dev libpulse-dev libdbus-1-dev libudev-dev \
    libpng-dev libjpeg-dev zlib1g-dev libfreetype-dev libharfbuzz-dev

# Compilar e instalar SDL3, SDL3_image e SDL3_ttf (uma única vez por máquina)
for repo in "SDL 3.4.16" "SDL_image 3.4.6" "SDL_ttf 3.2.2"; do
  set -- $repo
  curl -sSL -o /tmp/$1.tar.gz "https://github.com/libsdl-org/$1/releases/download/release-$2/$1-$2.tar.gz" 2>/dev/null || \
  curl -sSL -o /tmp/$1.tar.gz "https://github.com/libsdl-org/$1/releases/download/release-$2/SDL3-$2.tar.gz"
done
# (ou baixe manualmente os .tar.gz das páginas de release de cada projeto no GitHub)
# Para cada biblioteca extraída:
#   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
#   cmake --build build && sudo cmake --install build && sudo ldconfig
# (SDL3_image e SDL3_ttf: adicione -DSDLIMAGE_VENDORED=OFF / -DSDLTTF_VENDORED=OFF
#  para usar libpng/libjpeg/freetype/harfbuzz do sistema em vez de baixar dependências extras)

# Compilar o projeto
make
./proj1 assets/sample_images/sample_color.png

# Rodar os testes unitários (não depende da SDL)
make test
```

### Windows 10/11 (GCC / MinGW-w64)

1. Instale o **MSYS2** (ou outra distribuição MinGW-w64 com GCC 15.x) e, pelo terminal MSYS2:
   ```
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja \
             mingw-w64-x86_64-SDL3 mingw-w64-x86_64-SDL3_image mingw-w64-x86_64-SDL3_ttf
   ```
   Caso os pacotes `SDL3` ainda não estejam disponíveis no repositório do MSYS2, baixe os pacotes
   de desenvolvimento pré-compilados (`SDL3-devel-*-mingw.zip`, etc.) das páginas de release oficiais
   no GitHub (`libsdl-org/SDL`, `libsdl-org/SDL_image`, `libsdl-org/SDL_ttf`) e extraia cada um em
   uma pasta, por exemplo `C:\libs\SDL3`, `C:\libs\SDL3_image`, `C:\libs\SDL3_ttf`.
2. Compile informando os prefixos ao `make` (o `Makefile` detecta automaticamente o Windows e usa
   `pkg-config` se disponível; caso contrário, cai no modo manual abaixo):
   ```
   make SDL3_DIR=C:/libs/SDL3 SDL3_IMAGE_DIR=C:/libs/SDL3_image SDL3_TTF_DIR=C:/libs/SDL3_ttf
   ```
3. Copie as DLLs de `bin/` de cada biblioteca para a pasta do executável (`proj1.exe`) antes de
   executar, ou adicione-as ao `PATH`.
4. Integração com VS Code: crie `.vscode/tasks.json` chamando `mingw32-make` (ou `make`), como já é
   feito no repositório de exemplo da disciplina.

### Observação sobre portabilidade

O `Makefile` detecta o sistema operacional (`Windows_NT` vs. Linux/WSL) para escolher os comandos
de limpeza corretos (`del`/`rmdir` no Windows, `rm` no Linux) e tenta usar `pkg-config` para localizar
as bibliotecas SDL automaticamente; quando isso não é possível (comum em instalações manuais no
Windows), variáveis de ambiente (`SDL3_DIR`, `SDL3_IMAGE_DIR`, `SDL3_TTF_DIR`) permitem apontar
manualmente para os includes/libs, sem depender de caminhos fixos de uma única máquina.

## Integrantes e contribuições

| Nome completo     | RA       | Contribuições |
|-------------------|----------|---------------|
| Alexandre Luppi   | 10419724 | Teste do código no ambiente Windows, revisão da documentação e revisão do relatório de analise final |
| Enrico Spanier    | 10419775 | construção do código do projeto e teste em ambiente linux |
| Guilherme Vecchi  | 10418517 | estruturamento lógico das funções do projeto  |
| Matteo Porcare    | 10418276 | revisão e documentação do código |
