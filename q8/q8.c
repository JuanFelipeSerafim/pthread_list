#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define N 4 // Numero de threads
#define n 4 // Numeros de valores
#define P 10 // Precisao

// **Declaracao de variaveis**
// Definição de barreira
pthread_barrier_t barrier;

// Variaveis para armazenar inputs
int A[n][n] = {{2, 1, 3, 5}, {5, 7, 10, 12}, {2, 4, 8, 3}, {5, 2, 10, 11}};
int B[n] = {11, 13, 4, 6};

// Variavel para armazenar resultado
double x[n][P];

// ** Funcoes **
// Codigo a ser executado pela thread
void *threadCode(void *tid) {
    int k;
    int threadId = (*(int *)tid);

    // Laco de repeticao para execucao do codigo por diferentes threads usando diferentes parametros (o i varia para as threads)
    for (k = 0; k < P; k++) {
        int i, j;

        for (i = threadId; i < n; i += N) {
            // Codigo da operacao matematica e atribuicao ao vetor de resultados
            if (k == 0) {
                x[i][k] = 1;
            } else {
                float sum = 0;
                int j;

                for (j = 0; j < n; j++) {
                    if (i != j) {
                        sum += (float) (A[i][j] * x[j][k - 1]);
                    }
                }

                x[i][k] = (float) (1.0 / (A[i][i])) * (B[i] - sum);
            }

            printf("x%d^%d calculado na thread %d\n", i+1, k, threadId);
        }

        pthread_barrier_wait(&barrier); // Barreira para sincronizar a execucao dessa iteracao com todas as threads, ja que a iteracao seguinte depende de itens gerados nessa iteracao (com o mesmo "k")
    }
}

// Main
int main(int argc, char *argv[]) {
    pthread_t threads[N];
    int *taskIds[N];
    int t, u, i;

    pthread_barrier_init(&barrier, NULL, N); // Inicializa a barreira

    // Cria as threads
    for (t = 0; t < N; t++) {
        printf("No main: criando thread %d\n", t);
        taskIds[t] = (int *)malloc(sizeof(int));
        *taskIds[t] = t;
        pthread_create(&(threads[t]), NULL, threadCode, (void *)taskIds[t]);
    }

    for (u = 0; u < N; u++) {
        int *res;
        pthread_join(threads[u], NULL);
    }

    pthread_barrier_destroy(&barrier);

    // Exibe o resultado com o maximo nivel de precisao
    printf("Resultado:\n");
    for (i = 0; i < n; i++) {
        printf("x%d = %f\t", i + 1, x[i][P - 1]);
    }

    pthread_exit(NULL);
    return 0;
}