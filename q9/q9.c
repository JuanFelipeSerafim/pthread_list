#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#define NUM_MAX_PASSAGEIROS 10
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t filaVazia=PTHREAD_COND_INITIALIZER,filaCheia=PTHREAD_COND_INITIALIZER;
pthread_barrier_t barreiraPassageiros;

// Declaração de estruturas primitivas para criarmos uma fila
//FILA DE NO MÁXIMO 10 PASSAGEIROS
typedef struct Node{
    int idPassageiro;
    struct Node *next;
}Node;

typedef struct{
    int tamAtual;
    Node *head,*tail;
}Fila;
//

//Variaveis globais para fácil manipulação e acesso pelas threads
Fila *filaPassageiros=NULL;//->Região crítica
int ciclos=0,numPassageiro=0,encerraCarrinho=0;
//

void initFila(){
    filaPassageiros = (Fila*) malloc(sizeof(Fila));
    filaPassageiros->tamAtual=0;
    filaPassageiros->head=NULL;
    filaPassageiros->tail=NULL;
}

//Função que realizará o enfileiramento
void enfileirar(int idThread){
    
    Node *temp=NULL;   
    
    pthread_mutex_lock(&mutex);//-> Início da manipulação da região crítica

    while(filaPassageiros->tamAtual==NUM_MAX_PASSAGEIROS){//Estou conferindo se a fila está lotada. se sim, põe a thread que requisitou entrar na fila pra dormir
        printf("Fila cheia :( \n");
        pthread_cond_wait(&filaVazia,&mutex);//espero algum sinal de ALGUMA fila VAZIA porque TODAS ESTÃO CHEIAS!!!!
    }
    
    
    //Inserção de elementos no FIM da fila abaixo
    
    temp= (Node*) malloc(sizeof(Node));
    temp->idPassageiro=idThread;
    temp->next=NULL;//eh o ultimo elemento ele.
    
    if(filaPassageiros->tail!=NULL)//Fazendo o ultimo elemento antes do novo apontar para temp. Se a fila estiver vazia(tail==NULL), na linha abaixo coloco o tail para apontar para o único elemento.
        filaPassageiros->tail->next=temp;

    filaPassageiros->tail=temp;
    filaPassageiros->tamAtual++;

    printf("Enfileirando\nTAMFILA=%d\n\n",filaPassageiros->tamAtual);

    if(!filaPassageiros->tamAtual || filaPassageiros->tamAtual==1 )//se for 0 ou somente 1 elemento
        filaPassageiros->head=filaPassageiros->tail;


    //Se a fila estiver cheia, mando um sinal para o responsável do desenfileiramento acordar caso esteja dormindo
    if(filaPassageiros->tamAtual==NUM_MAX_PASSAGEIROS-1)
        pthread_cond_signal(&filaCheia);//se ainda tiver 1 elemento, tem a chance desse nao ter sido consumido. portanto, existe a tentativa de acordar a thread consumidora para consumir esse elemento.

    pthread_mutex_unlock(&mutex);//-> Final da manipulação da região crítica

}


void desenfileirar(){
    Node *temp=NULL;

    pthread_mutex_lock(&mutex);//-> Início da manipulação da região crítica

    while(filaPassageiros->tamAtual==0 && !encerraCarrinho){//Estou conferindo se a fila está totalmente vazia E se é o momento de encerrar o carrinho, visto que todas as threads já executaram seu ciclo.
        printf("Fila vazia :(\n");
        pthread_cond_wait(&filaCheia,&mutex);//Aguardo algum sinal de fila CHEIA.
    }
    // algoritmo de exclusão do 1º elemento da fila
    if(filaPassageiros->tamAtual>0){// Caso seja o momento de encerrar o carrinho, verifico novamente se ainda tem elementos na fila para retirá-los    
        temp=filaPassageiros->head;
        filaPassageiros->head=filaPassageiros->head->next;
        filaPassageiros->tamAtual--;
        free(temp);
        temp=NULL;
    }

    if(filaPassageiros->tamAtual==0)// Se a fila ta vazia, sinalizo que é necessário encher 
        pthread_cond_broadcast(&filaVazia);
    
    printf("Saindo do carrinho\nTAMFILA=%d\n\n",filaPassageiros->tamAtual);
    
    pthread_mutex_unlock(&mutex);//-> Fim da manipulação da região crítica
}


void* carrinho(void* ID){// Consumidor
    while(!encerraCarrinho){
        desenfileirar();
    }
    pthread_exit(NULL);
}

void* passageiro(void* ID){//produtor
    int idThread = *((int*)ID),i;
    for(i=0;i<ciclos;i++){
        enfileirar(idThread);
        pthread_barrier_wait(&barreiraPassageiros);//Aguardo todas as threads serem enfileiradas para prosseguir com o próximo enfileiramento. Nesse momento, a thread que enfileirará algum elemento dormirá até todas se enfileirarem
    }
    pthread_exit(NULL);
}


int main(){
    pthread_t *threadPassageiro=NULL,*threadCarrinho=NULL;
    int *idThreadPassageiro=NULL,idThreadCarrinho=0,i=0,j=0;
    
    printf("Digite a quantidade de passageiros\n");
    scanf("%d",&numPassageiro);
    printf("Diga o numero de ciclos\n");
    scanf("%d",&ciclos);
    
    
    threadCarrinho = (pthread_t*) malloc(sizeof(pthread_t));
    
    idThreadPassageiro= (int*) malloc(numPassageiro*sizeof(int));
    threadPassageiro = (pthread_t*) malloc(numPassageiro*sizeof(pthread_t));

    initFila();
    
    pthread_barrier_init(&barreiraPassageiros,NULL,numPassageiro);

    for(i=0;i<numPassageiro;i++){
        idThreadPassageiro[i]=i;
        pthread_create(&(threadPassageiro[i]),NULL,passageiro,(void*)&(idThreadPassageiro[i]));

    }
    pthread_create(threadCarrinho,NULL,carrinho,(void*)&(idThreadCarrinho));
    
    for(i=0;i<numPassageiro;i++){//Enquanto aguardo as threads encerrarem, existe a possibilidade de ter alguma na fila e o carrinho não ser alertado sobre ter algum elemento na fila. Por isso, a medida que as threads vão encerrando, dou um sinal para o carrinho retirar essas threads faltantes
        pthread_join(threadPassageiro[i],NULL);
        pthread_cond_signal(&filaCheia);
        if(i==numPassageiro-1)//Quando a última thread encerrar, altero a variável que fará (caso o carrinhos esteja dormindo) o carrinho não dormir novamente
            encerraCarrinho=1;
    }
    pthread_cond_signal(&filaCheia);// Esse realmente garante que a fila esvaziará
        
    pthread_join(*threadCarrinho,NULL);

    pthread_barrier_destroy(&barreiraPassageiros);
    
    free(threadCarrinho);
    free(idThreadPassageiro);
    free(threadPassageiro);
    free(filaPassageiros);
    idThreadPassageiro=NULL; threadPassageiro=NULL;threadCarrinho=NULL;filaPassageiros=NULL;
    pthread_exit(NULL);
    return 0;
}
