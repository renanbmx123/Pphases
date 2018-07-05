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

#define DEBUG1 1
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

    t1 = (my_rank == 0)? MPI_Wtime(): 0;

	#define LAST (proc_n - 1)		// Last process
    int i;							// for counters
	
	unsigned char k;				// contador de processos
    int psize = VET_SIZE/proc_n;	// tamanho do vetor do processo
	int pTochange = psize/25;		// Size to change date with others
	int left ,right;				// Auxiliar vector for change operation
	
	
	
	/* If rank !=0 or < proc_n, two space are availaible to set flag control. This is 
	 used for know, if im ordenated with my left and right neighbor. Use (my_rank+(my_rank - 1))
	 to left neighbor or (my_rank + my_rank to right neighbor) */
	//int aux[pTochange];				
	
	left = (my_rank !=0) ? my_rank -1:0 ;
	right = (my_rank < LAST) ? my_rank + 1: LAST;
	unsigned char vet_ctrl[(proc_n * 2)-2]; // Control vector 
	
    //int *vetor = (int *)malloc( (psize + pTochange) * sizeof(int)); // Data vector with extra size
	int vetor[psize + pTochange];
	
	memset(vet_ctrl, 0, sizeof(vet_ctrl));
    for (i = 0; i < psize; i++)
	{
		vetor[i] = (proc_n - my_rank) * psize - i;
	}
	
	
	#ifdef DEBUG1
	printf("[%d]Vetor: ", my_rank);
	for (i = 0; i < psize; i++)
		printf("%d ", vetor[i]);
	printf("\n\n");	
	#endif 
 
	while(!end) {
    	
		//******************************
		//#1. First we ordenate our vector
		//******************************
		
		#ifdef DEBUG
		printf("[%d]Vetor: ", my_rank);
		for (i = 0; i < psize; i++)
			printf("%d ", vetor[i]);
		printf("\n\n");	
		#endif 
		MPI_Barrier(MPI_COMM_WORLD);
		bs(psize, vetor);
		// Send to the right, if im not the last process.
		if (my_rank != LAST)
		{
			MPI_Send(&vetor[psize-1], 1, MPI_INT, right, RIGHT, MPI_COMM_WORLD);		
		}
		// Exeption case
		if (my_rank == 1)
		{
			MPI_Send(&vetor[0], 1, MPI_INT, left, LEFT, MPI_COMM_WORLD);
		}

		// Exception case
		if (my_rank == 0)
		{
			MPI_Recv(&vetor[psize], 1, MPI_INT, right, LEFT, MPI_COMM_WORLD, &status);
			if(vetor[psize] > vetor[psize-1] )
			{
				//printf("[0] = %d <- [1] = %d \n",vetor[psize-1],vetor[psize]);
				vet_ctrl[my_rank] = 1;
			}else
			{
				//printf("[0] = %d <- [1] = %d \n",vetor[psize-1],vetor[psize]);
				vet_ctrl[my_rank] = 0;
			}
		}

		// Recieve from left if im not 0 process.
		if (my_rank != 0)
		{
			MPI_Recv(&vetor[psize], 1, MPI_INT, left, RIGHT, MPI_COMM_WORLD, &status);
			// se o Maior recebido, for menor que meu Menor, OK
			if (vetor[0]  < vetor[psize])
			{
				vet_ctrl[my_rank] = 1;
			}
			else
			{
				vet_ctrl[my_rank] = 0;
			}
		}

		//******************************
		//#2. Bcast status of ordenation
		//******************************

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

		//******************************
		// #3. Converge
		//******************************

		// Send to left if im not the first process.
		if (my_rank != 0) 
		{
			/* first, send my portion, and wait to recive from neightbor if im no the least process*/
				MPI_Send(&vetor[0], pTochange, MPI_INT, left, TAG, MPI_COMM_WORLD); // send to left
		}		
	
		if (my_rank != proc_n - 1){
			if(vet_ctrl[my_rank] == 0 ) //Verify with the vet_ctrl if i was ordenate with my left neighbor, if not send to exchange.
			{
				// Im not the last process, than i wait to recive from right with is send from left.
				// Wait for right to send
				MPI_Recv(&vetor[psize], pTochange, MPI_INT, right, TAG, MPI_COMM_WORLD, &status); 
				// Ordenate all vector, include the portion we recive
				bs(psize+pTochange,vetor);
				// Send back my portion
				MPI_Send(&vetor[psize], pTochange, MPI_INT, right, TAG, MPI_COMM_WORLD);
				if (my_rank != 0)
				{
					MPI_Recv(&vetor[psize], pTochange, MPI_INT, left, TAG, MPI_COMM_WORLD, &status); 
					bs(psize+pTochange,vetor);
				}
						
			}
		}
		

	} // End While
	
	// End time.
	t2 = (my_rank == 0)?MPI_Wtime():0;
	

	#ifdef DEBUG
	if (my_rank == 0)
	{
		printf("=======================================\n\n");

		printf("[%d]Ctrl: ", my_rank);
		for (i = 0; i < proc_n; i++)
			printf("%d ", vet_ctrl[i]);
		printf("\n\n");
	}
	#endif 

	#ifdef DEBUG
	printf("[%d]Final: ", my_rank);
	for (i = 0; i < 4; i++)
		printf("%d ", vetor[i]);
	printf("\n\n");
	#endif

	if (my_rank == 0)
	{
		printf("Elapsed: %.4f s\n\n", t2 - t1);
	}

	free(vetor);
	MPI_Finalize();
	return 0;
}