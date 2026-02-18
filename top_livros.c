#include "top_livros.h"
#include <stdlib.h>
#include <stdio.h>

/* prioridade: mais emprestado (times_borrowed maior) */
static int higher(Book* a, Book* b) {
    if (a->times_borrowed != b->times_borrowed)
        return a->times_borrowed > b->times_borrowed;

    /* desempate determinístico: ISBN menor primeiro */
    return a->isbn < b->isbn;
}
/* Troca dois ponteiros de livros */
static void swap(Book** a, Book** b) {
    Book* t = *a;
    *a = *b;
    *b = t;
}
/* Sobe o elemento na heap até manter a propriedade */
static void sift_up(BookHeap* h, int i) {
    while (i > 0) {
        int p = (i - 1) / 2;
        if (higher(h->data[p], h->data[i])) break;
        swap(&h->data[p], &h->data[i]);
        i = p;
    }
}
/* Desce o elemento na heap até manter a propriedade */
static void sift_down(BookHeap* h, int i) {
    while (1) {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        int best = i;

        if (l < h->size && higher(h->data[l], h->data[best])) best = l;
        if (r < h->size && higher(h->data[r], h->data[best])) best = r;

        if (best == i) break;
        swap(&h->data[i], &h->data[best]);
        i = best;
    }
}
/* Inicializa o heap */
int heap_init(BookHeap* h, int cap) {
    h->size = 0;
    h->cap = cap;
    h->data = (Book**)malloc(sizeof(Book*) * (size_t)cap);
    if (!h->data) return 0;
    return 1;
}

/* Libera memória do heap */
void heap_free(BookHeap* h) {
    if (!h) return;
    free(h->data);
    h->data = NULL;
    h->size = 0;
    h->cap = 0;
}
/* Garante espaço no vetor (realoca se necessário) */
static int heap_ensure(BookHeap* h) {
    if (h->size < h->cap) return 1;
    int newcap = (h->cap == 0) ? 16 : h->cap * 2;
    Book** tmp = (Book**)realloc(h->data, sizeof(Book*) * (size_t)newcap);
    if (!tmp) return 0;
    h->data = tmp;
    h->cap = newcap;
    return 1;
}
/* Insere um livro no heap */
int heap_push(BookHeap* h, Book* b) {
    if (!heap_ensure(h)) return 0;
    h->data[h->size] = b;
    sift_up(h, h->size);
    h->size++;
    return 1;
}
/* Retorna o topo sem remover */
Book* heap_peek(BookHeap* h) {
    if (!h || h->size == 0) return NULL;
    return h->data[0];
}
/* Remove e retorna o livro com maior prioridade */
Book* heap_pop(BookHeap* h) {
    if (!h || h->size == 0) return NULL;
    Book* top = h->data[0];
    h->size--;
    if (h->size > 0) {
        h->data[0] = h->data[h->size];
        sift_down(h, 0);
    }
    return top;
}
/* Constrói heap a partir da lista de livros */
int heap_build_from_list(BookHeap* h, BookNode* head) {
    h->size = 0;
    for (BookNode* cur = head; cur; cur = cur->next) {
        if (!heap_push(h, &cur->data)) return 0;
    }
    return 1;
}

/* Imprime o top K sem destruir o heap original (faz cópia) */
void heap_print_top(BookHeap* h, int k) {
    if (!h || h->size == 0) {
        printf("\n(Nenhum livro no heap)\n");
        return;
    }
    if (k <= 0) return;
    if (k > h->size) k = h->size;
    /* Cria cópia do heap */
    BookHeap copy = {0};
    copy.cap = h->size;
    copy.size = h->size;
    copy.data = (Book**)malloc(sizeof(Book*) * (size_t)copy.cap);
    if (!copy.data) {
        printf("Erro: sem memória.\n");
        return;
    }
    for (int i = 0; i < h->size; i++) copy.data[i] = h->data[i];

    printf("\n---- TOP %d LIVROS (MAIS EMPRESTADOS) ----\n", k);
    for (int i = 1; i <= k; i++) {
        Book* b = heap_pop(&copy);
        if (!b) break;
        printf("%2d) %I64d | \"%s\" | emprest.: %d | disp: %d/%d\n",
               i, (long long)b->isbn, b->title,
               b->times_borrowed, b->copies_available, b->copies_total);
    }

    heap_free(&copy);
}
