#ifndef LIVROS_H
#define LIVROS_H

#include <stdio.h>

#define BOOKS_FILE "livros.dat"

typedef struct {
    long long isbn;
    char title[120];
    char author[80];
    int year;
    int copies_total;
    int copies_available;
    int times_borrowed;
} Book;
/* Nó da lista ligada de livros */
typedef struct BookNode {
    Book data;
    struct BookNode* next;
} BookNode;

/* Lista */
BookNode* books_find_by_isbn(BookNode* head, long long isbn);
void books_push_front(BookNode** head, const Book* b); /* Insere um livro no início da lista */
int  books_remove(BookNode** head, long long isbn); /* Remove um livro da lista usando o ISBN */
void books_print(BookNode* head);/* Mostra todos os livros na tela */
void books_free(BookNode* head);/* Libera toda a memória da lista */

/* Arquivo */
/* Salva os livros no arquivo binário */
void books_save(BookNode* head);
BookNode* books_load(void);

#endif
