#ifndef AVL_H
#define AVL_H

#include "livros.h"

typedef struct AVLNode {
    Book* book;
    int height;
    struct AVLNode* left;
    struct AVLNode* right;
} AVLNode;

/* Construção */
AVLNode* avl_insert(AVLNode* root, Book* book);

/* Busca por título */
Book* avl_search(AVLNode* root, const char* title);

/* Listagem ordenada */
void avl_print_inorder(AVLNode* root);

/* Liberação de memória  */
void avl_free(AVLNode* root);

/* Construir AVL a partir da lista de livros */
AVLNode* avl_build_from_list(BookNode* head);

#endif
