#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>    
#include <ctype.h>

#include "sorts.h"

// --- Configurações e Constantes ---

// Número de vezes que cada algoritmo será executado para calcular a média.
// Isso ajuda a suavizar flutuações de tempo causadas pelo sistema operacional.
#define N_RUNS 5 

// Tamanho máximo do buffer para ler o RGM (suficiente para 8 dígitos + newline).
#define RGM_MAX_LEN 20

// Limite máximo de linhas de resultado que podemos armazenar na memória.
// (3 algoritmos * (1 RGM + 3 Tamanhos de Benchmark)) = 12 linhas, então 30 sobra.
#define MAX_RESULTS 30 

// --- Estruturas de Apoio ---

/**
 * @brief Estrutura auxiliar para retornar as médias de uma bateria de testes.
 * Encapsula os resultados agregados de N_RUNS execuções.
 */
typedef struct {
    long long avg_steps; // Média de passos (comparações + trocas)
    double avg_time_ms;  // Média de tempo em milissegundos
} BenchmarkMetrics;

/**
 * @brief Estrutura para persistir uma linha do relatório final.
 * Permite armazenar todos os dados durante a execução e imprimi-los 
 * de forma organizada apenas no final do programa.
 */
typedef struct {
    const char* method_name; // Nome do algoritmo (ex: "Bubble")
    size_t n;                // Tamanho do vetor testado
    const char* case_name;   // Nome do caso (ex: "Aleatorio" ou "RGM")
    long long steps;         // Total de passos medidos
    double time_ms;          // Tempo medido
} ResultRow;

// --- Variáveis Globais ---

// Array global para armazenar os resultados. 
// Usado para evitar passar esse array como parâmetro em todas as funções.
ResultRow results[MAX_RESULTS];
int result_count = 0; // Contador de quantos resultados já foram salvos.

// --- Protótipos de Funções ---
// Declaração antecipada para que o 'main' possa chamá-las.

int* str_to_digits(const char* rgm_str, size_t *n_out);
void fill_random(int *v, size_t n);
void print_array(int *v, size_t n, const char* label);
void print_formatted_table();
void print_csv_block();

/**
 * @brief Função Wrapper: Executa uma bateria completa de testes e salva o resultado.
 * * Esta função serve como uma "fachada" simplificada para o main. Ela coordena:
 * 1. A execução do benchmark (chamando execute_batch).
 * 2. O armazenamento dos dados retornados no array global 'results'.
 */
void run_and_store(
    void (*sort_func)(int*, size_t, Metrics*), // Ponteiro para a função de ordenação
    const char* name,                          // Nome legível do algoritmo
    const int *source_data,                    // Vetor de origem dos dados
    size_t n,                                  // Tamanho do vetor
    const char* case_name,                     // Nome do caso de teste
    int is_random_case                         // Flag booleana (1=Gerar novos aleatórios, 0=Usar source_data fixo)
);

/**
 * @brief O coração da medição: Executa o algoritmo N_RUNS vezes e calcula a média.
 * * Responsável por alocar memória temporária, gerenciar o clock() e 
 * acumular as métricas de passos e tempo.
 */
BenchmarkMetrics execute_batch(
    void (*sort_func)(int*, size_t, Metrics*), 
    const int *source_data, 
    size_t n,
    int is_random_case
);

// --- Função Principal (Main) ---

