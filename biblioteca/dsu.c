#include "dsu.h"
#include <stdlib.h>

int dsu_init(DSU* d, int n) {
    d->n = n;
    /*aloca vetores*/
    d->parent = (int*)malloc(sizeof(int) * (size_t)n);
    d->rnk    = (int*)malloc(sizeof(int) * (size_t)n);
    d->sz     = (int*)malloc(sizeof(int) * (size_t)n);

    if (!d->parent || !d->rnk || !d->sz) return 0;
    /* cada elemento começa em seu próprio grupo */
    for (int i = 0; i < n; i++) {
        d->parent[i] = i;
        d->rnk[i] = 0;
        d->sz[i] = 1;
    }
    return 1;
}

void dsu_free(DSU* d) {
    if (!d) return;
    free(d->parent);
    free(d->rnk);
    free(d->sz);
    d->parent = NULL;
    d->rnk = NULL;
    d->sz = NULL;
    d->n = 0;
}

int dsu_find(DSU* d, int x) {
    if (d->parent[x] != x)
        d->parent[x] = dsu_find(d, d->parent[x]); 
    return d->parent[x];
}

/* Junta os grupos de a e b */
void dsu_union(DSU* d, int a, int b) {
    int ra = dsu_find(d, a);
    int rb = dsu_find(d, b);
    /* se já estão no mesmo grupo, não faz nada */
    if (ra == rb) return;

    /* liga o menor ao maior */
    if (d->rnk[ra] < d->rnk[rb]) {
        d->parent[ra] = rb;
        d->sz[rb] += d->sz[ra];
    } else if (d->rnk[ra] > d->rnk[rb]) {
        d->parent[rb] = ra;
        d->sz[ra] += d->sz[rb];
    } else {
        d->parent[rb] = ra;
        d->rnk[ra]++;
        d->sz[ra] += d->sz[rb];
    }
}

/* Verifica se dois elementos estão no mesmo grupo */
int dsu_same(DSU* d, int a, int b) {
    return dsu_find(d, a) == dsu_find(d, b);
}
/* Retorna o tamanho do grupo de a */

int dsu_size(DSU* d, int a) {
    int ra = dsu_find(d, a);
    return d->sz[ra];
}
