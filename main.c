/*
 * Ponto de entrada (main) para o TDE A2 de Ordenação.
 * * ATUALIZAÇÃO VISUAL:
 * - Armazena resultados em memória.
 * - Exibe uma tabela formatada e alinhada (ASCII) para leitura humana.
 * - Exibe o bloco CSV separadamente no final para facilitar o copy-paste.
 * - Loop de validação do RGM (não encerra no erro).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>    
#include <ctype.h> 

#include "sorts.h"

// --- Configurações ---
#define N_RUNS 5 
#define RGM_MAX_LEN 20
#define MAX_RESULTS 30 // Espaço suficiente para guardar os resultados

// --- Estruturas de Apoio ---

typedef struct {
    long long avg_steps;
    double avg_time_ms;
} BenchmarkMetrics;

// Estrutura para armazenar uma linha do relatório para impressão posterior
typedef struct {
    const char* method_name;
    size_t n;
    const char* case_name;
    long long steps;
    double time_ms;
} ResultRow;

// --- Variáveis Globais (para facilitar armazenamento) ---
ResultRow results[MAX_RESULTS];
int result_count = 0;

// --- Protótipos ---
int* str_to_digits(const char* rgm_str, size_t *n_out);
void fill_random(int *v, size_t n);
void print_array(int *v, size_t n, const char* label);
void print_formatted_table();
void print_csv_block();

/**
 * @brief Executa bateria de testes e guarda o resultado na lista global.
 */
void run_and_store(
    void (*sort_func)(int*, size_t, Metrics*), 
    const char* name,
    const int *source_data, 
    size_t n,
    const char* case_name,
    int is_random_case
);

BenchmarkMetrics execute_batch(
    void (*sort_func)(int*, size_t, Metrics*), 
    const int *source_data, 
    size_t n,
    int is_random_case
);

// --- Main ---

int main() {
    srand(time(NULL));

    // Configuração dos Algoritmos
    void (*sort_functions[])(int*, size_t, Metrics*) = {
        bubble_sort, 
        selection_sort, 
        insertion_sort
    };
    const char* sort_names[] = {"Bubble", "Selection", "Insertion"};
    int N_ALGOS = 3;

    // Configuração dos Tamanhos
    size_t benchmark_sizes[] = {100, 1000, 10000};
    int N_SIZES = 3;

    // 1. Leitura e Validação do RGM (Loop de Repetição)
    char rgm_str[RGM_MAX_LEN];
    size_t n_rgm;
    int *rgm_digits = NULL;

    printf("========================================\n");
    printf("   ANALISE DE ALGORITMOS DE ORDENACAO   \n");
    printf("========================================\n");

    while (1) {
        printf("Digite seu RGM (max 8 digitos): ");
        
        if (!fgets(rgm_str, RGM_MAX_LEN, stdin)) return 1; // Saída forçada (EOF)
        rgm_str[strcspn(rgm_str, "\n")] = 0;

        // Ignora input vazio
        if (strlen(rgm_str) == 0) continue;

        rgm_digits = str_to_digits(rgm_str, &n_rgm);
        
        if (!rgm_digits) {
            fprintf(stderr, ">> Erro: Entrada invalida. Use apenas numeros.\n\n");
            continue; // Volta para o início do while
        }

        // Verificação de tamanho limite (Max 8 dígitos)
        if (n_rgm > 8) {
            fprintf(stderr, ">> Erro: RGM muito longo (%zu digitos). Maximo permitido e 8.\n\n", n_rgm);
            free(rgm_digits);
            rgm_digits = NULL;
            continue; // Volta para o início do while
        }

        // Se chegou aqui, está tudo certo, sai do loop
        break;
    }

    printf("\n[RGM N=%zu: ", n_rgm);
    print_array(rgm_digits, n_rgm, "");
    printf("\nProcessando... (Media de %d execucoes por caso)\n", N_RUNS);
    printf("Aguarde, testes pesados podem demorar...\n\n");

    // 2. Teste A: RGM
    for (int i = 0; i < N_ALGOS; i++) {
        run_and_store(sort_functions[i], sort_names[i], rgm_digits, n_rgm, "RGM", 0);
    }

    // 3. Teste B: Aleatórios
    int *random_buffer = (int*)malloc(10000 * sizeof(int)); 
    
    for (int s = 0; s < N_SIZES; s++) {
        size_t current_n = benchmark_sizes[s];
        fill_random(random_buffer, current_n); // Preenche inicial

        for (int i = 0; i < N_ALGOS; i++) {
            run_and_store(sort_functions[i], sort_names[i], random_buffer, current_n, "Aleatorio", 1);
        }
        // Feedback visual de progresso
        printf(". "); 
        fflush(stdout);
    }
    printf("Concluido!\n\n");

    // 4. Exibição dos Resultados
    print_formatted_table();
    print_csv_block();

    // 5. Limpeza
    free(rgm_digits);
    free(random_buffer);
    return 0;
}

