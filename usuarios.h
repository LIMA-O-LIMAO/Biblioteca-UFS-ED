#ifndef USUARIOS_H
#define USUARIOS_H

#include <stdio.h>

#define USERS_FILE "usuarios.dat"

typedef struct {
    int id;
    char name[80];
    char email[80];
} User;
/* Nó da lista encadeada de usuários */
typedef struct UserNode {
    User data;
    struct UserNode* next; /* ponteiro para o próximo nó */
} UserNode;

/* Lista */
UserNode* users_find_by_id(UserNode* head, int id);
void users_push_front(UserNode** head, const User* u); /* Insere um novo usuário no início da lista */ 
int  users_remove(UserNode** head, int id); /* Remove um usuário da lista pelo ID */
void users_print(UserNode* head); /* Imprime todos os usuários da lista */
void users_free(UserNode* head); /* Libera toda a memória da lista de usuários */

/* Salva todos os usuários no arquivo binário */
void users_save(UserNode* head);
UserNode* users_load(void); /* Carrega os usuários do arquivo binário e cria a lista */

#endif
