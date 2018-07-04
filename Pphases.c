/*
 * TPP3 - FASES PARALELAS (FP)
 * 
 * 1. processamento local
 * 2. teste de condição de parada
 * 3. troca para convergencia
 * 
 * broadcast cuidar, nao tem Recv
 * 
 * sugestões
 * 
 * 1 - comparacao FP com DC (bsort e qsort)
 * 2 - % troca x convergencia/tempo (contar fases)
 * 3 - otimizacoes - juntar fase des
 * nao fazer tudo
*/

#include <stdio.h>
#include "mpi.h"
#include <stdlib.h> //malloc atoi
#include <string.h> //memcpy

#define VET_SIZE  8 // Trabalho Final com o valores 100.000 e 1.000.000

#define TAG 1
#define BSORT 2
#define QSORT 17

// BSORT
void bs(int n, int *vetor)
{
    int c = 0, d, troca, trocou = 1;

    while (c < (n - 1) & trocou)
    {
        trocou = 0;
        for (d = 0; d < n - c - 1; d++)
            if (vetor[d] > vetor[d + 1])
            {
                troca = vetor[d];
                vetor[d] = vetor[d + 1];
                vetor[d + 1] = troca;
                trocou = 1;
            }
        c++;
    }
}

// QSORT
int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

//if (argv[1] != NULL)
//    delta = atoi(argv[1]);
//printf("ARGV: %d\n", delta);

main(int argc, char **argv)
{
    MPI_Status status;     // Status da Mensagem
    double t1, t2;         // Armazena a diferença de tempo
	
    int my_rank;           // ID do processo
    int proc_n;            // Total de processos
    int algoritmo = QSORT; // Algoritmo de ordenação escolhido

    int para = 0;        // Condicao de todos os processos prontos

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    t1 = MPI_Wtime();
	
	// ############################
	// :: INSTANCIA AS VARIAVEIS ::

    int i, j, k;					// contadores
	int aux1, aux2;					// valor de troca entre processos
    int psize = VET_SIZE/proc_n;	// tamanho do vetor do processo
	
	// ToDo: Testar Se a divisao do vetor nao der valor inteiro
	/* if(my_rank == 0)
	 	if(VET_SIZE % proc_n != 0)
	 		psize++;
    */
	
	// vetor de controle
	int vet_ctrl[proc_n];	
	// vetor de dados
    int *vetor = (int *) malloc(psize * sizeof(int));	
	
    memset(vet_ctrl, 0, sizeof(vet_ctrl));
    for(i = 0; i < psize; i++) {
        vetor[i] = (proc_n - my_rank)*psize - i;
    }
	
	printf("[%d]Vetor Inicial: ", my_rank);
	for (i = 0; i < psize; i++)
		printf("%d ", vetor[i]);
	printf("\n\n");
	
	if(my_rank == 0) {
		printf("[%d]Ctrl: ", my_rank);
		for (i = 0; i < proc_n; i++)
			printf("%d ", vet_ctrl[i]);
		printf("\n\n");
		printf("=======================================\n\n"); 
	}
	
	vet_ctrl[0] = 1;
	
	
	while(!para) {
    //for(i = 0; i < 4; i++) {
		
		// ## 1. PROCESSAMENTO LOCAL ##
		
		if (algoritmo == QSORT)
			qsort(vetor, psize, sizeof(int), compare);
		else if (algoritmo == BSORT)
			bs(psize, vetor);
		
		// ## 2. TESTE DE CONDICAO DE PARADA ##
		
		// se nao for Ultimo, envio maior valor pra Direita   O -> O
		if(my_rank != proc_n-1) {
			MPI_Send(&vetor[psize-1], 1, MPI_INT, my_rank+1, TAG, MPI_COMM_WORLD);
		}
		// se nao for Primeiro, recebo maior valor da Esquerda
		if(my_rank != 0) {
			MPI_Recv(&aux1, 1, MPI_INT, my_rank-1, TAG, MPI_COMM_WORLD, &status);
		}
		// se o Maior recebido, for menor que meu Menor, OK
		if(aux1 < vetor[0]) {
			vet_ctrl[my_rank] = 1;
		}
		else {
			//vet_ctrl[my_rank] = 0;
		}
		
		
		printf("[%d]Vetor: ", my_rank);
		for (i = 0; i < psize; i++)
			printf("%d ", vetor[i]);
		printf("\n\n");
		
		printf("[%d]Ctrl: ", my_rank);
		for (i = 0; i < proc_n; i++)
			printf("%d ", vet_ctrl[i]);
		printf("\n\n");
		
		
		// BROADCAST
		for (i = 0; i < proc_n; i++) {
			MPI_Bcast(&vet_ctrl[i], 1, MPI_INT, i, MPI_COMM_WORLD);
		}
		// CONDICAO DE PARADA
		k = 0;
		for (i = 0; i < proc_n; i++) {
			if(vet_ctrl[i] == 1) {
				k++;
			}
		}
		if(k == proc_n) {
			para = 1;
		}
		
		/*
		printf("[%d]Vetor Ctrl: \n", my_rank);
		for (i = 0; i < proc_n; i++)
			printf("%d ", vet_ctrl[i]);
		printf("\n\n");
		*/
		
		// ## 3. TROCA PARA CONVERGENCIA ##
		
		// se nao for Primeiro, envio meu menor valor pra Esquerda  O <- O
        if(my_rank != 0) {
            MPI_Send(&vetor[0], 1, MPI_INT, my_rank-1, TAG, MPI_COMM_WORLD);
        }
		// se nao for Ultimo, recebo o menor valor da Direita
        if(my_rank != proc_n-1) {
            MPI_Recv(&aux1, 1, MPI_INT, my_rank+1, TAG, MPI_COMM_WORLD, &status);
			// se o Menor recebido, for menor que meu Maior, Jogo no vetor
			if(aux1 < vetor[psize-1]) {
				
				aux2 = vetor[psize-1];
				vetor[psize-1] =  aux1;
				
				if (algoritmo == QSORT)
					qsort(vetor, psize, sizeof(int), compare);
				else if (algoritmo == BSORT)
					bs(psize, vetor);
				
				// envio maior pra Direita
				MPI_Send(&aux2, 1, MPI_INT, my_rank+1, TAG, MPI_COMM_WORLD);
			}
        }
		// se nao for Primeiro, recebo os Maiores valores da Esquerda
        if(my_rank != 0) {
            MPI_Recv(&aux1, 1, MPI_INT, my_rank-1, TAG, MPI_COMM_WORLD, &status);
			// se o Maior recebido, for maior que meu Menor, Jogo no vetor
			if(aux1 > vetor[0]) {
				// Sobreescreve o primeiro valor da troca
				vetor[0] = aux1;
			}
		}
	}
	
	// FINALIZA CODIGO
	
    t2 = MPI_Wtime();
	
	printf("[%d]Final: ", my_rank);
	for (i = 0; i < psize; i++)
		printf("%d ", vetor[i]);
	printf("\n\n");
	
	if(my_rank == 0) {
		printf("[%d]Ctrl: ", my_rank);
		for (i = 0; i < proc_n; i++)
			printf("%d ", vet_ctrl[i]);
		printf("\n\n");
	
		printf("MPI ");
		if (algoritmo == BSORT)
			printf("Bubble ");
		else if (algoritmo == QSORT)
			printf("Quick ");
		printf("Elapsed: %.4f s\n\n", t2 - t1);
	}
    
    free(vetor);
    MPI_Finalize();
    //return 0;
}