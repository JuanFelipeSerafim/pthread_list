#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define linhaDisplay 150
#define colunaDisplay 40

//Nome do arquivo: assetID.txt

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
char display[150][40]={};// regiao critica aqui vúú


//considerando que oq tem dentro da funcao das threads é unico para cada uma das threads e somente variaveis globais que podem ser acessadas por todas as threads, seguimos... p.s:é isso mesmo! https://stackoverflow.com/questions/7387620/does-local-variable-in-thread-function-have-separe-copy-according-to-thread
void* read_assets(void* ID){
    FILE *arq=NULL;
    int idThread= *((int*)ID),i=0,j=0,linha=0,coluna=0,linhaInit=0,colunaInit=0;
    char nomeArq[11]="asset .txt";
    nomeArq[5]=48+idThread;//convertendo o id da thread em seu correspondente em caractere de acordo com a tabela ascii

    arq = fopen(nomeArq,"rb");
    if(arq==NULL){
        printf("Erro ao abrir o arquivo. Talvez nao esteja na pasta de execucao do codigo\n");
        exit(-1);
    }

    pthread_mutex_lock(&mutex);// vo comecar a alterar a regiao critica

    printf(" '%s' aqui pela thread %d\n",nomeArq,idThread);//linha teste

    fscanf(arq,"%d %d",&colunaInit,&linhaInit);
    fscanf(arq,"%d %d",&coluna,&linha);
    
    for(i=linhaInit;i<linhaInit+linha;i++)
        for(j=colunaInit;j<colunaInit+coluna;j++)
            fscanf(arq,"%c",&(display[i][j]));
    
    fclose(arq);
    arq=NULL;
    
    //printando o que foi lido
    for(i=0;i<linhaDisplay;i++)
        for(j=0;j<colunaDisplay;j++)
            printf("%c",display[i][j]);

    printf("\n");
    //limpando o display pra outras threads utilizarem
    for(i=0;i<linhaDisplay;i++)
        for(j=0;j<colunaDisplay;j++)
            display[i][j]=0;
        
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}


int main(){
    pthread_t *threads=NULL;
    int *idThread=NULL,i=0,numThreads=0,j=0;
    
    //inicializando o display
    for(;i<linhaDisplay;i++)
        for(;j<colunaDisplay;j++)
            display[i][j]=0;//espaco pela tabela ascii

    printf("Digite a quantidade de assets que vc deseja que aparecam na tela(MAX 10):\n");
    scanf("%d",&numThreads);
    if(!numThreads)//se o usuario digitar '0' de sacanagem
        numThreads++;
    
    threads = (pthread_t*) malloc(numThreads*sizeof(pthread_t));
    idThread = (int*) calloc(numThreads,sizeof(int));
    
    if(idThread==NULL || threads==NULL){
        printf("Falha ao alocar os vetores. Falow!!\n");
        exit(-1);
    }

    for(i=0;i<numThreads;i++){
        idThread[i]=i;
        pthread_create(&(threads[i]),NULL,read_assets,(void*)&(idThread[i]));// poderia colocar uma variavel aqui pra receber o retorno da pthread_create e verificar qual erro (caso tenha dado) ocorreu. Retorna '0' se tudo bem.    
    }

    for(i=0;i<numThreads;i++)//aguarda as outras threads concluirem
        pthread_join(threads[i],NULL);
    
    free(threads);
    free(idThread);
    threads=NULL;idThread=NULL;
    pthread_exit(NULL);// isso é pra thread da main
    return 0;
}