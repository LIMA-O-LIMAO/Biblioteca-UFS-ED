#include "emprestimos.h"
#include <stdio.h>
#include <stdlib.h>

#define LOANS_FILE   "emprestimos.dat"
#define WAITS_FILE   "filas.dat"
#define HISTORY_FILE "historico.dat"

/* ---------- Função segura de alocação ---------- */

static void* xmalloc(size_t sz) {
    void* p = malloc(sz);
    if (!p) {
        printf("Erro: sem memória.\n");
        exit(1);
    }
    return p;
}

/* ---------- HISTÓRICO (PILHA) ---------- */

static void hist_push(LoanSystem* ls, ActionType t, int user_id, long long isbn) {
    HistNode* h = (HistNode*)xmalloc(sizeof(HistNode));
    h->type = t;
    h->user_id = user_id;
    h->isbn = isbn;
    h->next = ls->history;
    ls->history = h;
}

/* Imprime o histórico (mais recente primeiro) */
void ls_print_history(LoanSystem* ls) {
    if (!ls->history) {
        printf("\n(Histórico vazio)\n");
        return;
    }

    printf("\n---- HISTÓRICO (TOPO = MAIS RECENTE) ----\n");
    for (HistNode* h = ls->history; h; h = h->next) {
        const char* label = "???";
        switch (h->type) {
            case ACT_BORROW:      label = "EMPRÉSTIMO"; break;
            case ACT_RETURN:      label = "DEVOLUÇÃO"; break;
            case ACT_ENQUEUE:     label = "FILA"; break;
            case ACT_AUTO_BORROW: label = "AUTO-EMPRÉSTIMO"; break;
            default:              label = "DESCONHECIDO"; break;
        }
        printf("%s | user_id=%d | isbn=%I64d\n", label, h->user_id, (long long)h->isbn);
    }
}

/* ---------- EMPRÉSTIMOS ATIVOS (LISTA) ---------- */

/* Procura um empréstimo específico */
static LoanNode* loan_find(LoanSystem* ls, int user_id, long long isbn) {
    for (LoanNode* cur = ls->loans; cur; cur = cur->next) {
        if (cur->user_id == user_id && cur->isbn == isbn) return cur;
    }
    return NULL;
}
/* Adiciona um empréstimo ativo */
static void loan_add(LoanSystem* ls, int user_id, long long isbn) {
    LoanNode* n = (LoanNode*)xmalloc(sizeof(LoanNode));
    n->user_id = user_id;
    n->isbn = isbn;
    n->next = ls->loans;
    ls->loans = n;
}