int main() {
    // Inicializa o gerador de números aleatórios com o tempo atual.
    // Garante que cada execução do programa gere vetores diferentes.
    srand(time(NULL));

    // Vetor de ponteiros para funcoes: Permite iterar sobre os algoritmos em um loop.
    void (*sort_functions[])(int*, size_t, Metrics*) = {
        bubble_sort, 
        selection_sort, 
        insertion_sort
    };
    // Nomes correspondentes para exibição
    const char* sort_names[] = {"Bubble", "Selection", "Insertion"};
    int N_ALGOS = 3;

    // Definição dos tamanhos para o teste de escalabilidade (Benchmark)
    size_t benchmark_sizes[] = {100, 1000, 10000};
    int N_SIZES = 3;

    // Variáveis para processamento do RGM
    char rgm_str[RGM_MAX_LEN];
    size_t n_rgm;
    int *rgm_digits = NULL;

    printf("========================================\n");
    printf("   ANALISE DE ALGORITMOS DE ORDENACAO   \n");
    printf("========================================\n");

    // --- 1. Leitura e Validação Robusta do RGM ---
    // Loop infinito que só quebra quando uma entrada válida é fornecida.
    while (1) {
        printf("Digite seu RGM (max 8 digitos): ");
        
        // fgets é mais seguro que scanf pois evita buffer overflow
        if (!fgets(rgm_str, RGM_MAX_LEN, stdin)) return 1; // Erro fatal ou EOF
        
        // Remove o caractere de nova linha (\n) que o fgets captura
        rgm_str[strcspn(rgm_str, "\n")] = 0;

        // Ignora se o usuário apenas apertou Enter
        if (strlen(rgm_str) == 0) continue;

        // --- Validação Estrita: Verifica caractere por caractere ---
        int contains_invalid_char = 0;
        for (int i = 0; rgm_str[i] != '\0'; i++) {
            if (!isdigit((unsigned char)rgm_str[i])) {
                contains_invalid_char = 1;
                break; // Encontrou erro, para de verificar
            }
        }

        // Se encontrou letra ou símbolo, rejeita e reinicia o loop
        if (contains_invalid_char) {
            fprintf(stderr, ">> Erro: O RGM deve conter APENAS numeros. Letras/simbolos nao sao permitidos.\n");
            fprintf(stderr, ">> Por favor, tente novamente.\n\n");
            continue; 
        }

        // Converte string para vetor de inteiros
        rgm_digits = str_to_digits(rgm_str, &n_rgm);
        
        // Validação extra de segurança
        if (!rgm_digits || n_rgm == 0) {
            fprintf(stderr, ">> Erro: Entrada invalida.\n\n");
            continue; 
        }

        // Validação de Regra de Negócio: Máximo 8 dígitos
        if (n_rgm > 8) {
            fprintf(stderr, ">> Erro: RGM muito longo (%zu digitos). Maximo permitido e 8.\n\n", n_rgm);
            free(rgm_digits); // Libera memória antes de pedir de novo
            rgm_digits = NULL;
            continue; 
        }

        // Se chegou aqui, a entrada é válida. Sai do while.
        break;
    }

    printf("\n[RGM] N=%zu: ", n_rgm);
    print_array(rgm_digits, n_rgm, "");
    printf("\nProcessando... (Media de %d execucoes por caso)\n", N_RUNS);
    printf("Aguarde, testes pesados podem demorar...\n\n");

    // --- 2. Execução dos Testes com RGM (Entrada A) ---
    // Roda os 3 algoritmos usando os dígitos do RGM como entrada.
    for (int i = 0; i < N_ALGOS; i++) {
        // is_random_case = 0: Usa os dados fixos do RGM
        run_and_store(sort_functions[i], sort_names[i], rgm_digits, n_rgm, "RGM", 0);
    }

    // --- 3. Execução dos Benchmarks Aleatórios (Entrada B) ---
    // Aloca um buffer grande o suficiente para o maior teste (10.000)
    // para evitar ficar dando malloc/free dentro do loop repetidamente.
    int *random_buffer = (int*)malloc(10000 * sizeof(int)); 
    
    for (int s = 0; s < N_SIZES; s++) {
        size_t current_n = benchmark_sizes[s];
        
        // Gera um vetor aleatório inicial
        fill_random(random_buffer, current_n); 

        for (int i = 0; i < N_ALGOS; i++) {
            // is_random_case = 1: A função vai gerar novos aleatórios a cada rodada interna
            run_and_store(sort_functions[i], sort_names[i], random_buffer, current_n, "Aleatorio", 1);
        }
        // Feedback visual (.) para o usuário saber que não travou
        printf(". "); 
        fflush(stdout);
    }
    printf("Concluido!\n\n");

    // --- 4. Relatórios Finais ---
    print_formatted_table(); // Tabela bonita
    print_csv_block();       // Dados brutos para Excel

    // --- 5. Limpeza de Memória ---
    free(rgm_digits);
    free(random_buffer);
    
    return 0; // Sucesso
}

