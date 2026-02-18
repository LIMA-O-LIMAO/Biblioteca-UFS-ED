#include "hash_livros.h"
#include <stdlib.h>
#include <stdio.h>

/* hash simples para long long */
static unsigned int hash_isbn(long long isbn) {
   
    unsigned long long x = (unsigned long long)isbn;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return (unsigned int)(x & 0xFFFFFFFFu);
}

static int bucket_index(HashBooks* hb, long long isbn) {
    unsigned int h = hash_isbn(isbn);
    return (int)(h % (unsigned int)hb->size);
}

int hb_init(HashBooks* hb, int size) {
    hb->size = size;
    hb->buckets = (HashBookNode**)calloc((size_t)size, sizeof(HashBookNode*));
    if (!hb->buckets) return 0;
    return 1;
}

void hb_free(HashBooks* hb) {
    if (!hb || !hb->buckets) return;

    for (int i = 0; i < hb->size; i++) {
        HashBookNode* cur = hb->buckets[i];
        while (cur) {
            HashBookNode* next = cur->next;
            free(cur);
            cur = next;
        }
    }
    free(hb->buckets);
    hb->buckets = NULL;
    hb->size = 0;
}

Book* hb_get(HashBooks* hb, long long isbn) {
    if (!hb || !hb->buckets) return NULL;

    int idx = bucket_index(hb, isbn);
    for (HashBookNode* cur = hb->buckets[idx]; cur; cur = cur->next) {
        if (cur->book->isbn == isbn) return cur->book;
    }
    return NULL;
}

int hb_insert(HashBooks* hb, Book* book) {
    if (!hb || !hb->buckets || !book) return 0;

    if (hb_get(hb, book->isbn)) return 0; /* já existe */

    int idx = bucket_index(hb, book->isbn);
    HashBookNode* n = (HashBookNode*)malloc(sizeof(HashBookNode));
    if (!n) {
        printf("Erro: sem memória.\n");
        exit(1);
    }
    n->book = book;
    n->next = hb->buckets[idx];
    hb->buckets[idx] = n;
    return 1;
}

int hb_remove(HashBooks* hb, long long isbn) {
    if (!hb || !hb->buckets) return 0;

    int idx = bucket_index(hb, isbn);
    HashBookNode* prev = NULL;
    HashBookNode* cur = hb->buckets[idx];

    while (cur) {
        if (cur->book->isbn == isbn) {
            if (prev) prev->next = cur->next;
            else hb->buckets[idx] = cur->next;
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

void hb_build_from_list(HashBooks* hb, BookNode* head) {
    /* assume hash vazia; em uso real, chamar hb_free/hb_init ou limpar antes */
    for (BookNode* cur = head; cur; cur = cur->next) {
        hb_insert(hb, &cur->data);
    }
}
