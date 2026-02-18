#ifndef TOP_LIVROS_H
#define TOP_LIVROS_H

#include "livros.h"

typedef struct {
    Book** data;   /* vetor de ponteiros para Book */
    int size;      /* quantidade atual */
    int cap;       /* capacidade */
} BookHeap;

/* Inicializa o heap com capacidade inicial 'cap' */
int  heap_init(BookHeap* h, int cap);
void heap_free(BookHeap* h);

/* construir do zero a partir da lista */
int  heap_build_from_list(BookHeap* h, BookNode* head);

/* operações principais (fila de prioridade / max-heap) */
Book* heap_peek(BookHeap* h);          /* maior prioridade */
Book* heap_pop(BookHeap* h);           /* remove maior */
int   heap_push(BookHeap* h, Book* b); /* insere */

/* imprime TOP-K sem destruir o heap original */
void heap_print_top(BookHeap* h, int k);

#endif
