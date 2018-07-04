/*
 * TPP3 - FASES PARALELAS (FP)
 * 
 * 1. processamento local
 * 2. teste de condição de parada
 * 3. troca para convergencia
 * 
 * broadcast cuidar, nao tem Recv
 
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

#define VET_SIZE  1000 // Trabalho Final com o valores 100.000 e 1.000.000

//#define DEBUG 1
#define LEFT 2
#define RIGHT 3
#define TAG 4
#define CHANGE 5


// BSORT
void bs(int n, int *vetor)
{
    int c = 0, d, troca, trocou = 1;

    while ((c < (n - 1)) & trocou)
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

void my_bcast(void* data, int count, MPI_Datatype datatype, int root,
              MPI_Comm communicator) {
  int world_rank;
  MPI_Comm_rank(communicator, &world_rank);
  int world_size;
  MPI_Comm_size(communicator, &world_size);

  if (world_rank == root) {
    // If we are the root process, send our data to everyone
    int i;
    for (i = 0; i < world_size; i++) {
      if (i != world_rank) {
        MPI_Send(data, count, datatype, i, 0, communicator);
      }
    }
  } else {
    // If we are a receiver process, receive the data from the root
    MPI_Recv(data, count, datatype, root, 0, communicator,
             MPI_STATUS_IGNORE);
  }
}


int main(int argc, char **argv)
{
    MPI_Status status;     // Message status
    double t1, t2;         // Count exectuion time 
	
    int my_rank;           // Process ID
    int proc_n;            // Number of process
	
   
	unsigned char end = 0;        // Control the main loop

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    t1 = MPI_Wtime();
	
    int i;					// for counters
	
	unsigned char k;				// contador de processos
    int psize = VET_SIZE/proc_n;	// tamanho do vetor do processo
	int pTochange = psize/25;		// Size to change date with others
	int left ,right;				// Auxiliar vector for change operation
	
	/* If rank !=0 or < proc_n, two space are availaible to set flag control. This is 
	 used for know, if im ordenated with my left and right neighbor. Use (my_rank+(my_rank - 1))
	 to left neighbor or (my_rank + my_rank to right neighbor) */
	//int aux[pTochange];				
	
	left = (my_rank !=0) ? my_rank -1:0 ;
	right = (my_rank < proc_n - 1) ? my_rank + 1: proc_n - 1;
	unsigned char vet_ctrl[(proc_n * 2)-2]; // Control vector 
	
    int *vetor = (int *)malloc( (psize + pTochange) * sizeof(int)); // Data vector with extra size
	
	memset(vet_ctrl, 0, sizeof(vet_ctrl));
    for (i = 0; i < psize; i++)
	{
		vetor[i] = (proc_n - my_rank) * psize - i;
	}
	#ifdef DEBUG
		printf("[%d]Vetor Inicial: ", my_rank);
	for (i = 0; i < psize; i++)
		printf("%d ", vetor[i]);
	printf("\n\n");	
	#endif 
	
	MPI_Barrier(MPI_COMM_WORLD);	// Aguarda todos os processos chegarem ate aqui

	#ifdef DEBUG
	if(my_rank == 0) {
		printf("[%d]Ctrl: ", my_rank);
		for (i = 0; i < proc_n; i++)
			printf("%d ", vet_ctrl[i]);
		printf("\n\n");
		printf("=======================================\n\n"); 
	}
	#endif 
	
	MPI_Barrier(MPI_COMM_WORLD);	// Aguarda todos os processos chegarem ate aqui
	
	while(!end) {
    
		// 1. First we ordenate our local  vector
		bs(psize, vetor);

		#ifdef DEBUG
		printf("[%d]Vetor: ", my_rank);
		for (i = 0; i < psize; i++)
			printf("%d ", vetor[i]);
		printf("\n\n");
		#endif
	
		// se nao for Ultimo, envio maior valor pra Direita   O -> O
		if (my_rank != proc_n - 1)
		{
			MPI_Send(&vetor[psize], 1, MPI_INT, right, TAG, MPI_COMM_WORLD);		
		}
		if (my_rank == 1)
		{
			MPI_Send(&vetor[0], 1, MPI_INT, left, TAG, MPI_COMM_WORLD);
		}
		// se nao for Primeiro, recebo maior valor da Esquerda
		if (my_rank != 0)
		{
			MPI_Recv(&vetor[psize+1], 1, MPI_INT, left, TAG, MPI_COMM_WORLD, &status);
			// se o Maior recebido, for menor que meu Menor, OK
			if (vetor[0] < vetor[psize+1])
			{
				vet_ctrl[my_rank] = 1;
			}
			else
			{
				vet_ctrl[my_rank] = 0;
			}
		}
		else
		{
			MPI_Recv(&vetor[psize+1], 1, MPI_INT, right, TAG, MPI_COMM_WORLD, &status);	
			if (vetor[psize] > vetor[psize+1])
			{
				vet_ctrl[my_rank] = 1;
			}
			else
			{
				vet_ctrl[my_rank] = 0;
			}
		}
		
		// BROADCAST
		for (i = 0; i < proc_n; i++)
		{
			MPI_Bcast(&vet_ctrl[i], 1, MPI_UNSIGNED_CHAR, i, MPI_COMM_WORLD);
		}
		MPI_Barrier(MPI_COMM_WORLD);
		
		// Stop condition, verify if all neighbors are ordenate
		k = 0;
		for (i = 0; i < (proc_n*2)-2; i++)
		{
			if (vet_ctrl[i] == 1) 
			{
				k++;
			}
		}

		if (k == proc_n)
		{
			end = 1;
			break;
		}
		
		// ## 3. TROCA PARA CONVERGENCIA ##
		
		// se nao for Primeiro, envio meu menor valor pra Esquerda  O <- O
        if (my_rank != 0) 
		{
			if(vet_ctrl[my_rank] != 1 ) //Verify with the vet_ctrl if i was ordenate with my left neighbor, if not send to exchange.
				MPI_Send(&vetor[0], pTochange, MPI_INT, left, LEFT, MPI_COMM_WORLD); // send to left
		}
		if(my_rank != proc_n -1){
			// Recieve from left
			if(vet_ctrl[right] != 1 ){
				MPI_Recv(&vetor[psize+1], pTochange, MPI_INT, right, RIGHT, MPI_COMM_WORLD, &status); 
				bs((psize+pTochange),vetor);
			}
		}
		MPI_Send(&vetor[psize - pTochange], pTochange, MPI_INT, right, CHANGE, MPI_COMM_WORLD);
		
		MPI_Barrier(MPI_COMM_WORLD); //To synchronize all processes at this point

		

	}
	// FINALIZA CODIGO

	t2 = MPI_Wtime();
	
	MPI_Barrier(MPI_COMM_WORLD); // Aguarda todos os processos chegarem ate aqui

	if (my_rank == 0)
	{
		printf("=======================================\n\n");

		printf("[%d]Ctrl: ", my_rank);
		for (i = 0; i < proc_n; i++)
			printf("%d ", vet_ctrl[i]);
		printf("\n\n");
	}

	MPI_Barrier(MPI_COMM_WORLD); // Aguarda todos os processos chegarem ate aqui

	printf("[%d]Final: ", my_rank);
	for (i = 0; i < 4; i++)
		printf("%d ", vetor[i]);
	printf("\n\n");

	MPI_Barrier(MPI_COMM_WORLD); // Aguarda todos os processos chegarem ate aqui
	
	if (my_rank == 0)
	{
		printf("MPI ");
		printf("Bubble ");
		printf("Elapsed: %.4f s\n\n", t2 - t1);
	}

	free(vetor);
	MPI_Finalize();
	return 0;
}