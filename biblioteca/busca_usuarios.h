#ifndef BUSCA_USUARIOS_H
#define BUSCA_USUARIOS_H

#include "usuarios.h"

User** users_build_sorted_array(UserNode* head, int* out_n);
User*  users_binary_search(User** arr, int n, int id);

#endif