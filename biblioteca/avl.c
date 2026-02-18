#include "avl.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---------- Funções auxiliares ---------- */


static int max_int(int a, int b) { return (a > b) ? a : b; }/* Retorna o maior valor entre dois números */

/* Retorna a altura de um nó */
static int height(AVLNode* n) {
    return n ? n->height : 0;
}

/* Cria um novo nó da árvore */
static AVLNode* node_new(Book* book) {
    AVLNode* n = (AVLNode*)malloc(sizeof(AVLNode));
    if (!n) {
        printf("Erro: sem memória.\n");
        exit(1);
    }
    n->book = book;
    n->height = 1; /* folha */
    n->left = NULL;
    n->right = NULL;
    return n;
}

static int balance_factor(AVLNode* n) {
    if (!n) return 0;
    return height(n->left) - height(n->right);
}

/* Rotação para a direita */
static AVLNode* rotate_right(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = 1 + max_int(height(y->left), height(y->right));
    x->height = 1 + max_int(height(x->left), height(x->right));

    return x;
}

/* Rotação para a esquerda */
static AVLNode* rotate_left(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = 1 + max_int(height(x->left), height(x->right));
    y->height = 1 + max_int(height(y->left), height(y->right));

    return y;
}

/* ---------- Funções principais ---------- */

/* Insere um livro na árvore AVL */
AVLNode* avl_insert(AVLNode* root, Book* book) {
    if (!root) return node_new(book);// cria o primeiro nó

    int cmp = strcmp(book->title, root->book->title);

    if (cmp < 0) {
        root->left = avl_insert(root->left, book);
    } else if (cmp > 0) {
        root->right = avl_insert(root->right, book);
    } else {
        /* Título repetido: não insere */
        return root;
    }

    /* atualiza altura */
    root->height = 1 + max_int(height(root->left), height(root->right));

    /* Verifica se a árvore ficou desbalanceada */
    int bf = balance_factor(root);

    /* Caso esquerda-esquerda */
    if (bf > 1 && strcmp(book->title, root->left->book->title) < 0)
        return rotate_right(root);

    /* Caso direita-direita */
    if (bf < -1 && strcmp(book->title, root->right->book->title) > 0)
        return rotate_left(root);

    /* Caso esquerda-direita */
    if (bf > 1 && strcmp(book->title, root->left->book->title) > 0) {
        root->left = rotate_left(root->left);
        return rotate_right(root);
    }

    /* Caso direita-esquerda */
    if (bf < -1 && strcmp(book->title, root->right->book->title) < 0) {
        root->right = rotate_right(root->right);
        return rotate_left(root);
    }

    return root;
}
/* Busca um livro pelo título */
Book* avl_search(AVLNode* root, const char* title) {
    if (!root) return NULL;

    int cmp = strcmp(title, root->book->title);
    if (cmp == 0) return root->book;
    if (cmp < 0) return avl_search(root->left, title);
    return avl_search(root->right, title);
}
/* Imprime os livros em ordem alfabética */
void avl_print_inorder(AVLNode* root) {
    if (!root) return;

    avl_print_inorder(root->left);

    Book* b = root->book;
    printf("ISBN %I64d | \"%s\" | %s | %d\n",
           (long long)b->isbn, b->title, b->author, b->year);

    avl_print_inorder(root->right);
}
/* Libera toda a árvore da memória */
void avl_free(AVLNode* root) {
    if (!root) return;
    avl_free(root->left);
    avl_free(root->right);
    free(root);
}
/* Constrói a AVL a partir de uma lista encadeada */
AVLNode* avl_build_from_list(BookNode* head) {
    AVLNode* root = NULL;
    for (BookNode* cur = head; cur; cur = cur->next) {
        root = avl_insert(root, &cur->data);
    }
    return root;
}
