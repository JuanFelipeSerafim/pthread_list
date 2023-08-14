#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define TAM 10
#define N 100
#define M 10
//Variáveis globais para facilitar o acesso das threads
int array[TAM] = {0};//->Região Crítica!!

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t canRead = PTHREAD_COND_INITIALIZER;

void *write(void *n) {
    int tid = *((int *)n),i=0;
    while (1) {
        pthread_mutex_lock(&mutex);//Nessa questão somente é necessário travar o mutex durante a escrita. Durante a leitura, as threads são livres pra ler arquivos ao mesmo tempo

        array[i] = i;
        printf("Thread %d escreveu %d no array\n", tid, i);

        i=(++i)%TAM;//1º i++; 2º i=i%TAM; Necessário para ficar acessando todas as posições do array

        pthread_cond_broadcast(&canRead);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

void *read(void *n) {
    int tid = *((int *)n),i=0;

    while (1) {
        
        pthread_cond_wait(&canRead, &mutex);

        printf("Thread %d leu %d do array\n", tid, array[i]);

        i=(++i)%TAM;

    }

    pthread_exit(NULL);
}

int main() {
    //Declaração de variáveis
    pthread_t escritores[M];
    pthread_t leitores[N];
    int idLeitores[N];
    int idEscritores[M];

    for (int i = 0; i < M; i++) {
        idEscritores[i] = i;
        pthread_create(&escritores[i], NULL, write, (void *)&idEscritores[i]);
    }
    for (int i = 0; i < N; i++) {
        idLeitores[i] = i;
        pthread_create(&leitores[i], NULL, read, (void *)&idLeitores[i]);
    }

    for (int i = 0; i < M; i++) {
        pthread_join(escritores[i], NULL);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(leitores[i], NULL);
    }

    pthread_exit(NULL);
}