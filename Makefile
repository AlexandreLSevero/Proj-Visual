# Makefile - Proj1 (Processamento de Imagens)
#
# Compatível com:
#   - GCC 15.1.0 no Windows 10/11 (MinGW-w64, via mingw32-make ou make)
#   - GCC 15.2.0 no WSL Ubuntu 26.04
#
# Detecta o sistema operacional automaticamente para escolher os
# comandos de limpeza corretos (del/rmdir no Windows, rm no Linux) e usa
# pkg-config quando disponível para localizar as flags de compilação e
# ligação das bibliotecas SDL3, sem depender de caminhos absolutos
# fixos de uma máquina específica.
#
# Variáveis de ambiente que podem ser usadas para apontar para uma
# instalação manual das bibliotecas SDL (caso pkg-config não as
# encontre), por exemplo no Windows:
#   SDL3_DIR, SDL3_IMAGE_DIR, SDL3_TTF_DIR (cada uma contendo include/ e lib/)

CC := gcc
STD := -std=c99
WARN := -Wall -Wextra
OPT := -O2
SRC_DIR := src
BUILD_DIR := build
TARGET_NAME := proj1

ifeq ($(OS),Windows_NT)
    DETECTED_OS := Windows
else
    DETECTED_OS := $(shell uname -s)
endif

ifeq ($(DETECTED_OS),Windows)
    TARGET := $(TARGET_NAME).exe
    RM := rm -f
    RMDIR := rm -rf
    MKDIR := mkdir -p $(BUILD_DIR)
    PATHSEP := \\
else
    TARGET := $(TARGET_NAME)
    RM := rm -f
    RMDIR := rm -rf
    MKDIR := mkdir -p $(BUILD_DIR)
    PATHSEP := /
endif

PKG_CONFIG := pkg-config
HAVE_PKGCONFIG := $(shell $(PKG_CONFIG) --exists sdl3 sdl3-image sdl3-ttf 2>/dev/null && echo yes)

ifeq ($(HAVE_PKGCONFIG),yes)
    SDL_CFLAGS := $(shell $(PKG_CONFIG) --cflags sdl3 sdl3-image sdl3-ttf)
    SDL_LIBS := $(shell $(PKG_CONFIG) --libs sdl3 sdl3-image sdl3-ttf)
else
    # Fallback manual: usado quando pkg-config não encontra os pacotes
    # (comum em instalações manuais no Windows). Ajuste SDL3_DIR,
    # SDL3_IMAGE_DIR e SDL3_TTF_DIR (ou passe-as via linha de comando,
    # ex.: make SDL3_DIR=C:/libs/SDL3) se necessário.
    SDL3_DIR ?= /usr/local
    SDL3_IMAGE_DIR ?= /usr/local
    SDL3_TTF_DIR ?= /usr/local
    SDL_CFLAGS := -I$(SDL3_DIR)/include -I$(SDL3_IMAGE_DIR)/include -I$(SDL3_TTF_DIR)/include
    SDL_LIBS := -L$(SDL3_DIR)/lib -L$(SDL3_IMAGE_DIR)/lib -L$(SDL3_TTF_DIR)/lib \
                -lSDL3 -lSDL3_image -lSDL3_ttf
endif

CFLAGS := $(STD) $(WARN) $(OPT) $(SDL_CFLAGS)
LDFLAGS := $(SDL_LIBS) -lm

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	$(MKDIR)

# Executa o programa com uma imagem de exemplo (ajuste o caminho conforme necessário).
run: $(TARGET)
	./$(TARGET) assets/sample_images/sample_color.png

# Testes unitários das funções puras de processamento de imagem
# (pixel_ops.c), sem dependência da SDL.
test:
	$(CC) $(STD) $(WARN) -O0 -g -o $(BUILD_DIR)/test_pixel_ops tests/test_pixel_ops.c $(SRC_DIR)/pixel_ops.c -lm
	./$(BUILD_DIR)/test_pixel_ops

clean:
ifeq ($(DETECTED_OS),Windows)
	-$(RMDIR) $(BUILD_DIR) 2>/dev/null
	-$(RM) $(TARGET) 2>/dev/null
else
	-$(RMDIR) $(BUILD_DIR)
	-$(RM) $(TARGET)
endif
