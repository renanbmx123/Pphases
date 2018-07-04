					Enunciado 3o Trabalho MPI: Fases Paralelas (FP)

O objetivo do trabalho é implementar, usando a biblioteca MPI, uma versão paralela seguindo o modelo fases paralelas, de um programa que ordena um grande vetor usando o algortimo Bubble Sort (o programa sequencial está no final desta especificação). Após implementado, o programa deve ser executado no cluster Grad com 2 nós para realização das medições de desempenho para um tamanho de vetor de 1.000.000 elementos (sem os prints de tela). Cada grupo (de no máximo dois integrantes) deve entregar um relatório em .pdf de uma página com a análise dos resultados e o código anexado (seguir modelo proposto).

Cada um dos processos é responsável por 1/np do vetor (neste caso 1/16 ou 1/32 com HT), que já pode ser gerado internamente em cada processo, sem a necessidade de gerar em um único processo e depois distribuir entre os restantes. Depois, o processamento segue as seguintes fases, que são executadas em paralelo por todos os processos até a conclusão da ordenação (por isso o nome do modelo):

    Ordenação local: cada processo ordena a sua parte do vetor global (usar o código da rotina BS abaixo);

    É feita uma verificação distribuída para identificar se o vetor global esta ordenado: cada processo envia o seu maior valor para o vizinho da direita, este compara com o seu menor valor e verifica se os dois processos estão ordenados entre si. Como todos os processos fazem esta operação, cada um sabe se está ordenado em relação ao vizinho da esquerda. Esta informação é distribuída para todos os processos com uma operação de comunicação coletiva (Broadcast). Se todos estiverem ordenados entre si, todos terminam;

    Se não, cada processo troca uma parte dos seus valores mais baixos com os mais altos do vizinho da esquerda. A ideia é empurrar os valores que estão foram de lugar para os processos corretos, e volta para a fase 1.

Os itens para avaliação são:

    execução da versão sequencial do algoritmo Bubble Sort para o vetor inteiro (inicializar com dados em ordem decrescente);
    implementação da versão paralela SPMD do algoritmo MPI descrito acima seguindo o modelo de fases paralelas;
    medição dos tempos de execução para a versão sequencial em uma máquina qualquer do aluno ou laboratório e da versão paralela para 1.000.000 elementos (usando 2 nós exclusivos da máquina grad totalizando 16/32 (HT) processos);
    cálculo do speed up e da eficiência para os casos de teste;
    análise do desempenho deste modelo em relação ao de divisão e conquista para a mesma tarefa (tamanho de vetor);
    clareza do código (utilização de comentários e nomes de variáveis adequadas);
    relatório no formato .pdf com uma página (coluna dupla)

#include <stdio.h>
#include <stdlib.h>

#define DEBUG 1            // comentar esta linha quando for medir tempo
#define ARRAY_SIZE 40      // trabalho final com o valores 10.000, 100.000, 1.000.000

void bs(int n, int * vetor)
{
    int c=0, d, troca, trocou =1;

    while (c < (n-1) & trocou )
        {
        trocou = 0;
        for (d = 0 ; d < n - c - 1; d++)
            if (vetor[d] > vetor[d+1])
                {
                troca      = vetor[d];
                vetor[d]   = vetor[d+1];
                vetor[d+1] = troca;
                trocou = 1;
                }
        c++;
        }
}

int main()
{
    int vetor[ARRAY_SIZE];
    int i;

    for (i=0 ; i<ARRAY_SIZE; i++)              /* init array with worst case for sorting */
        vetor[i] = ARRAY_SIZE-i;
   

    #ifdef DEBUG
    printf("\nVetor: ");
    for (i=0 ; i<ARRAY_SIZE; i++)              /* print unsorted array */
        printf("[%03d] ", vetor[i]);
    #endif

    bs(ARRAY_SIZE, vetor);                     /* sort array */

    #ifdef DEBUG
    printf("\nVetor: ");
    for (i=0 ; i<ARRAY_SIZE; i++)                              /* print sorted array */
        printf("[%03d] ", vetor[i]);
    #endif

    return 0;
}
