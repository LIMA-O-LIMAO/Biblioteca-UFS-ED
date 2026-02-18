#ifndef TEXTO_BUSCA_H
#define TEXTO_BUSCA_H

#include "livros.h"


/* Nó da lista ligada de ISBNs */
typedef struct IsbnNode {
    long long isbn;
    struct IsbnNode* next;
} IsbnNode;

/* Entrada da tabela hash:
   cada palavra aponta para uma lista de ISBNs */
typedef struct WordEntry {
    char word[32];
    IsbnNode* isbns;
    struct WordEntry* next;
} WordEntry;

/* Estrutura principal do índice textual */
typedef struct {
    WordEntry** buckets;
    int size;
} TextIndex;

/* Inicializa o índice com 'size' posições */
int  ti_init(TextIndex* ti, int size);
void ti_free(TextIndex* ti);/* Libera toda a memória do índice */

/* Constrói o índice a partir da lista de livros */
void ti_build(TextIndex* ti, BookNode* books);

/* procurar palavra (retorna lista de ISBNs ou NULL) */
IsbnNode* ti_find(TextIndex* ti, const char* word);

#endif