// --- Implementação das Funções Auxiliares ---

void run_and_store(
    void (*sort_func)(int*, size_t, Metrics*), 
    const char* name,
    const int *source_data, 
    size_t n,
    const char* case_name,
    int is_random_case
) {
    // Executa o processamento pesado
    BenchmarkMetrics res = execute_batch(sort_func, source_data, n, is_random_case);
    
    // Salva o resultado na estrutura global se houver espaço
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
    
    // Vetor temporário que será ordenado (preserva o original intacto)
    int *temp_vec = (int*)malloc(n * sizeof(int));

    for (int k = 0; k < N_RUNS; k++) {
        if (is_random_case) {
            // Caso Aleatório: Gera novos dados para evitar vício (ordenar vetor já ordenado)
            fill_random(temp_vec, n);
        } else {
            // Caso Fixo (RGM): Copia sempre os dados originais para restaurar a desordem
            memcpy(temp_vec, source_data, n * sizeof(int));
        }

        // Reseta métricas antes de cada execução
        reset_metrics(&m);
        
        // Medição do tempo
        clock_t t0 = clock();
        sort_func(temp_vec, n, &m); // Chama o algoritmo via ponteiro
        clock_t t1 = clock();

        // Acumula os resultados
        double run_time_ms = 1000.0 * (t1 - t0) / CLOCKS_PER_SEC;
        total_time += run_time_ms;
        total_steps += (m.steps_cmp + m.steps_swap);
        
        // Se o teste for muito rápido (< 1ms), força uma pausa minúscula 
        // para garantir que a semente do rand() mude se for baseada em tempo,
        // embora srand() seja chamado apenas uma vez no main.
        if (run_time_ms < 1.0) {
            // sleep(0) em alguns sistemas cede a vez da CPU
        }
    }

    free(temp_vec);
    
    // Calcula e retorna as médias
    BenchmarkMetrics res;
    res.avg_time_ms = total_time / N_RUNS;
    res.avg_steps = total_steps / N_RUNS;
    return res;
}

void print_formatted_table() {
    // Cabeçalho da tabela
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
        
        // Adiciona uma linha divisória quando o tamanho N muda (ex: de 100 para 1000)
        // para melhorar a visualização dos grupos.
        if (i < result_count - 1 && results[i].n != results[i+1].n) {
            printf("|------------|--------|------------|-----------------|--------------|\n");
        }
    }
    printf("===========================================================================\n\n");
}

void print_csv_block() {
    // Formato CSV simples para facilitar importação
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
    // Primeiro passo: Contar quantos dígitos válidos existem
    size_t n = 0;
    for (size_t i = 0; rgm_str[i]; i++) {
        if (isdigit((unsigned char)rgm_str[i])) n++;
    }

    if (n == 0) { *n_out = 0; return NULL; }

    // Segundo passo: Alocar memória exata
    int *digits = malloc(n * sizeof(int));
    if (!digits) return NULL; // Falha de alocação

    // Terceiro passo: Preencher o vetor convertendo char para int
    size_t k = 0;
    for (size_t i = 0; rgm_str[i]; i++) {
        if (isdigit((unsigned char)rgm_str[i])) {
            digits[k++] = rgm_str[i] - '0'; // Truque ASCII: '5' - '0' = 5
        }
    }
    *n_out = n;
    return digits;
}

void fill_random(int *v, size_t n) {
    for (size_t i = 0; i < n; i++) {
        v[i] = rand() % 10000; // Gera números entre 0 e 9999
    }
}

void print_array(int *v, size_t n, const char* label) {
    printf("%s[", label);
    for (size_t i = 0; i < n; i++) {
        printf("%d%s", v[i], (i < n - 1) ? ", " : "");
    }
    printf("]\n");
}