// --- Implementação ---

void run_and_store(
    void (*sort_func)(int*, size_t, Metrics*), 
    const char* name,
    const int *source_data, 
    size_t n,
    const char* case_name,
    int is_random_case
) {
    BenchmarkMetrics res = execute_batch(sort_func, source_data, n, is_random_case);
    
    // Armazena no array global
    if (result_count < MAX_RESULTS) {
        results[result_count].method_name = name;
        results[result_count].n = n;
        results[result_count].case_name = case_name;
        results[result_count].steps = res.avg_steps;
        results[result_count].time_ms = res.avg_time_ms;
        result_count++;
    }
}

BenchmarkMetrics execute_batch(
    void (*sort_func)(int*, size_t, Metrics*), 
    const int *source_data, 
    size_t n,
    int is_random_case
) {
    Metrics m;
    double total_time = 0;
    long long total_steps = 0;
    int *temp_vec = (int*)malloc(n * sizeof(int));

    for (int k = 0; k < N_RUNS; k++) {
        if (is_random_case) fill_random(temp_vec, n);
        else memcpy(temp_vec, source_data, n * sizeof(int));

        reset_metrics(&m);
        clock_t t0 = clock();
        sort_func(temp_vec, n, &m);
        clock_t t1 = clock();

        double run_time_ms = 1000.0 * (t1 - t0) / CLOCKS_PER_SEC;
        total_time += run_time_ms;
        total_steps += (m.steps_cmp + m.steps_swap);
        
        if (run_time_ms < 1.0) sleep(0);
    }

    free(temp_vec);
    BenchmarkMetrics res;
    res.avg_time_ms = total_time / N_RUNS;
    res.avg_steps = total_steps / N_RUNS;
    return res;
}

void print_formatted_table() {
    printf("===========================================================================\n");
    printf("| %-10s | %-6s | %-10s | %-15s | %-12s |\n", "METODO", "N", "CASO", "PASSOS", "TEMPO (ms)");
    printf("|------------|--------|------------|-----------------|--------------|\n");
    
    for (int i = 0; i < result_count; i++) {
        printf("| %-10s | %-6zu | %-10s | %-15lld | %-12.4f |\n", 
            results[i].method_name, 
            results[i].n, 
            results[i].case_name, 
            results[i].steps, 
            results[i].time_ms);
        
        // Adiciona separador visual entre grupos de tamanho diferente (exceto no último)
        if (i < result_count - 1 && results[i].n != results[i+1].n) {
            printf("|------------|--------|------------|-----------------|--------------|\n");
        }
    }
    printf("===========================================================================\n\n");
}

void print_csv_block() {
    printf("Copie os dados abaixo para seu relatorio/excel:\n");
    printf(">>> INICIO CSV <<<\n");
    printf("metodo,N,caso,passos,tempo_ms\n");
    for (int i = 0; i < result_count; i++) {
        printf("%s,%zu,%s,%lld,%.4f\n", 
            results[i].method_name, 
            results[i].n, 
            results[i].case_name, 
            results[i].steps, 
            results[i].time_ms);
    }
    printf(">>> FIM CSV <<<\n");
}

int* str_to_digits(const char* rgm_str, size_t *n_out) {
    size_t n = 0;
    for (size_t i = 0; rgm_str[i]; i++) if (isdigit((unsigned char)rgm_str[i])) n++;
    if (n == 0) { *n_out = 0; return NULL; }
    int *digits = malloc(n * sizeof(int));
    size_t k = 0;
    for (size_t i = 0; rgm_str[i]; i++) {
        if (isdigit((unsigned char)rgm_str[i])) digits[k++] = rgm_str[i] - '0';
    }
    *n_out = n;
    return digits;
}

void fill_random(int *v, size_t n) {
    for (size_t i = 0; i < n; i++) v[i] = rand() % 10000;
}

void print_array(int *v, size_t n, const char* label) {
    printf("%s[", label);
    for (size_t i = 0; i < n; i++) printf("%d%s", v[i], (i < n - 1) ? ", " : "");
    printf("]\n");
}