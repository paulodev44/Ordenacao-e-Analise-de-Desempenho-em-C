/*
 * Implementação dos algoritmos de ordenação (Bubble, Selection, Insertion).
 * Cada função incrementa os contadores na estrutura Metrics.
 */

#include "sorts.h"
#include <stddef.h>

/**
 * @brief Reseta os contadores de métricas.
 */
void reset_metrics(Metrics *m) {
    if (m) {
        m->steps_cmp = 0;
        m->steps_swap = 0;
    }
}

/**
 * @brief Troca dois inteiros de lugar.
 * Helper function.
 */
static void swap(int *a, int *b, Metrics *m) {
    int temp = *a;
    *a = *b;
    *b = temp;
    // Política: 1 swap() conta como 1 "operação de troca"
    m->steps_swap++;
}

/**
 * @brief Implementação do Bubble Sort com contagem de passos.
 */
void bubble_sort(int *v, size_t n, Metrics *m) {
    if (n < 2) return;

    for (size_t i = 0; i < n - 1; i++) {
        // Otimização: se não houver trocas, o vetor está ordenado
        int swapped = 0;
        for (size_t j = 0; j < n - i - 1; j++) {
            // Conta 1 comparação
            m->steps_cmp++;
            if (v[j] > v[j + 1]) {
                swap(&v[j], &v[j + 1], m); // swap() conta 1 'steps_swap'
                swapped = 1;
            }
        }
        if (swapped == 0) {
            break; // Otimização para casos quase ordenados
        }
    }
}

/**
 * @brief Implementação do Selection Sort com contagem de passos.
 */
void selection_sort(int *v, size_t n, Metrics *m) {
    if (n < 2) return;

    for (size_t i = 0; i < n - 1; i++) {
        size_t min_idx = i;
        for (size_t j = i + 1; j < n; j++) {
            // Conta 1 comparação
            m->steps_cmp++;
            if (v[j] < v[min_idx]) {
                min_idx = j;
            }
        }
        // Só troca se o mínimo não for o próprio elemento
        if (min_idx != i) {
            swap(&v[i], &v[min_idx], m); // swap() conta 1 'steps_swap'
        }
    }
}

/**
 * @brief Implementação do Insertion Sort com contagem de passos.
 */
void insertion_sort(int *v, size_t n, Metrics *m) {
    if (n < 2) return;

    for (size_t i = 1; i < n; i++) {
        int key = v[i];
        long j = i - 1; // Usamos 'long' para evitar underflow no 'while'

        // Política: 1 'steps_cmp' por comparação (v[j] > key)
        // Política: 1 'steps_swap' por movimentação (v[j+1] = v[j])
        
        // Conta a primeira comparação (ou a falha, se j < 0)
        m->steps_cmp++; 
        while (j >= 0 && v[j] > key) {
            v[j + 1] = v[j];
            m->steps_swap++; // Conta a movimentação
            j--;
            
            // Conta as comparações subsequentes dentro do loop
            if (j >= 0) {
                 m->steps_cmp++;
            }
        }
        v[j + 1] = key;
        // Conta a inserção final como uma "troca/movimentação"
        m->steps_swap++; 
    }
}