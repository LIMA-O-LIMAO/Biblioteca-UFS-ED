#include "bptree.h"
#include <stdlib.h>
#include <stdio.h>

/* Cria um novo nó da B+ Tree */
static BPNode* bp_new(int leaf) {
    BPNode* n = (BPNode*)malloc(sizeof(BPNode));
    if (!n) { printf("Erro: sem memória.\n"); exit(1); }
    n->leaf = leaf; /* 1 = folha, 0 = nó interno */
    n->nkeys = 0; /* começa sem chaves */
    n->next = NULL; /* usado só em folhas */
    // zera filhos e valores
    for (int i = 0; i < BP_ORDER; i++) n->child[i] = NULL;
    for (int i = 0; i < BP_ORDER - 1; i++) n->vals[i] = NULL;
    return n;
}

/* Libera um nó e seus filhos */
static void bp_free_node(BPNode* n) {
    if (!n) return;
    if (!n->leaf) {
        for (int i = 0; i <= n->nkeys; i++) bp_free_node(n->child[i]);
    }
    free(n);
}
/* ---------- Criação e destruição ---------- */
BPTree* bpt_create(void) {
    BPTree* t = (BPTree*)malloc(sizeof(BPTree));
    if (!t) { printf("Erro: sem memória.\n"); exit(1); }
    t->root = bp_new(1);  /*  raiz começa como folha */

    return t;
}
/* Libera toda a árvore */

void bpt_free(BPTree* t) {
    if (!t) return;
    bp_free_node(t->root);
    free(t);
}
/* Encontra a folha onde a chave deveria estar */
static BPNode* find_leaf(BPNode* root, long long k) {
    BPNode* c = root;
    while (!c->leaf) {
        int i = 0;
        while (i < c->nkeys && k >= c->keys[i]) i++;
        c = c->child[i];
    }
    return c;
}
/* Busca um livro pelo ISBN */
Book* bpt_search(BPTree* t, long long isbn) {
    if (!t || !t->root) return NULL;
    BPNode* leaf = find_leaf(t->root, isbn);
    for (int i = 0; i < leaf->nkeys; i++) {
        if (leaf->keys[i] == isbn) return leaf->vals[i];
    }
    return NULL;
}

/* insere em folha (sem split) */
static void leaf_insert_simple(BPNode* leaf, long long k, Book* v) {
    int i = leaf->nkeys - 1;
 /* abre espaço mantendo ordenado */
    while (i >= 0 && k < leaf->keys[i]) {
        leaf->keys[i + 1] = leaf->keys[i];
        leaf->vals[i + 1] = leaf->vals[i];
        i--;
    }    /* se já existir, apenas substitui*/
    if (i >= 0 && leaf->keys[i] == k) {
        leaf->vals[i] = v;
        return;
    }
    leaf->keys[i + 1] = k;
    leaf->vals[i + 1] = v;
    leaf->nkeys++;
}

/* divide uma folha em duas */
static BPNode* split_leaf(BPNode* leaf, long long* out_promote_key) {
    BPNode* newleaf = bp_new(1);

    int split = (BP_ORDER) / 2; 
    int move = leaf->nkeys - split;

    /*move metade das chaves para a nova folha*/
    for (int i = 0; i < move; i++) {
        newleaf->keys[i] = leaf->keys[split + i];
        newleaf->vals[i] = leaf->vals[split + i];
    }
    newleaf->nkeys = move;
    leaf->nkeys = split;

    newleaf->next = leaf->next;
    leaf->next = newleaf;

    *out_promote_key = newleaf->keys[0];
    return newleaf;
}

/* Divide um nó interno */
static BPNode* split_internal(BPNode* node, long long* out_promote_key) {
    BPNode* newn = bp_new(0);

    int mid = node->nkeys / 2; /* ex: 3 keys -> mid=1 */
    *out_promote_key = node->keys[mid];

    /* newn pega keys após mid */
    int j = 0;
    for (int i = mid + 1; i < node->nkeys; i++) {
        newn->keys[j++] = node->keys[i];
    }
    newn->nkeys = j;

    /* filhos correspondentes */
    j = 0;
    for (int i = mid + 1; i <= node->nkeys; i++) {
        newn->child[j++] = node->child[i];
    }

    node->nkeys = mid;
    return newn;
}

