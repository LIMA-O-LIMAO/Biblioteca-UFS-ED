#include "busca_usuarios.h"
#include <stdlib.h>


/* Troca dois ponteiros de lugar */
static void swap(User** a, User** b) {
    User* tmp = *a;
    *a = *b;
    *b = tmp;
}

/* Divide o vetor em duas partes para o quicksort */
static int partition(User** arr, int lo, int hi) {
    int pivot = arr[hi]->id;
    int i = lo - 1;

    for (int j = lo; j < hi; j++) {
        if (arr[j]->id <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[hi]);
    return i + 1;
}
/* Ordena o vetor de usuários pelo ID */
static void quicksort(User** arr, int lo, int hi) {
    if (lo < hi) {
        int p = partition(arr, lo, hi);
        quicksort(arr, lo, p - 1);
        quicksort(arr, p + 1, hi);
    }
}
/* Cria um vetor ordenado de usuários a partir da lista */
User** users_build_sorted_array(UserNode* head, int* out_n) {
    int n = 0;
    /* conta quantos usuários existem */
    for (UserNode* cur = head; cur; cur = cur->next) n++;

    *out_n = n;
    if (n == 0) return NULL;

    User** arr = (User**)malloc(sizeof(User*) * (size_t)n);
    if (!arr) return NULL;

    int i = 0;
    for (UserNode* cur = head; cur; cur = cur->next) {
        arr[i++] = &cur->data;
    }

    quicksort(arr, 0, n - 1);
    return arr;
}
/* Busca um usuário pelo ID usando busca binária */
User* users_binary_search(User** arr, int n, int id) {
    int lo = 0, hi = n - 1;

    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int v = arr[mid]->id;

        if (v == id) return arr[mid];
        if (id < v) hi = mid - 1;
        else lo = mid + 1;
    }
    return NULL;
}
