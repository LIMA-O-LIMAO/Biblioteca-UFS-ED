#include "texto_busca.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Função hash para palavras (algoritmo djb2) */
static unsigned int hash_word(const char* s) {
  
    unsigned int h = 5381u;
    for (; *s; s++) {
        h = ((h << 5) + h) + (unsigned char)(*s);
    }
    return h;
}
/* Insere um ISBN em uma lista ligada se ele ainda não existir */
static void isbn_list_add_unique(IsbnNode** head, long long isbn) {
      /* percorre a lista para verificar duplicatas */
    for (IsbnNode* cur = *head; cur; cur = cur->next) {
        if (cur->isbn == isbn) return;
    }
        /* cria novo nó */
    IsbnNode* n = (IsbnNode*)malloc(sizeof(IsbnNode));
    if (!n) { printf("Erro: sem memória.\n"); exit(1); }
    /* insere no início */
    n->isbn = isbn;
    n->next = *head;
    *head = n;
}
/* Normaliza uma palavra:
   - remove símbolos
   - transforma em minúsculas */
static void normalize_word(char* out, size_t out_sz, const char* in) {
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0' && j + 1 < out_sz; i++) {
        unsigned char c = (unsigned char)in[i];
        if (isalnum(c)) { /* aceita apenas letras e números */
            out[j++] = (char)tolower(c);
        }
    }
    out[j] = '\0';
}
/* Busca uma palavra na tabela hash.
   Se não existir, cria a entrada. */
static WordEntry* entry_get_or_create(TextIndex* ti, const char* word) {
    unsigned int h = hash_word(word);
    int idx = (int)(h % (unsigned int)ti->size);
    /* percorre a lista da posição da hash */
    for (WordEntry* e = ti->buckets[idx]; e; e = e->next) {
        if (strcmp(e->word, word) == 0) return e;
    }

    /* não achou → cria nova entrada */
    WordEntry* e = (WordEntry*)malloc(sizeof(WordEntry));
    if (!e) { printf("Erro: sem memória.\n"); exit(1); }
    strncpy(e->word, word, sizeof(e->word) - 1);
    e->word[sizeof(e->word) - 1] = '\0';
    e->isbns = NULL;
    /* encadeamento na tabela hash */
    e->next = ti->buckets[idx];
    ti->buckets[idx] = e;
    return e;
}
/* Indexa um texto (título ou autor) em palavras */
static void index_text(TextIndex* ti, const char* text, long long isbn) {
    /* quebra em tokens simples por espaço/pontuação */
    char buf[128];
    size_t len = strlen(text);
    size_t start = 0;

    while (start < len) {
        /* pula caracteres que não são letras/números */
        while (start < len && !isalnum((unsigned char)text[start])) start++;
        if (start >= len) break;
        /* encontra o fim da palavra */
        size_t end = start;
        while (end < len && isalnum((unsigned char)text[end])) end++;

        size_t toklen = end - start;
        if (toklen > 0) {
            if (toklen >= sizeof(buf)) toklen = sizeof(buf) - 1;
            memcpy(buf, text + start, toklen);
            buf[toklen] = '\0';

            char w[32];
            normalize_word(w, sizeof(w), buf);
            if (w[0] != '\0') {
                WordEntry* e = entry_get_or_create(ti, w);
                isbn_list_add_unique(&e->isbns, isbn);
            }
        }

        start = end + 1;
    }
}
/* Inicializa a tabela hash */
int ti_init(TextIndex* ti, int size) {
    ti->size = size;
    ti->buckets = (WordEntry**)calloc((size_t)size, sizeof(WordEntry*));
    return ti->buckets != NULL;
}
/* Libera toda a memória do índice */
void ti_free(TextIndex* ti) {
    if (!ti || !ti->buckets) return;

    for (int i = 0; i < ti->size; i++) {
        WordEntry* e = ti->buckets[i];
        while (e) {
            WordEntry* nx = e->next;

            IsbnNode* l = e->isbns;
            while (l) {
                IsbnNode* ln = l->next;
                free(l);
                l = ln;
            }

            free(e);
            e = nx;
        }
    }

    free(ti->buckets);
    ti->buckets = NULL;
    ti->size = 0;
}
/* Constrói o índice a partir da lista de livros */
void ti_build(TextIndex* ti, BookNode* books) {
    for (BookNode* cur = books; cur; cur = cur->next) {
        index_text(ti, cur->data.title, cur->data.isbn);
        index_text(ti, cur->data.author, cur->data.isbn);
    }
}
/* Busca uma palavra no índice e retorna a lista de ISBNs */
IsbnNode* ti_find(TextIndex* ti, const char* word_in) {
    if (!ti || !ti->buckets) return NULL;

    char w[32];
    normalize_word(w, sizeof(w), word_in);
    if (w[0] == '\0') return NULL;

    unsigned int h = hash_word(w);
    int idx = (int)(h % (unsigned int)ti->size);

    for (WordEntry* e = ti->buckets[idx]; e; e = e->next) {
        if (strcmp(e->word, w) == 0) return e->isbns;
    }
    return NULL;
}
