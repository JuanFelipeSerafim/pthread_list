#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 3
#define MAX_ITEMS 200
#define THREADS 3

// **Estruturas**
// Estrutura para definir um processo com uma funcao e dois parametros
typedef struct Process {
    int (*Function)();
    int param1;
    int param2;
} Process;

Process queue[MAX_ITEMS];  // Fila de processos a serem executados
int buffer[BUFFER_SIZE];   // Buffer (nesse caso vai armazenar apenas o id dos processos)
int items = 0;             // Número de itens do buffer
int first = 0;             // Variável de apoio do buffer
int last = 0;              // Variável de apoio do buffer

int queuedCounter = 0;          // Contador de processos colocados na fila
int result[MAX_ITEMS];          // Vetor que armazena os resultados dos processos
int resolved[MAX_ITEMS] = {0};  // Vetor que armazena se processo foi concluido ou nao

// **Variaveis de suporte da biblioteca**
// Mutexes
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resultMutex = PTHREAD_MUTEX_INITIALIZER;

// Variaveis de condicao
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t fill = PTHREAD_COND_INITIALIZER;
pthread_cond_t resolve = PTHREAD_COND_INITIALIZER;

// ** Q7: Inicialização de variaveis e constantes globais **
#define C 3
int ballance[C] = {500, 1000, 100};  // Saldos iniciais

// **Funcoes**
// Funcao "agendarExecucao" que bota um item na lista de processos
int putQueue(int param1, int param2, int (*func)(int, int)) {
    Process process;
    process.param1 = param1;
    process.param2 = param2;
    process.Function = func;

    queue[queuedCounter] = process;
    queuedCounter++;

    return queuedCounter - 1;
}

// Funcao pegarResultadoExecucao, que retorna o resultado de um processo quando este eh concluido
int printResult(int id, int ignoredVar) {
    // Checa se foi resolvido, se nao, espera
    while (!resolved[id]) {
        pthread_cond_wait(&resolve, &mutex);
    }

    printf("Resultado do processo %d: %d\n", id, result[id]);

    return result[id];
}

// Funcao que bota um item da fila no buffer
void put(int i) {
    // Como uma regiao critica vai ser acessada, eh necessario usar mutex
    pthread_mutex_lock(&mutex);

    // Se o buffer estiver cheio, espera esvaziar
    while (items == BUFFER_SIZE) {
        pthread_cond_wait(&empty, &mutex);
    }

    buffer[last] = i;

    items++;
    last++;

    if (last == BUFFER_SIZE) {
        last = 0;
    }

    // Se a fila de itens estava vazia e agora possui um item, sinaliza que ha algo a ser consumido/executado
    if (items == 1) {
        pthread_cond_broadcast(&fill);
    }

    pthread_mutex_unlock(&mutex);
}

// Funcao responsavel por gerenciar a insercao de itens no buffer, a partir do numero de process agendados
void *bufferManager() {
    for (int i = 0; i < queuedCounter; i++) {
        put(i);
    }

    pthread_exit(NULL);
}

// Funcao responsavel por pegar o item do buffer que deve ser executado no momento e executa-lo
int get() {
    int id;
    // Usa mutex pois acessa regiao critica
    pthread_mutex_lock(&mutex);

    // Se nao tiver itens no buffer, espera que algum seja posto
    while (items == 0) {
        pthread_cond_wait(&fill, &mutex);
    }

    id = buffer[first];

    // Executa a funcao, salva o resultado e marca ocmo resolvido
    Process process = queue[id];
    result[id] = process.Function(process.param1, process.param2);
    resolved[id] = 1;

    // Manda um sinal indicando que algo foi resolvido. Caso tenha sido alguma funcao na qual a funcao print espera a resolucao, vai destravar sua execucao
    pthread_cond_broadcast(&resolve);

    items--;
    first++;

    if (first == BUFFER_SIZE) {
        first = 0;
    }

    // Se for liberado espaco no buffer, sinaliza, para que seja preenchido
    if (items == BUFFER_SIZE - 1) {
        pthread_cond_signal(&empty);
    }

    pthread_mutex_unlock(&mutex);

    return id;
}

// Funcao responsavel por gerenciar a execucao de itens do buffer, a partir do numero de process agendados
void *executer(void *n) {
    int tid = *((int *)n);

    for (int i = tid; i < queuedCounter; i += THREADS) {
        get(i);
    }

    pthread_exit(NULL);
}

// Função de débito para teste da API (Q7)
int debit(int client, int value) {
    if (ballance[client] - value >= 0) {
        ballance[client] -= value;
    }

    return ballance[client];
}

// Execução de funções de operação bancária, usando a API (Q7)
void scheduler() {
    // Realiza débito de 50 reais no usuario 1
    int idOp1 = putQueue(0, 50, &debit);

    // Imprime o resultado da operacao 1: 450
    putQueue(idOp1, 0, &printResult);

    // Realiza débito de 10 reais no usuario 1
    int idOp2 = putQueue(0, 20, &debit);

    // Realiza débito de 100 reais no usuario 2
    int idOp3 = putQueue(1, 200, &debit);

    // Imprime o resultado da operacao 3: 800
    putQueue(idOp3, 0, &printResult);

    // Imprime o resultado da operacao 2: 430
    putQueue(idOp2, 0, &printResult);

    // Realiza débito de 75 reais no usuario 3
    int idOp4 = putQueue(2, 75, &debit);

    // Imprime o resultado da operacao 4: 25
    putQueue(idOp4, 0, &printResult);
}

int main() {
    // Declaracao das threads
    pthread_t producerThread;
    pthread_t consumerThread[THREADS];
    int *ids[THREADS];

    // Funcao de agendamento de processos
    scheduler();

    // Criacao das threads
    for (int i = 0; i < THREADS; i++) {
        ids[i] = (int *)malloc(sizeof(int));
        *ids[i] = i;
        pthread_create(&consumerThread[i], NULL, executer, (void *)ids[i]);
    }

    pthread_create(&producerThread, NULL, bufferManager, NULL);

    pthread_exit(NULL);
}