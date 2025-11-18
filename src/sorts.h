/*
 * Arquivo de cabeçalho (header) para os algoritmos de ordenação.
 * Define a estrutura de Métricas e os protótipos das funções.
 */

#ifndef SORTS_H
#define SORTS_H

#include <stddef.h> // Para size_t

// Estrutura para coletar métricas de desempenho
typedef struct {
    long long steps_cmp;  // Contador para comparações
    long long steps_swap; // Contador para trocas/movimentações
} Metrics;

/**
 * @brief Reseta os contadores de métricas para zero.
 * @param m Ponteiro para a estrutura Metrics.
 */
void reset_metrics(Metrics *m);

/**
 * @brief Ordena um vetor de inteiros usando o método Bubble Sort.
 * @param v Ponteiro para o vetor.
 * @param n Tamanho do vetor.
 * @param m Ponteiro para a estrutura Metrics para contagem de passos.
 */
void bubble_sort(int *v, size_t n, Metrics *m);

/**
 * @brief Ordena um vetor de inteiros usando o método Selection Sort.
 * @param v Ponteiro para o vetor.
 * @param n Tamanho do vetor.
 * @param m Ponteiro para a estrutura Metrics para contagem de passos.
 */
void selection_sort(int *v, size_t n, Metrics *m);

/**
 * @brief Ordena um vetor de inteiros usando o método Insertion Sort.
 * @param v Ponteiro para o vetor.
 * @param n Tamanho do vetor.
 * @param m Ponteiro para a estrutura Metrics para contagem de passos.
 */
void insertion_sort(int *v, size_t n, Metrics *m);

#endif // SORTS_H