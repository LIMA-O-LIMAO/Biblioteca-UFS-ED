#ifndef DSU_H
#define DSU_H

typedef struct {
    int n;
    int* parent;
    int* rnk;   /* ajuda a manter a árvore baixa */
    int* sz;    /* tamanho do conjunto */
} DSU;

int  dsu_init(DSU* d, int n);
void dsu_free(DSU* d);

int  dsu_find(DSU* d, int x);
void dsu_union(DSU* d, int a, int b);

int  dsu_same(DSU* d, int a, int b);
int  dsu_size(DSU* d, int a);

#endif
