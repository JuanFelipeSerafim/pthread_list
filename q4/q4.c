#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sudoku[9][9]={};
int valido=1;//invalido eh 0


void lerArq(char numArq){
    FILE *arq=NULL;
    char nomeArq[]="matriz .txt";
    int i=0,j=0;
    nomeArq[6]=numArq;
    arq= fopen(nomeArq,"rb");
    if(arq==NULL){
        printf("Falha na leitura. Talvez nao esteja na pasta o arquivo da matriz a ser analisada\n");
        exit(-1);
    }
    //leitura de variáveis do arquivo
    for(i=0;i<9;i++)
        for(j=0;j<9;j++)
            fscanf(arq,"%d", &(sudoku[i][j]) );
    
    // verificando a leitura da matriz
    for(i=0;i<9;i++){
        for(j=0;j<9;j++)
            printf("%d ",sudoku[i][j]);
        printf("\n");
    }
    fclose(arq);
    arq=NULL;
}

void* verificaLinhaColuna(void* ID){//1+2+3+4+5+6+7+8+9=45
    int idThread = *((int*)ID),somaLinha=0,somaColuna=0,i=0;
    if(idThread<9){//as 9 primeiras threads irão analisar as linhas
        for(i=0;i<9 && valido;i++)
            somaLinha+=sudoku[idThread][i];
        if(somaLinha!=45)
            valido=0;
    }
    else{//as demais threads irão analisar as colunas
        for(i=0;i<9 && valido;i++)
            somaColuna+=sudoku[i][idThread-9];
        if(somaColuna!=45)
            valido=0;
    }
    pthread_exit(NULL);
}

void *verificaQuadrado(void* ID){// queremos acessar as submatrizes e analisa-las 
    int idThread = *((int*)ID),i=0,j=0, somaSubMat=0;
    //cada thread será responsavel por uma submatriz
    if(idThread<=2){// linha de 0 a 2. Aqui são as 3 primeiras submatrizes da esquerda pra direita
        for(i=0;i<3 && valido;i++){
            for(j=3*idThread;j<3*idThread+3 && valido;j++){//0->0,1->3,2->6
                somaSubMat+=sudoku[i][j];
            }
        }
        if(somaSubMat!=45)
            valido=0;
    }
    else if(idThread<=5){// linha de 3 a 5. Aqui são as 3 submatrizes da metade do sudoku da esquerda pra direita
        for(i=3;i<6 && valido;i++){
            for(j=3*idThread-9;j<(3*idThread-9)+3 && valido;j++){//3->0,4->3,5->6
                somaSubMat+=sudoku[i][j];
            }
        }
        if(somaSubMat!=45)
            valido=0;
    }
    else{// linha de 6 a 8. Aqui são as 3 ultimas submatrizes do final do sudoku da esquerda pra direita
        for(i=6;i<9 && valido;i++){
            for(j=3*idThread-18;j<(3*idThread-18)+3 && valido;j++){//6->0,7->3,8->6 OBRIGADO FUNÇÃO AFIM. assim q consegui percorrer as colunas pelo id das threads
                somaSubMat+=sudoku[i][j];
            }
        }
        if(somaSubMat!=45)
            valido=0;
    }
    pthread_exit(NULL);
}

int main(){
    pthread_t *threadsLinhaColuna=NULL,*threadsQuadrado=NULL;// dois tipos de threads 
    int *idThreadsLinhaColuna=NULL,*idThreadsQuadrado=NULL,i=0;
    char numArq=0;
    
    printf("Digite o numero do arquivo a ser analisado (MAX 2)\n");
    scanf(" %c",&numArq);
    if(numArq=='0')// se o usuario por 0 ""por engano""
        numArq++;
    
    lerArq(numArq);

    threadsLinhaColuna= (pthread_t*) malloc(18*sizeof(pthread_t));
    idThreadsLinhaColuna= (int*) calloc(18,sizeof(int));
    
    threadsQuadrado= (pthread_t*) malloc(9*sizeof(pthread_t));// 9 submatrizes 
    idThreadsQuadrado= (int*) calloc(9,sizeof(int));
    
    for(;i<18;i++){//o numero de linhas+colunas como um todo
        if(i<9){
            idThreadsLinhaColuna[i]=i;
            idThreadsQuadrado[i]=i;
            pthread_create( &(threadsLinhaColuna[i]),NULL,verificaLinhaColuna,&(idThreadsLinhaColuna[i]) );
            pthread_create( &(threadsQuadrado[i]),NULL,verificaQuadrado,&(idThreadsQuadrado[i]) );
        }
        else{
            idThreadsLinhaColuna[i]=i;
            pthread_create( &(threadsLinhaColuna[i]),NULL,verificaLinhaColuna,&(idThreadsLinhaColuna[i]) );
        }
    }
    
    for(i=0;i<18;i++){
        if(i<9){
            pthread_join(threadsLinhaColuna[i],NULL);
            pthread_join(threadsQuadrado[i],NULL);
        }
        else{
            pthread_join(threadsLinhaColuna[i],NULL);
        }
    }
    free(threadsLinhaColuna);
    free(threadsQuadrado);
    free(idThreadsLinhaColuna);
    free(idThreadsQuadrado);
    threadsLinhaColuna=NULL;threadsQuadrado=NULL;idThreadsLinhaColuna=NULL;idThreadsQuadrado=NULL;
    printf("%s\n",valido?"VALIDO":"INVALIDO");
    pthread_exit(NULL);//thread da main
    return 0;
}