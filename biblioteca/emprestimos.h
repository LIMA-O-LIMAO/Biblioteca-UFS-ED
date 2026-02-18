#ifndef EMPRESTIMOS_H
#define EMPRESTIMOS_H

#include "livros.h"
#include "usuarios.h"

/* Ação para histórico (PILHA) */
typedef enum {
    ACT_BORROW = 1,      // empréstimo realizado
    ACT_RETURN = 2,      // devolução
    ACT_ENQUEUE = 3,     // entrou na fila de espera
    ACT_AUTO_BORROW = 4  // empréstimo automático (quando alguém devolve e chama o próximo da fila)
} ActionType;

/* Nó de empréstimo ativo (LISTA) */
typedef struct LoanNode {
    int user_id;
    long long isbn;
    struct LoanNode* next;
} LoanNode;

/* Nó da fila de espera (FILA) */
typedef struct WaitNode {
    int user_id;
    struct WaitNode* next;
} WaitNode;

/* Uma fila por ISBN (mapeamento via lista) */
typedef struct WaitList {
    long long isbn;
    WaitNode* front;
    WaitNode* rear;
    struct WaitList* next;
} WaitList;

/* Histórico (PILHA) */
typedef struct HistNode {
    ActionType type;
    int user_id;
    long long isbn;
    struct HistNode* next;
} HistNode;

/* Sistema de empréstimos */
typedef struct {
    LoanNode* loans;     // lista de empréstimos ativos
    WaitList* waits;     // várias filas, uma por ISBN
    HistNode* history;   // pilha de histórico
} LoanSystem;

/* Lifecycle */
void ls_init(LoanSystem* ls);
void ls_free(LoanSystem* ls);

/* Operações */
void ls_borrow(LoanSystem* ls, UserNode* users, BookNode* books, int user_id, long long isbn);
void ls_return(LoanSystem* ls, UserNode* users, BookNode* books, int user_id, long long isbn);

/* Relatórios */
void ls_print_loans(LoanSystem* ls);
void ls_print_history(LoanSystem* ls);
void ls_print_waits(LoanSystem* ls);

/* Persistência */
void ls_save(LoanSystem* ls);
void ls_load(LoanSystem* ls);

#endif
