#include <stdio.h>
#include "mpi.h"
#include <stdlib.h> 
#include <string.h> 

#define VET_SIZE  1000 // Trabalho Final com o valores 100.000 e 1.000.000
//#define DEBUG 1

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

int main(int argc, char **argv){
   
    MPI_Status status;     // Message status
    double t1, t2;         // Count exectuion time 
	
    int my_rank;           // Process ID
    int proc_n;            // Number of process
	int aux;			   // Store auxiliary values
	

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_n);

    t1 = (my_rank == 0)? MPI_Wtime(): 0; // Time counter on process 0

	#define LAST (proc_n - 1)		// Last process
    int i;							// for counters
	unsigned char end = 0;        	// Control the main loop
	unsigned char k;				// Process counter
    int psize = VET_SIZE/proc_n;	// tamanho do vetor do processo
	int pTochange = psize/25;		// Size to change date with others
	int left ,right;				// Auxiliar vector for change operation
	unsigned char vet_ctrl[proc_n]; // Control vector 
	int *vector = (int *)malloc((pTochange + psize) * sizeof(int)); // Data vector with extra size
	
	left = (my_rank !=0) ? my_rank - 1:0 ;			// My left neighboor
	right = (my_rank < LAST) ? my_rank + 1: LAST;	// My right neighboor
	memset(vet_ctrl, 0, sizeof(vet_ctrl));

    for (i = 0; i < psize; i++)
	{
		vector[i] = (proc_n - my_rank) * psize - i;
	}

	#ifdef DEBUG
	printf("[%d]vector: ", my_rank);
	for (i = 0; i < psize; i++)
		printf("%d ", vector[i]);
	printf("\n\n");	
	#endif 
 
	while(!end) {
    	
		//******************************
		//#1. Local ordenation
		//******************************

		bs(psize, vector);

		// Send to the right, if im not the last process.
		if (my_rank != LAST){
			MPI_Send(&vector[psize - 1], 1, MPI_INT, right, 0, MPI_COMM_WORLD);		
			
		}
		// Recieve from left if im not 0 process.
		if (my_rank != 0){
			MPI_Recv(&aux, 1, MPI_INT, left, 0, MPI_COMM_WORLD, &status);
			/* If the left element size is less then my least element 
			set ordenetion vector, if not clear the vector.
				*/
			if (aux  < vector[0]){
				vet_ctrl[my_rank] = 1;
			}
			else{
				vet_ctrl[my_rank] = 0;
			}
		}
		// Exeption case
		if (my_rank == 1){
			MPI_Send(vector, 1, MPI_INT, left, 0, MPI_COMM_WORLD);
		}
		// Exception case, recive from 1
		if (my_rank == 0){
			MPI_Recv(&aux, 1, MPI_INT, right, 0, MPI_COMM_WORLD, &status);
			/* If the element size from second process is greatter then my last element 
			set ordenetion vector, if not clear the vector.
				*/
			if(aux > vector[psize-1] ){
				vet_ctrl[my_rank] = 1;
			}else{
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
		// Stop condition, verify if all neighbors are ordenate
		k = 0;
		for (i = 0; i < (proc_n*2)-2; i++){
			if (vet_ctrl[i] == 1) {
				k++;
			}
		}
		if (k == proc_n){
			end = 1;
			break;
		}

		//******************************
		// #3. Converge
		//******************************

		// Send to left if im not the first process.
		if (my_rank != 0) {
			/* first, send my portion, and wait to recive from neightbor if im no the least process*/
				MPI_Send(vector, pTochange, MPI_INT, left, 0, MPI_COMM_WORLD); // send to left
		}		
	
		if (my_rank != LAST){
			// Wait for right to send
			MPI_Recv(&vector[psize], pTochange, MPI_INT, right, 0, MPI_COMM_WORLD, &status); 
			// Ordenate vector without my left portion
			bs(psize,vector + pTochange);
			// Send back the right portion
			MPI_Send(&vector[psize], pTochange, MPI_INT, right, 0, MPI_COMM_WORLD);		
		}
		if (my_rank !=0){
			MPI_Recv(vector, pTochange, MPI_INT, left, 0, MPI_COMM_WORLD, &status); 
		}

	} // End While
	// At this point, our vector is ordenate like a charm!!!

	// End time.
	t2 = (my_rank == 0)?MPI_Wtime():0;

	#ifdef DEBUG
	printf("[%d]Vector: ", my_rank);
	for (i = 0; i < psize; i++)
		printf("%d ", vector[i]);
	printf("\n\n");
	#endif
	
	if (my_rank == 0)
	{
		printf("Elapsed: %.4f s\n\n", t2 - t1);
	}

	free(vector);
	MPI_Finalize();
	return 0;
}