/* Remove um empréstimo ativo */
static int loan_remove(LoanSystem* ls, int user_id, long long isbn) {
    LoanNode* prev = NULL;
    LoanNode* cur = ls->loans;

    while (cur) {
        if (cur->user_id == user_id && cur->isbn == isbn) {
            if (prev) prev->next = cur->next;
            else ls->loans = cur->next;
            free(cur);
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}
/* Imprime os empréstimos ativos */
void ls_print_loans(LoanSystem* ls) {
    if (!ls->loans) {
        printf("\n(Nenhum empréstimo ativo)\n");
        return;
    }

    printf("\n---- EMPRÉSTIMOS ATIVOS ----\n");
    for (LoanNode* cur = ls->loans; cur; cur = cur->next) {
        printf("user_id=%d | isbn=%I64d\n", cur->user_id, (long long)cur->isbn);
    }
}

/* ---------- FILA DE ESPERA (FILA) ---------- */

/* Procura a fila de um livro */
static WaitList* waitlist_find(LoanSystem* ls, long long isbn) {
    for (WaitList* w = ls->waits; w; w = w->next) {
        if (w->isbn == isbn) return w;
    }
    return NULL;
}
/* Retorna a fila do livro (cria se não existir) */
static WaitList* waitlist_get_or_create(LoanSystem* ls, long long isbn) {
    WaitList* w = waitlist_find(ls, isbn);
    if (w) return w;

    w = (WaitList*)xmalloc(sizeof(WaitList));
    w->isbn = isbn;
    w->front = NULL;
    w->rear = NULL;
    w->next = ls->waits;
    ls->waits = w;
    return w;
}
/* Coloca usuário no final da fila */
static void wait_enqueue(LoanSystem* ls, long long isbn, int user_id) {
    WaitList* w = waitlist_get_or_create(ls, isbn);

    WaitNode* n = (WaitNode*)xmalloc(sizeof(WaitNode));
    n->user_id = user_id;
    n->next = NULL;

    if (!w->rear) {
        w->front = w->rear = n;
    } else {
        w->rear->next = n;
        w->rear = n;
    }
}
/* Remove o primeiro da fila */
static int wait_dequeue(LoanSystem* ls, long long isbn, int* out_user_id) {
    WaitList* w = waitlist_find(ls, isbn);
    if (!w || !w->front) return 0;

    WaitNode* n = w->front;
    *out_user_id = n->user_id;

    w->front = n->next;
    if (!w->front) w->rear = NULL;

    free(n);
    return 1;
}

/* Imprime todas as filas */
void ls_print_waits(LoanSystem* ls) {
    if (!ls->waits) {
        printf("\n(Nenhuma fila de espera)\n");
        return;
    }

    printf("\n---- FILAS DE ESPERA ----\n");
    for (WaitList* w = ls->waits; w; w = w->next) {
        printf("ISBN %I64d: ", (long long)w->isbn);
        if (!w->front) {
            printf("(vazia)\n");
            continue;
        }
        for (WaitNode* n = w->front; n; n = n->next) {
            printf("%d ", n->user_id);
        }
        printf("\n");
    }
}

/* ---------- API PRINCIPAL ---------- */

void ls_init(LoanSystem* ls) {
    ls->loans = NULL;
    ls->waits = NULL;
    ls->history = NULL;
}

void ls_free(LoanSystem* ls) {

    while (ls->loans) {
        LoanNode* next = ls->loans->next;
        free(ls->loans);
        ls->loans = next;
    }

    while (ls->waits) {
        WaitList* wn = ls->waits->next;

        while (ls->waits->front) {
            WaitNode* nx = ls->waits->front->next;
            free(ls->waits->front);
            ls->waits->front = nx;
        }
        free(ls->waits);
        ls->waits = wn;
    }

    while (ls->history) {
        HistNode* next = ls->history->next;
        free(ls->history);
        ls->history = next;
    }
}
/* Realiza um empréstimo */
void ls_borrow(LoanSystem* ls, UserNode* users, BookNode* books, int user_id, long long isbn) {
    if (!users_find_by_id(users, user_id)) {
        printf("Usuário não encontrado.\n");
        return;
    }

    BookNode* bn = books_find_by_isbn(books, isbn);
    if (!bn) {
        printf("Livro não encontrado.\n");
        return;
    }

    if (loan_find(ls, user_id, isbn)) {
        printf("Esse usuário já tem esse livro emprestado.\n");
        return;
    }
/*se tiver cópia, empresta*/
    if (bn->data.copies_available > 0) {
        bn->data.copies_available--;
        bn->data.times_borrowed++;
        loan_add(ls, user_id, isbn);
        hist_push(ls, ACT_BORROW, user_id, isbn);

        printf("Empréstimo realizado! user_id=%d | isbn=%I64d\n", user_id, (long long)isbn);
        return;
    }

    wait_enqueue(ls, isbn, user_id);
    hist_push(ls, ACT_ENQUEUE, user_id, isbn);
    printf("Sem exemplares disponíveis. Usuário entrou na fila. (isbn=%I64d)\n", (long long)isbn);
}
/* Realiza uma devolução */
void ls_return(LoanSystem* ls, UserNode* users, BookNode* books, int user_id, long long isbn) {
    BookNode* bn = books_find_by_isbn(books, isbn);
    if (!bn) {
        printf("Livro não encontrado.\n");
        return;
    }

    if (!loan_remove(ls, user_id, isbn)) {
        printf("Empréstimo ativo não encontrado para esse usuário e ISBN.\n");
        return;
    }

    bn->data.copies_available++;
    hist_push(ls, ACT_RETURN, user_id, isbn);

    printf("Devolução realizada! user_id=%d | isbn=%I64d\n", user_id, (long long)isbn);

    /* se tiver fila, empresta automaticamente */
    int next_user = 0;
    if (bn->data.copies_available > 0 && wait_dequeue(ls, isbn, &next_user)) {
        if (users_find_by_id(users, next_user)) {
            bn->data.copies_available--;
            bn->data.times_borrowed++;
            loan_add(ls, next_user, isbn);
            hist_push(ls, ACT_AUTO_BORROW, next_user, isbn);

            printf("Auto-empréstimo para o próximo da fila: user_id=%d | isbn=%I64d\n",
                   next_user, (long long)isbn);
        } else {
            printf("Aviso: próximo da fila (user_id=%d) não existe mais. Ignorado.\n", next_user);
        }
    }
}

/* ---------- PERSISTÊNCIA ---------- */

void ls_save(LoanSystem* ls) {
  /* salvar empréstimos */
    FILE* f = fopen(LOANS_FILE, "wb");
    if (!f) {
        printf("Erro ao abrir %s para escrita.\n", LOANS_FILE);
    } else {
        for (LoanNode* cur = ls->loans; cur; cur = cur->next) {
            fwrite(&cur->user_id, sizeof(int), 1, f);
            fwrite(&cur->isbn, sizeof(long long), 1, f);
        }
        fclose(f);
    }

   /* salvar filas */
    f = fopen(WAITS_FILE, "wb");
    if (!f) {
        printf("Erro ao abrir %s para escrita.\n", WAITS_FILE);
    } else {
        for (WaitList* w = ls->waits; w; w = w->next) {
            fwrite(&w->isbn, sizeof(long long), 1, f);

            int count = 0;
            for (WaitNode* n = w->front; n; n = n->next) count++;
            fwrite(&count, sizeof(int), 1, f);

            for (WaitNode* n = w->front; n; n = n->next) {
                fwrite(&n->user_id, sizeof(int), 1, f);
            }
        }
        fclose(f);
    }

    /* salvar histórico */
    f = fopen(HISTORY_FILE, "wb");
    if (!f) {
        printf("Erro ao abrir %s para escrita.\n", HISTORY_FILE);
    } else {
        for (HistNode* h = ls->history; h; h = h->next) {
            int type = (int)h->type;
            fwrite(&type, sizeof(int), 1, f);
            fwrite(&h->user_id, sizeof(int), 1, f);
            fwrite(&h->isbn, sizeof(long long), 1, f);
        }
        fclose(f);
    }

    printf("Empréstimos (empréstimos/filas/histórico) salvos.\n");
}

void ls_load(LoanSystem* ls) {
    /* carregar empréstimos */
    FILE* f = fopen(LOANS_FILE, "rb");
    if (f) {
        int user_id;
        long long isbn;

        while (fread(&user_id, sizeof(int), 1, f) == 1 &&
               fread(&isbn, sizeof(long long), 1, f) == 1) {
            loan_add(ls, user_id, isbn);
        }
        fclose(f);
    }

        /* carregar filas */
    f = fopen(WAITS_FILE, "rb");
    if (f) {
        long long isbn;
        int count;

        while (fread(&isbn, sizeof(long long), 1, f) == 1 &&
               fread(&count, sizeof(int), 1, f) == 1) {
            for (int i = 0; i < count; i++) {
                int user_id;
                if (fread(&user_id, sizeof(int), 1, f) != 1) break;
                wait_enqueue(ls, isbn, user_id);
            }
        }
        fclose(f);
    }

       /* carregar histórico mantendo ordem */
    f = fopen(HISTORY_FILE, "rb");
    if (f) {
        typedef struct {
            int type;
            int user_id;
            long long isbn;
        } HistRec;

        HistRec* arr = NULL;
        size_t used = 0, cap = 0;

        while (1) {
            HistRec r;
            if (fread(&r.type, sizeof(int), 1, f) != 1) break;
            if (fread(&r.user_id, sizeof(int), 1, f) != 1) break;
            if (fread(&r.isbn, sizeof(long long), 1, f) != 1) break;

            if (used == cap) {
                size_t newcap = (cap == 0) ? 16 : cap * 2;
                HistRec* tmp = (HistRec*)realloc(arr, newcap * sizeof(HistRec));
                if (!tmp) {
                    free(arr);
                    arr = NULL;
                    used = cap = 0;
                    printf("Aviso: sem memória ao carregar histórico.\n");
                    break;
                }
                arr = tmp;
                cap = newcap;
            }
            arr[used++] = r;
        }
        fclose(f);

        
        for (size_t i = used; i > 0; i--) {
            HistRec r = arr[i - 1];
            hist_push(ls, (ActionType)r.type, r.user_id, r.isbn);
        }
        free(arr);
    }

    printf("Empréstimos (empréstimos/filas/histórico) carregados.\n");
}
