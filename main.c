#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "livros.h"
#include "usuarios.h"
#include "emprestimos.h"
#include "avl.h"
#include "hash_livros.h"
#include "busca_usuarios.h"
#include "top_livros.h"
#include "dsu.h"
#include "texto_busca.h"
#include "bptree.h"



static void trim_newline(char* s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static void read_line(const char* prompt, char* out, size_t out_sz) {
    printf("%s", prompt);
    if (!fgets(out, (int)out_sz, stdin)) {
        out[0] = '\0';
        return;
    }
    trim_newline(out);
}

static long long read_ll(const char* prompt) {
    char buf[64];
    while (1) {
        read_line(prompt, buf, sizeof(buf));
        if (buf[0] == '\0') continue;

        char* end = NULL;
        long long v = strtoll(buf, &end, 10);
        if (end && *end == '\0') return v;

        printf("Entrada inválida. Tente novamente.\n");
    }
}

static int read_int(const char* prompt) {
    char buf[64];
    while (1) {
        read_line(prompt, buf, sizeof(buf));
        if (buf[0] == '\0') continue;

        char* end = NULL;
        long v = strtol(buf, &end, 10);
        if (end && *end == '\0') return (int)v;

        printf("Entrada inválida. Tente novamente.\n");
    }
}


/* Converte user_id em índice do vetor ordenado */
static int index_of_user(User** arr, int n, int user_id) {
    User* u = users_binary_search(arr, n, user_id);
    if (!u) return -1;
    for (int i = 0; i < n; i++) {
        if (arr[i] == u) return i;
    }
    return -1;
}
/* Estrutura auxiliar para mapear primeiro usuário por ISBN */
typedef struct IsbnFirst {
    long long isbn;
    int first_idx;
    struct IsbnFirst* next;
} IsbnFirst;
/* Busca ISBN no mapa */
static IsbnFirst* map_find(IsbnFirst* m, long long isbn) {
    for (IsbnFirst* cur = m; cur; cur = cur->next) {
        if (cur->isbn == isbn) return cur;
    }
    return NULL;
}

static void map_free(IsbnFirst* m) {
    while (m) {
        IsbnFirst* nx = m->next;
        free(m);
        m = nx;
    }
}

/* Constrói o DSU a partir do histórico de empréstimos */
static int build_dsu_from_history(DSU* d, UserNode* users, LoanSystem* ls) {
    int n = 0;
    User** arr = users_build_sorted_array(users, &n);
    if (!arr || n == 0) {
        free(arr);
        return 0;
    }

    if (!dsu_init(d, n)) {
        free(arr);
        return 0;
    }

    IsbnFirst* map = NULL;

    /* Percorre histórico e conecta usuários que pegaram o mesmo livro */
    for (HistNode* h = ls->history; h; h = h->next) {
        if (h->type != ACT_BORROW && h->type != ACT_AUTO_BORROW) continue;

        int idx = index_of_user(arr, n, h->user_id);
        if (idx < 0) continue;

        IsbnFirst* e = map_find(map, h->isbn);
        if (!e) {
            e = (IsbnFirst*)malloc(sizeof(IsbnFirst));
            if (!e) break;
            e->isbn = h->isbn;
            e->first_idx = idx;
            e->next = map;
            map = e;
        } else {
            dsu_union(d, e->first_idx, idx);
        }
    }

    map_free(map);
    free(arr);
    return 1;
}


static void ui_add_book(BookNode** books, HashBooks* hb) {
    Book b;
    memset(&b, 0, sizeof(b));

    b.isbn = read_ll("ISBN (somente números): ");
    if (hb_get(hb, b.isbn)) {
        printf("Já existe livro com esse ISBN.\n");
        return;
    }

    read_line("Título: ", b.title, sizeof(b.title));
    read_line("Autor: ", b.author, sizeof(b.author));
    b.year = read_int("Ano: ");
    b.copies_total = read_int("Qtd de exemplares: ");
    if (b.copies_total < 0) b.copies_total = 0;

    b.copies_available = b.copies_total;
    b.times_borrowed = 0;

    books_push_front(books, &b);
    hb_insert(hb, &(*books)->data);

    printf("Livro cadastrado!\n");
}

static void ui_remove_book(BookNode** books, HashBooks* hb) {
    long long isbn = read_ll("ISBN para remover: ");

    if (books_remove(books, isbn)) {
        hb_remove(hb, isbn);
        printf("Removido.\n");
    } else {
        printf("Não encontrado.\n");
    }
}

static void ui_find_book_by_isbn_fast(HashBooks* hb) {
    long long isbn = read_ll("ISBN para buscar (HASH): ");

    Book* b = hb_get(hb, isbn);
    if (!b) {
        printf("Não encontrado.\n");
    } else {
        printf("Encontrado: ISBN %I64d | \"%s\" | %s | %d | Disp: %d/%d\n",
               (long long)b->isbn, b->title, b->author, b->year,
               b->copies_available, b->copies_total);
    }
}

/* Busca em Texto (título/autor) */
static void ui_text_search(BookNode* books, HashBooks* hb) {
    char q[64];
    read_line("Palavra para buscar (título/autor): ", q, sizeof(q));
    if (q[0] == '\0') return;

    TextIndex ti;
    if (!ti_init(&ti, 1009)) {
        printf("Erro ao criar índice de texto.\n");
        return;
    }
    ti_build(&ti, books);

    IsbnNode* hits = ti_find(&ti, q);
    if (!hits) {
        printf("Nenhum livro encontrado para \"%s\".\n", q);
        ti_free(&ti);
        return;
    }

    printf("\n---- RESULTADOS PARA \"%s\" ----\n", q);
    int count = 0;
    for (IsbnNode* cur = hits; cur; cur = cur->next) {
        Book* b = hb_get(hb, cur->isbn);
        if (!b) continue;
        printf("- %I64d | \"%s\" | %s | %d | emprest.: %d\n",
               (long long)b->isbn, b->title, b->author, b->year, b->times_borrowed);
        count++;
    }
    if (count == 0) printf("(Nenhum livro válido encontrado.)\n");

    ti_free(&ti);
}

/* AVL apenas para ordenação (AVL é ABB balanceada) */
static void ui_list_books_avl(BookNode* books) {
    AVLNode* root = avl_build_from_list(books);
    if (!root) {
        printf("Não há livros cadastrados.\n");
        return;
    }

    printf("\n---- LIVROS EM ORDEM ALFABÉTICA (AVL) ----\n");
    avl_print_inorder(root);
    avl_free(root);
}

/* B+ para range por ISBN */
static void ui_bptree_range(BookNode* books) {
    long long a = read_ll("ISBN início: ");
    long long b = read_ll("ISBN fim: ");

    BPTree* bp = bpt_build_from_list(books);
    bpt_print_range(bp, a, b);
    bpt_free(bp);
}

/* TOP livros (Fila de prioridade) */
static void ui_top_books(BookNode* books) {
    int k = read_int("Mostrar TOP quantos? ");
    if (k <= 0) return;

    BookHeap h;
    if (!heap_init(&h, 32)) {
        printf("Erro ao criar heap.\n");
        return;
    }

    heap_build_from_list(&h, books);
    heap_print_top(&h, k);
    heap_free(&h);
}

/* ---------- UI USUÁRIOS ---------- */

static void ui_add_user(UserNode** users) {
    User u;
    memset(&u, 0, sizeof(u));

    u.id = read_int("ID do usuário: ");
    if (users_find_by_id(*users, u.id)) {
        printf("Já existe usuário com esse ID.\n");
        return;
    }

    read_line("Nome: ", u.name, sizeof(u.name));
    read_line("Email: ", u.email, sizeof(u.email));

    users_push_front(users, &u);
    printf("Usuário cadastrado!\n");
}

static void ui_remove_user(UserNode** users) {
    int id = read_int("ID para remover: ");
    if (users_remove(users, id)) printf("Removido.\n");
    else printf("Não encontrado.\n");
}

static void ui_find_user(UserNode* users) {
    int id = read_int("ID para buscar (binária): ");

    int n = 0;
    User** arr = users_build_sorted_array(users, &n);
    if (!arr || n == 0) {
        printf("Não há usuários cadastrados.\n");
        free(arr);
        return;
    }

    User* u = users_binary_search(arr, n, id);
    if (!u) {
        printf("Não encontrado.\n");
    } else {
        printf("Encontrado: ID %d | Nome: %s | Email: %s\n", u->id, u->name, u->email);
    }

    free(arr);
}

/* ---------- UI EMPRÉSTIMOS ---------- */

static void ui_borrow(LoanSystem* ls, UserNode* users, BookNode* books) {
    int user_id = read_int("ID do usuário: ");
    long long isbn = read_ll("ISBN do livro: ");
    ls_borrow(ls, users, books, user_id, isbn);
}

static void ui_return(LoanSystem* ls, UserNode* users, BookNode* books) {
    int user_id = read_int("ID do usuário: ");
    long long isbn = read_ll("ISBN do livro: ");
    ls_return(ls, users, books, user_id, isbn);
}

/* ---------- UI DSU (Comunidades) ---------- */

static void ui_dsu_same(UserNode* users, LoanSystem* ls) {
    int a = read_int("User A (ID): ");
    int b = read_int("User B (ID): ");

    DSU d;
    if (!build_dsu_from_history(&d, users, ls)) {
        printf("Não foi possível construir DSU (sem usuários/histórico).\n");
        return;
    }

    int n = 0;
    User** arr = users_build_sorted_array(users, &n);
    if (!arr || n == 0) {
        free(arr);
        dsu_free(&d);
        printf("Sem usuários.\n");
        return;
    }

    int ia = index_of_user(arr, n, a);
    int ib = index_of_user(arr, n, b);

    if (ia < 0 || ib < 0) {
        printf("Um dos usuários não existe.\n");
    } else {
        printf("Conectados (mesma comunidade)? %s\n", dsu_same(&d, ia, ib) ? "SIM" : "NÃO");
    }

    free(arr);
    dsu_free(&d);
}

static void ui_dsu_size(UserNode* users, LoanSystem* ls) {
    int a = read_int("User (ID): ");

    DSU d;
    if (!build_dsu_from_history(&d, users, ls)) {
        printf("Não foi possível construir DSU (sem usuários/histórico).\n");
        return;
    }

    int n = 0;
    User** arr = users_build_sorted_array(users, &n);
    if (!arr || n == 0) {
        free(arr);
        dsu_free(&d);
        printf("Sem usuários.\n");
        return;
    }

    int ia = index_of_user(arr, n, a);
    if (ia < 0) {
        printf("Usuário não existe.\n");
    } else {
        printf("Tamanho da comunidade do usuário %d: %d\n", a, dsu_size(&d, ia));
    }

    free(arr);
    dsu_free(&d);
}

/* ---------- MENU ---------- */

static void show_menu(void) {
    printf("\n===== BIBLIOTECA =====\n");

    printf("\n-- LIVROS --\n");
    printf("1) Cadastrar livro\n");
    printf("2) Listar livros\n");
    printf("3) Buscar livro por ISBN (HASH)\n");
    printf("4) Buscar livros por titulo/autor (Busca em Texto)\n");
    printf("5) Listar livros em ordem alfabética (AVL = ABB balanceada)\n");
    printf("6) Listar livros por intervalo de ISBN (Árvore B+)\n");
    printf("7) TOP livros mais emprestados (HEAP)\n");
    printf("8) Remover livro\n");

    printf("\n-- USUÁRIOS --\n");
    printf("9) Cadastrar usuário\n");
    printf("10) Listar usuários\n");
    printf("11) Buscar usuário por ID (binária)\n");
    printf("12) Remover usuário\n");

    printf("\n-- EMPRÉSTIMOS --\n");
    printf("13) Solicitar empréstimo\n");
    printf("14) Devolver livro\n");
    printf("15) Listar empréstimos ativos\n");
    printf("16) Mostrar filas de espera\n");
    printf("17) Mostrar histórico (pilha)\n");

    printf("\n-- ARQUIVOS --\n");
    printf("18) Salvar (tudo)\n");

    printf("\n-- COMUNIDADES (DSU) --\n");
    printf("19) Verificar se 2 usuários estão conectados\n");
    printf("20) Tamanho da comunidade de um usuário\n");

    printf("\n0) Sair\n");
}

int main(void) {
    BookNode* books = books_load();
    UserNode* users = users_load();

    LoanSystem ls;
    ls_init(&ls);
    ls_load(&ls);

    HashBooks hb;
    if (!hb_init(&hb, 997)) {
        printf("Erro ao criar tabela hash.\n");
        return 1;
    }
    hb_build_from_list(&hb, books);

    printf("Arquivos carregados. (%s, %s, emprestimos.dat, filas.dat, historico.dat)\n",
           BOOKS_FILE, USERS_FILE);

    while (1) {
        show_menu();
        int op = read_int("Escolha: ");

        switch (op) {
            /* LIVROS */
            case 1: ui_add_book(&books, &hb); break;
            case 2: books_print(books); break;
            case 3: ui_find_book_by_isbn_fast(&hb); break;
            case 4: ui_text_search(books, &hb); break;
            case 5: ui_list_books_avl(books); break;
            case 6: ui_bptree_range(books); break;
            case 7: ui_top_books(books); break;
            case 8: ui_remove_book(&books, &hb); break;

            /* USUÁRIOS */
            case 9: ui_add_user(&users); break;
            case 10: users_print(users); break;
            case 11: ui_find_user(users); break;
            case 12: ui_remove_user(&users); break;

            /* EMPRÉSTIMOS */
            case 13: ui_borrow(&ls, users, books); break;
            case 14: ui_return(&ls, users, books); break;
            case 15: ls_print_loans(&ls); break;
            case 16: ls_print_waits(&ls); break;
            case 17: ls_print_history(&ls); break;

            /* ARQUIVOS */
            case 18:
                books_save(books);
                users_save(users);
                ls_save(&ls);
                break;

            /* DSU */
            case 19: ui_dsu_same(users, &ls); break;
            case 20: ui_dsu_size(users, &ls); break;

            case 0:
                books_save(books);
                users_save(users);
                ls_save(&ls);

                hb_free(&hb);
                ls_free(&ls);
                books_free(books);
                users_free(users);

                printf("Saindo.\n");
                return 0;

            default:
                printf("Opção inválida.\n");
        }
    }
}
