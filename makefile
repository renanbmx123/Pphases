#Para escrever comentários ##
############################# Makefile ##########################
Pphases: Pphases.o 
        # O compilador faz a ligação entre os dois objetos
	mpicc -o Pphases Pphases.o
#-----> Distancia com o botão TAB ### e não com espaços
Pphases.o: Pphases.c 
	mpicc -o Pphases.o -c Pphases.c -W -Wall
clean:
	rm -rf *.o
mrproper: clean
	rm -rf Pphases
