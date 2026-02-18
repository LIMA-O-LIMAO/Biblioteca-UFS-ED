# 📚 Sistema de Biblioteca em C

Projeto acadêmico desenvolvido em C com foco em **Estruturas de Dados**, implementando múltiplas técnicas clássicas sem uso de bibliotecas externas.

---

# 🎯 Objetivo

Demonstrar na prática o uso de:

* Listas encadeadas
* Pilhas
* Filas
* Busca Linear
* Busca Binária
* Tabela Hash
* Árvore AVL (ABB balanceada)
* Heap (Fila de Prioridade)
* Conjuntos Disjuntos (DSU / Union-Find)
* Árvore B+
* Estrutura de busca em texto (índice invertido)
* Arquivos sequenciais

---

# 🏗 Arquitetura do Sistema

O sistema é modularizado em múltiplos arquivos `.h` e `.c`, cada um responsável por uma estrutura de dados específica.

## 📂 Principais Módulos

| Módulo           | Responsabilidade                           |
| ---------------- | ------------------------------------------ |
| livros.c         | Lista encadeada de livros                  |
| usuarios.c       | Lista encadeada de usuários                |
| emprestimos.c    | Controle de empréstimos, filas e histórico |
| hash_livros.c    | Busca rápida por ISBN (Tabela Hash)        |
| busca_usuarios.c | Busca binária por ID                       |
| avl.c            | Ordenação de livros por título             |
| top_livros.c     | Heap para ranking de livros                |
| dsu.c            | Conjuntos Disjuntos (comunidades)          |
| texto_busca.c    | Índice invertido para busca textual        |
| bptree.c         | Árvore B+ para busca por intervalo de ISBN |

---

# 🧠 Estruturas de Dados Utilizadas

## 🔹 1. Lista Encadeada

Usada para armazenar:

* Livros
* Usuários

Complexidade média:

* Inserção: O(1)
* Busca: O(n)

---

## 🔹 2. Tabela Hash

Utilizada para busca rápida de livros por ISBN.

Complexidade média:

* Busca: O(1)

---

## 🔹 3. Busca Binária

Aplicada sobre vetor ordenado de usuários por ID.

Complexidade:

* O(log n)

---

## 🔹 4. Árvore AVL

Usada para listar livros em ordem alfabética.

Propriedades:

* ABB balanceada
* Altura O(log n)

---

## 🔹 5. Heap (Fila de Prioridade)

Usado para exibir os livros mais emprestados.

Complexidade:

* Inserção: O(log n)
* Remoção: O(log n)

---

## 🔹 6. Pilha

Utilizada para armazenar histórico de empréstimos.

Estrutura LIFO.

---

## 🔹 7. Fila

Utilizada para fila de espera de livros.

Estrutura FIFO.

---

## 🔹 8. DSU (Conjuntos Disjuntos)

Permite identificar comunidades de usuários com base em livros compartilhados.

Operações:

* union
* find
* same
* size

Complexidade quase constante (com path compression).

---

## 🔹 9. Árvore B+

Usada para listar livros por intervalo de ISBN.

Características:

* Multi-chaves por nó
* Balanceada
* Dados armazenados apenas nas folhas
* Folhas encadeadas

Ideal para consultas por intervalo.

---

## 🔹 10. Busca em Texto (Índice Invertido)

Permite buscar livros por palavras no título ou autor.

Estrutura baseada em hash + listas encadeadas.

---

# 💾 Persistência em Arquivos

Arquivos utilizados:

* books.dat
* users.dat
* emprestimos.dat
* filas.dat
* historico.dat

Modelo de armazenamento:

* Arquivos sequenciais
* Leitura na inicialização
* Salvamento manual ou ao sair

---

# 📋 Funcionalidades do Sistema

## 📚 Livros

* Cadastrar livro
* Listar livros
* Buscar por ISBN (Hash)
* Buscar por palavra (Busca textual)
* Listar em ordem alfabética (AVL)
* Listar por intervalo de ISBN (Árvore B+)
* Ranking de mais emprestados (Heap)
* Remover livro

## 👤 Usuários

* Cadastrar
* Listar
* Buscar por ID (Binária)
* Remover

## 📖 Empréstimos

* Solicitar empréstimo
* Devolver livro
* Listar empréstimos ativos
* Ver filas de espera
* Ver histórico

## 🌐 Comunidades

* Verificar se usuários estão conectados
* Tamanho da comunidade

---

# ⚙ Compilação

```bash
gcc -Wall -Wextra -O2 main.c livros.c usuarios.c busca_usuarios.c emprestimos.c avl.c hash_livros.c top_livros.c dsu.c texto_busca.c bptree.c -o biblioteca.exe
```

---

# 🎓 Objetivo Acadêmico

Este projeto cobre praticamente todo o conteúdo típico de:

* Estrutura de Dados

Demonstrando aplicação prática de múltiplas estruturas no mesmo sistema integrado.

---

# 👨‍💻 Autor

NOMES.

---

# 🚀 Status

✔ Funcional ✔ Modularizado ✔ Sem uso de bibliotecas externas ✔ Estruturas implementadas manualmente