/* ---------- Pilha de pais ---------- */
typedef struct ParentStack {
    BPNode* node;
    int child_index;
    struct ParentStack* next;
} ParentStack;
/* Empilha pai */
static void ps_push(ParentStack** st, BPNode* node, int idx) {
    ParentStack* p = (ParentStack*)malloc(sizeof(ParentStack));
    if (!p) { printf("Erro: sem memória.\n"); exit(1); }
    p->node = node;
    p->child_index = idx;
    p->next = *st;
    *st = p;
}
/* Remove do topo */
static ParentStack* ps_pop(ParentStack** st) {
    if (!*st) return NULL;
    ParentStack* top = *st;
    *st = top->next;
    return top;
}
/* Libera a pilha */
static void ps_free(ParentStack* st) {
    while (st) {
        ParentStack* nx = st->next;
        free(st);
        st = nx;
    }
}
/* ---------- Inserção principal ---------- */
void bpt_insert(BPTree* t, long long isbn, Book* book) {
    BPNode* root = t->root;

    /* desce guardando pais */
    ParentStack* st = NULL;
    BPNode* c = root;
    while (!c->leaf) {
        int i = 0;
        while (i < c->nkeys && isbn >= c->keys[i]) i++;
        ps_push(&st, c, i);
        c = c->child[i];
    }

    /* insere na folha */
    leaf_insert_simple(c, isbn, book);

    /* se não estourou, ok */
    if (c->nkeys <= BP_ORDER - 1) {
        ps_free(st);
        return;
    }

    /* split folha */
    long long promote = 0;
    BPNode* newleaf = split_leaf(c, &promote);

    /* sobe promovendo */
    while (1) {
        ParentStack* p = ps_pop(&st);
/* se não tem pai, cria nova raiz */
        if (!p) {
          
            BPNode* newroot = bp_new(0);
            newroot->keys[0] = promote;
            newroot->child[0] = t->root;
            newroot->child[1] = newleaf;
            newroot->nkeys = 1;
            t->root = newroot;
            break;
        }

        BPNode* parent = p->node;
        int idx = p->child_index;
        free(p);

        /* abre espaço no pai */
        for (int j = parent->nkeys; j >= idx; j--) {
            parent->keys[j + 1] = parent->keys[j];
        }
        for (int j = parent->nkeys + 1; j >= idx + 1; j--) {
            parent->child[j + 1] = parent->child[j];
        }
        parent->keys[idx] = promote;
        parent->child[idx + 1] = newleaf;
        parent->nkeys++;
         /* se ainda couber, termina */
        if (parent->nkeys <= BP_ORDER - 1) {
            ps_free(st);
            return;
        }

        /* senão, divide nó interno */
        long long promote2 = 0;
        BPNode* newinternal = split_internal(parent, &promote2);
        promote = promote2;
        newleaf = newinternal; 
    }

    ps_free(st);
}

void bpt_print_range(BPTree* t, long long a, long long b) {
    if (!t || !t->root) return;
    if (a > b) { long long tmp = a; a = b; b = tmp; }

    BPNode* leaf = find_leaf(t->root, a);
    printf("\n---- ISBN no intervalo [%I64d, %I64d] ----\n", (long long)a, (long long)b);

    int printed = 0;
    while (leaf) {
        for (int i = 0; i < leaf->nkeys; i++) {
            long long k = leaf->keys[i];
            if (k < a) continue;
            if (k > b) {
                if (!printed) printf("(nenhum)\n");
                return;
            }
            Book* bk = leaf->vals[i];
            if (bk) {
                printf("%I64d | \"%s\" | %s | %d\n",
                       (long long)bk->isbn, bk->title, bk->author, bk->year);
                printed = 1;
            }
        }
        leaf = leaf->next;
    }
    if (!printed) printf("(nenhum)\n");
}
/* Cria a B+ Tree a partir de uma lista */
BPTree* bpt_build_from_list(BookNode* books) {
    BPTree* t = bpt_create();
    for (BookNode* cur = books; cur; cur = cur->next) {
        bpt_insert(t, cur->data.isbn, &cur->data);
    }
    return t;
}
