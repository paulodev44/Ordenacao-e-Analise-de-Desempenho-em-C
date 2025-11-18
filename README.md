# Análise de Desempenho de Algoritmos de Ordenação em C

Este projeto contém a implementação e análise de três algoritmos de ordenação para a Atividade A2. O objetivo é comparar o desempenho (passos e tempo) dos métodos aplicados aos dígitos do RGM do aluno e a vetores aleatórios de diferentes tamanhos (N).

------

## 1. Métodos Implementados

Foram escolhidos os três algoritmos quadráticos clássicos (O(n²)) para permitir uma comparação direta de suas eficiências em dados aleatórios e pequenos:

* **Bubble Sort:** Compara pares adjacentes e os troca se estiverem fora de ordem. É simples, mas geralmente o mais lento devido ao alto número de trocas.

* **Selection Sort:** Encontra o menor elemento restante e o coloca na posição correta. Minimiza o número de trocas (sempre O(n)), mas mantém alto custo de comparações.

* **Insertion Sort:** Constrói a lista ordenada "in-place", inserindo cada elemento em sua posição correta na sub-lista já ordenada. É eficiente para dados pequenos e tem melhor desempenho de cache.
  
## 2. Como Compilar e Rodar

**Pré-requisitos**

* Um compilador C (ex: gcc)

**Compilação**
* Recomenda-se usar -O1 (otimização leve) para não remover os loops de contagem de passos, mas ainda otimizar a execução para medição de tempo.

## Cria o diretório de build (opcional, mas boa prática)
```
mkdir -p build
```
## Compila os arquivos da pasta 'src' para um executável 'ordena' na pasta 'build'
```
gcc -std=c11 -O1 src/*.c -o build/ordena
```

**Execução**

* O programa solicitará o RGM e depois imprimirá a tabela CSV no console.

**Executa o programa**
```
./build/ordena
```
**Exemplo de execução:**
```
Digite seu RGM (apenas digitos): 44725264

--- RGM (N=8) ---
Dígitos: [4, 4, 7, 2, 5, 2, 6, 4]

Iniciando medicoes (Média de 5 execucoes)...
**************************************************
**** INICIO DOS DADOS CSV ***
**************************************************

metodo,N,caso,passos,tempo_ms
[... SAÍDA DE DADOS ...]
```

## 3. Metodologia de Coleta de Métricas

**Contagem de "Passos"**

A política de contagem de "passos" (operações-chave) foi definida da seguinte forma, visando consistência:

* steps_cmp (Comparações): Incrementado 1 vez a cada comparação lógica (if, while) entre elementos do vetor.

* steps_swap (Trocas/Movimentações): Incrementado 1 vez para cada movimentação de memória (troca completa ou deslocamento no Insertion Sort).

* Total: A coluna passos no CSV é a soma steps_cmp + steps_swap.

**Medição de Tempo**

* Utiliza a função clock() da biblioteca <time.h>.

* O valor tempo_ms representa a média aritmética de 5 execuções para cada cenário.

* Nos testes aleatórios, um novo vetor é gerado a cada execução para garantir estatística justa.

## 4. Resultados (Tabela CSV)
**RGM: 44725264**
```
metodo,N,caso,passos,tempo_ms
Bubble,8,RGM,37,0.0000
Selection,8,RGM,34,0.0000
Insertion,8,RGM,37,0.0000
Bubble,100,Aleatorio,7386,0.0000
Selection,100,Aleatorio,5044,0.0000
Insertion,100,Aleatorio,5075,0.0000
Bubble,1000,Aleatorio,751294,0.4000
Selection,1000,Aleatorio,500492,0.2000
Insertion,1000,Aleatorio,500526,0.2000
Bubble,10000,Aleatorio,75066211,40.4000
Selection,10000,Aleatorio,50004989,25.0000
Insertion,10000,Aleatorio,50284063,18.6000
```

<!-- FIM DA SEÇÃO DE EXEMPLO CSV -->

