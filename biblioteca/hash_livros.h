#ifndef HASH_LIVROS_H
#define HASH_LIVROS_H

#include "livros.h"

typedef struct HashBookNode {
    Book* book;
    struct HashBookNode* next;
} HashBookNode;

typedef struct {
    HashBookNode** buckets;
    int size; /* número de buckets */
} HashBooks;

/* lifecycle */
int  hb_init(HashBooks* hb, int size);
void hb_free(HashBooks* hb);

/* operações */
void hb_build_from_list(HashBooks* hb, BookNode* head);
Book* hb_get(HashBooks* hb, long long isbn);
int  hb_insert(HashBooks* hb, Book* book);      /* 1 se inseriu, 0 se já existia */
int  hb_remove(HashBooks* hb, long long isbn);  /* 1 se removeu, 0 se não achou */

#endif
