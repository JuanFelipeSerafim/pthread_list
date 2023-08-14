#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
/* Nessa questão o mutex será necessário, pois a variavel "count" é considerada uma região crítica. Dessa forma, evitamos que exista um incremento duplo sem querer*/
long int count=0;

pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;


void* incrementar(void *idDaThread){ // funcao a ser acessada cada vez que uma thread for "acionada". Sempre nessa mesma característica de descricao inicial.
    int idThread= *((int*)idDaThread);
    while(count<1000000){
        pthread_mutex_lock(&mutex);//->a thread que conseguir travar o mutex terá o acesso privilegiado a esse trecho do código
    //////////////////////////////////////
        if(count>=1000000){//esse broder aqui eh caso o valor seja incrementado, seja avisado ao usuario que a thread n chegou lá e o s.o resolveu colocar outra thread pra rodar
            pthread_mutex_unlock(&mutex);
            break;
        }
        count++;
        if(count==1000000)
            printf("Thread %d chegou ao 1.000.000!\n",idThread);
    //////////////////////////////////////
        pthread_mutex_unlock(&mutex);       
    }
    pthread_exit(NULL);
}


int main(){
    // declaração de variáveis
    pthread_t *thread=NULL; //sera um vetor de threads
    int *idDasThreads=NULL,
    i=0,numThread=0;


    printf("Digite a qtd de thread\n");
    scanf("%d",&numThread);
    
    //alocação da quantidade de threads, juntamente com seus identificadores
    thread= (pthread_t*) malloc(numThread*sizeof(pthread_t));
    idDasThreads = (int*) calloc(numThread,sizeof(int));
    
    //criação das threads para seu uso
    for(;i<numThread;i++){
        idDasThreads[i]=i;//id das threads
        pthread_create(&(thread[i]),NULL,incrementar,(void*)&(idDasThreads[i]));// criacao das threads sendo referenciadas com o id acima. retorna 0 se der certo!
    }

    for(i=0;i<numThread;i++) // quando alguma thread terminar a execucao, irá ficar esperando aqui pelas outras
        pthread_join(thread[i],NULL);
    
    printf("Contador na main == %ld\n",count);
    pthread_exit(NULL);
    
    free(thread);
    free(idDasThreads);
    thread=NULL; idDasThreads=NULL;
    return 0;
}