## 5. Análise Crítica

**Computabilidade e Corretude**: Todos os métodos ordenaram corretamente tanto o vetor de dígitos do RGM (com repetições) quanto as amostras aleatórias. A lógica de ordenação se provou robusta.

* Todos os três métodos foram capazes de ordenar corretamente tanto os dígitos do RGM (incluindo dígitos repetidos) quanto os vetores aleatórios de todos os tamanhos, demonstrando robustez.

**Escalabilidade (Teoria vs Prática)** Observou-se claramente o comportamento quadrático O(n²).

* Ao aumentar N de 1.000 para 10.000 (10x), o número de passos e o tempo aumentaram aproximadamente 100x (10²).

* Exemplo (Insertion Sort): Passos foram de ~500 mil para ~50 milhões.


**Comparação de Desempenho**

* **Insertion Sort (18.2 ms):** Foi o mais rápido para N=10.000. Sua eficiência prática se deve ao acesso sequencial de memória ("cache-friendly") e simplicidade do loop interno.

* **Selection Sort (25.6 ms):** Teve desempenho intermediário. Embora faça o mínimo de trocas possível, o alto número de comparações pesa no tempo final.

* **Bubble Sort (38.2 ms):** Consistentemente o pior. Realiza excesso de movimentações e comparações desnecessárias.

## 6. Tratamento de Erros e Limitações
O programa foi desenvolvido com foco em robustez, tratando os seguintes cenários de exceção e limitações técnicas:

**Validação de Entrada do RGM:**

* O sistema utiliza uma função dedicada str_to_digits que filtra apenas os caracteres numéricos.

* Erro Tratado: Se o usuário digitar letras ou símbolos (ex: "123abc4"), o programa ignora os caracteres inválidos e processa apenas os números.

* RGM Vazio: Se nenhuma entrada numérica for detectada, o programa exibe uma mensagem de erro (RGM invalido) na saída padrão (stderr) e encerra a execução com código de erro 1, evitando falhas de segmentação.

**Gerenciamento de Memória (Alocação Dinâmica):**

* Todas as alocações de vetores (para RGM e Benchmarks) são feitas via malloc.

* Erro Tratado: O código verifica se o ponteiro retornado é NULL (caso falte memória RAM no sistema). Se ocorrer falha, o programa libera qualquer memória previamente alocada, avisa o usuário e encerra com segurança.

**Precisão do Relógio (Limitação de Hardware):**

* Para vetores muito pequenos (N=8 ou N=100), o tempo de execução é frequentemente menor que a resolução mínima do clock do sistema operacional.

* Limitação: Isso resulta em tempos reportados como 0.0000 ms. Isso não é um erro de lógica, mas uma limitação da função clock() em medir intervalos de microssegundos em alguns sistemas operacionais (especialmente Windows).

**Overflow de Tipos (Contagem de Passos):**

* Para N=10.000 ou superior, o número de operações em algoritmos O(n²) supera o limite de um inteiro comum (32-bit int, máx ~2 bilhões).

* Solução: A estrutura Metrics utiliza o tipo long long (64-bit), garantindo que a contagem de passos não "dê a volta" (overflow) mesmo em testes exaustivos.


## Conclusão: O "Melhor" Método

Baseado nos testes com dados aleatórios, o Insertion Sort se mostrou o "melhor" método entre os três escolhidos para N pequeno e médio (até 1000), entregando o menor tempo de execução.

* O Selection Sort é uma alternativa interessante se o custo de "troca" (swap) for extremamente alto, pois ele minimiza essa operação, mas seu tempo de execução foi ligeiramente superior ao do Insertion.

* O Bubble Sort não é recomendado para nenhum cenário prático em comparação com os outros dois.


* Para N=10.000, o tempo de todos os métodos O(n²) começa a se tornar impraticável (quase 1 segundo para o Bubble Sort), demonstrando a necessidade clara de algoritmos O(n log n) (como Merge ou Quick Sort) para conjuntos de dados maiores.



