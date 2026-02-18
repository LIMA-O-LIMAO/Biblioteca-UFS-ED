#include "usuarios.h"
#include <stdlib.h>
#include <string.h>

/* ---------- internos ---------- */

/* Cria um novo nó da lista a partir de um User */
static UserNode* usernode_new(const User* u) {
    UserNode* n = (UserNode*)malloc(sizeof(UserNode)); /* aloca memória */
    if (!n) {
        printf("Erro: sem memória.\n");
        exit(1);
    }
    n->data = *u; /* copia os dados do usuário */
    n->next = NULL;
    return n;
}

/* ---------- Lista ---------- */

/* Procura um usuário pelo ID (busca linear) */
UserNode* users_find_by_id(UserNode* head, int id) {
    for (UserNode* cur = head; cur; cur = cur->next) {
        if (cur->data.id == id) return cur;
    }
    return NULL;
}

/* Insere um usuário no início da lista */
void users_push_front(UserNode** head, const User* u) {
    UserNode* n = usernode_new(u);
    n->next = *head; /* novo nó aponta para o antigo primeiro */
    *head = n; /* cabeça passa a ser o novo nó */
}
/* Remove um usuário pelo ID */
int users_remove(UserNode** head, int id) {
    UserNode* prev = NULL;
    UserNode* cur = *head;

    while (cur) {
        if (cur->data.id == id) {
            if (prev) prev->next = cur->next; /* remove do meio/fim */
            else *head = cur->next;           /* remove da cabeça */
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

/* Imprime todos os usuários */
void users_print(UserNode* head) {
    if (!head) {
        printf("\n(Nenhum usuário cadastrado)\n");
        return;
    }

    printf("\n---- LISTA DE USUÁRIOS ----\n");
    for (UserNode* cur = head; cur; cur = cur->next) {
        User* u = &cur->data;
        printf("ID: %d | Nome: %s | Email: %s\n", u->id, u->name, u->email);
    }
}

void users_free(UserNode* head) {
    while (head) {
        UserNode* next = head->next;
        free(head);
        head = next;
    }
}

/* ---------- Arquivo ---------- */
/* Salva todos os usuários em arquivo binário */
void users_save(UserNode* head) {
    FILE* f = fopen(USERS_FILE, "wb");
    if (!f) {
        printf("Erro ao abrir %s para escrita.\n", USERS_FILE);
        return;
    }
    for (UserNode* cur = head; cur; cur = cur->next) {
        fwrite(&cur->data, sizeof(User), 1, f);
    }
    fclose(f);
    printf("Usuários salvos em %s.\n", USERS_FILE);
}
/* Carrega os usuários do arquivo binário */
UserNode* users_load(void) {
    FILE* f = fopen(USERS_FILE, "rb");
    if (!f) return NULL;

    UserNode* head = NULL;
    User tmp;

    while (fread(&tmp, sizeof(User), 1, f) == 1) {
        if (!users_find_by_id(head, tmp.id)) {
            users_push_front(&head, &tmp);
        }
    }
    fclose(f);
    return head;
}
