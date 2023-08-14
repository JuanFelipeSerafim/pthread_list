#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Nessa questão não se faz necessário o uso de mutex porque pra cada elemento da matriz, uma thread é responsável para a conversão do pixel em escala de cinza


typedef struct{// concentrando os 3 valores em 1 variável só para facilitar a manipulação
    int R,G,B;
}Pixel;

Pixel **imagem_rgb=NULL;
int **imagem_gray=NULL;
int linha=0,coluna=0,valor_max=0;

void abrir_arquivo(){
    FILE *arq=NULL;
    char PPM[2]={};
    int i=0,j=0;
    arq=fopen("q2.txt","rb");
    
    if(arq==NULL){
        printf("Erro ao abrir!\nTalvez nao esteja na pasta de execucao do programa!\n");
        exit(-1);
    }
    fscanf(arq,"%s",PPM);

    if(strcmp(PPM,"P3")!=0){
        printf("Nao eh PPM do tipo P3!\nSaindo...");
        exit(-1);
    }

    fscanf(arq,"%d %d\n%d",&coluna,&linha,&valor_max);

    //alocando a matriz para a leitura do arquivo e para a saída
    imagem_rgb=(Pixel**)calloc(linha,sizeof(Pixel*));
    imagem_gray=(int**)calloc(linha,sizeof(int*));

    if(imagem_rgb==NULL || imagem_gray==NULL){
        printf("Falha na alocacao\n");
        exit(-1);
    }

    for(i=0;i<linha;i++){
        imagem_rgb[i]=(Pixel*)calloc(coluna,sizeof(Pixel));
        imagem_gray[i]=(int*)calloc(coluna,sizeof(int));
        if(imagem_rgb[i]==NULL || imagem_gray[i]==NULL){
            printf("Falha na alocacao\n");
            exit(-1);
        }
    }

    for(i=0;i<linha;i++)// lendo o conteúdo do arquivo necessário para o funcionamento do código
        for(j=0;j<coluna;j++)
            fscanf(arq,"%d %d %d",&(imagem_rgb[i][j].R),&(imagem_rgb[i][j].G),&(imagem_rgb[i][j].B));
    
    fclose(arq);
    arq=NULL;
}


//considerando a matriz M=|a0   a1   a2|
//                        |a3   a4   a5|
//cada thread será um elemento daqui
//figura ilustra a escolha do elemento vide numero de colunas

void* conversor_cinza(void* ID){
    int idThread = *((int*)ID),
    linhaMatriz=idThread/coluna,//utilizando o id de cada thread como sendo o identificado de qual elemento da matriz ela irá realizar o trabalho
    colunaMatriz=idThread%coluna;

    printf("Thread %d aqui\n", idThread);
    //conversao e salvamento na matriz imagem_gray
    //"Para fazer a conversão para tons de cinza (C), adote a seguinte fórmula: C = R*0.30 + G*0.59+ B*0.11"
    imagem_gray[linhaMatriz][colunaMatriz]=(imagem_rgb[linhaMatriz][colunaMatriz].R)*0.3+(imagem_rgb[linhaMatriz][colunaMatriz].G)*0.59+(imagem_rgb[linhaMatriz][colunaMatriz].B)*0.11;
    pthread_exit(NULL);
}


int main(){
    //declaração de variáveis
    int i=0,j=0;
    pthread_t *threads=NULL;
    int *idThread=NULL;
    abrir_arquivo();

    threads= (pthread_t *) malloc(linha*coluna*sizeof(pthread_t));//cria as threads e os id's de cada uma abaixo
    idThread= (int*) calloc(linha*coluna,sizeof(int));
    
    for(;i<linha*coluna;i++){
        idThread[i]=i;
        pthread_create(&(threads[i]),NULL,conversor_cinza,(void*)&(idThread[i]));
    }

    for(i=0;i<linha*coluna;i++)//aguarda as outras concluirem
        pthread_join(threads[i],NULL);

    for(i=0;i<linha;i++){// printando a escala de cinza de cada pixel 
        for(j=0;j<coluna;j++)
            printf("%d ",imagem_gray[i][j]);
        printf("\n");
    }


    for(i=0;i<linha;i++){
        free(imagem_rgb[i]);
        free(imagem_gray[i]);
    }

    free(imagem_gray);
    free(imagem_rgb);
    
    free(threads);
    free(idThread);

    imagem_rgb=NULL;imagem_gray=NULL;threads=NULL;idThread=NULL;
    
    pthread_exit(NULL);
    
    return 0;
}