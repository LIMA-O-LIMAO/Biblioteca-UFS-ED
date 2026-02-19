# =========================
# Makefile - Biblioteca (Windows/MinGW)
# =========================

CC      := gcc
CFLAGS  := -Wall -Wextra -O2
LDFLAGS :=

TARGET  := biblioteca.exe

SRC := main.c \
       livros.c \
       usuarios.c \
       busca_usuarios.c \
       emprestimos.c \
       avl.c \
       hash_livros.c \
       top_livros.c \
       dsu.c \
       texto_busca.c \
       bptree.c

OBJ := $(SRC:.c=.o)

.PHONY: all clean run rebuild

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	.\$(TARGET)

clean:
	del /Q $(OBJ) $(TARGET) 2>nul

rebuild: clean all
