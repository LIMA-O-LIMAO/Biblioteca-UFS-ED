#include "livros.h"
#include <stdlib.h>
#include <string.h>

/* cria um novo nó de livro e copia os dados */
static BookNode* booknode_new(const Book* b) {
    BookNode* n = (BookNode*)malloc(sizeof(BookNode));
    if (!n) {
        printf("Erro: sem memória.\n");
        exit(1);
    }
    n->data = *b; /* copia os dados do livro */
    n->next = NULL;
    return n;
}

/* ---------- Lista ---------- */


/* Procura um livro pelo ISBN na lista */
BookNode* books_find_by_isbn(BookNode* head, long long isbn) {
    for (BookNode* cur = head; cur; cur = cur->next) {
        if (cur->data.isbn == isbn) return cur;
    }
    return NULL;
}
/* Insere um livro no início da lista */
void books_push_front(BookNode** head, const Book* b) {
    BookNode* n = booknode_new(b);/*cria o nó*/
    n->next = *head;/*aponta para o antigo primeiro*/
    *head = n;/*atualiza a cabeça da lista*/
}

/* Remove um livro da lista usando o ISBN */
int books_remove(BookNode** head, long long isbn) {
    BookNode* prev = NULL;
    BookNode* cur = *head;

    while (cur) {
        if (cur->data.isbn == isbn) {
            if (prev) prev->next = cur->next;
            else *head = cur->next;
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}
/* Mostra todos os livros cadastrados */
void books_print(BookNode* head) {
    if (!head) {
        printf("\n(Nenhum livro cadastrado)\n");
        return;
    }

    printf("\n---- LISTA DE LIVROS ----\n");
    for (BookNode* cur = head; cur; cur = cur->next) {
        Book* b = &cur->data;

        printf("ISBN: %I64d | \"%s\" | Autor: %s | Ano: %d | Disp: %d/%d | Emprest.: %d\n",
               b->isbn, b->title, b->author, b->year,
               b->copies_available, b->copies_total, b->times_borrowed);
    }
}

void books_free(BookNode* head) {
    while (head) {
        BookNode* next = head->next;
        free(head);
        head = next;
    }
}

/* ---------- Arquivo ---------- */
/* Salva todos os livros em um arquivo binário */
void books_save(BookNode* head) {
    FILE* f = fopen(BOOKS_FILE, "wb");
    if (!f) {
        printf("Erro ao abrir %s para escrita.\n", BOOKS_FILE);
        return;
    }
    for (BookNode* cur = head; cur; cur = cur->next) {
        fwrite(&cur->data, sizeof(Book), 1, f);
    }
    fclose(f);
    printf("Livros salvos em %s.\n", BOOKS_FILE);
}
/* Lê os livros do arquivo e recria a lista */
BookNode* books_load(void) {
    FILE* f = fopen(BOOKS_FILE, "rb");
    if (!f) return NULL;

    BookNode* head = NULL;
    Book tmp;

    while (fread(&tmp, sizeof(Book), 1, f) == 1) {
        if (!books_find_by_isbn(head, tmp.isbn)) {
            books_push_front(&head, &tmp);
        }
    }
    fclose(f);
    return head;
}
