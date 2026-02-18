#ifndef BPTREE_H
#define BPTREE_H

#include "livros.h"

#define BP_ORDER 4   /* número máximo de filhos por nó */

/* Nó da B+ Tree */
typedef struct BPNode {
    int leaf;
    int nkeys;
    long long keys[BP_ORDER - 1];

    struct BPNode* child[BP_ORDER];  /* internos */
    Book* vals[BP_ORDER - 1];        /* folhas */

    struct BPNode* next;             /* folhas encadeadas */
} BPNode;

typedef struct {
    BPNode* root;
} BPTree;


BPTree* bpt_create(void);
void    bpt_free(BPTree* t);

/* operações */
Book*   bpt_search(BPTree* t, long long isbn);
void    bpt_insert(BPTree* t, long long isbn, Book* book);

/* Mostra todos os livros com ISBN entre a e b */
void    bpt_print_range(BPTree* t, long long a, long long b);

/* Cria a B+ Tree a partir da lista de livros */
BPTree* bpt_build_from_list(BookNode* books);

#endif
