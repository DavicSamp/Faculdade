#include <stdio.h>
#include <stdlib.h>

#define VECTOR_SIZE 65536  // 2^16

// Função para carregar o vetor a partir de um arquivo CSV
void load_vector_from_csv(const char *filename, int *vector, int size) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        if (fscanf(file, "%d,", &vector[i]) != 1) {
            fprintf(stderr, "Erro ao ler o arquivo CSV\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    int rank, size;
    int *vector;
    int local_found = 0;
    int global_found = 0;
    int element_to_find = 603461;  // Elemento a ser buscado

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Alocar memória para o vetor
    vector = (int *)malloc(VECTOR_SIZE * sizeof(int));
    if (vector == NULL) {
        perror("Erro ao alocar memória");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Processo 0 carrega o vetor do arquivo CSV
    if (rank == 0) {
        load_vector_from_csv("vetor1.csv", vector, VECTOR_SIZE);
    }

    // Broadcast do vetor para todos os processos
    MPI_Bcast(vector, VECTOR_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    // Dividir o vetor entre os processos
    int local_size = VECTOR_SIZE / size;
    int start = rank * local_size;
    int end = start + local_size;

    // Busca local pelo elemento
    for (int i = start; i < end; i++) {
        if (vector[i] == element_to_find) {
            local_found = 1;
            break;
        }
    }

    // Reduzir o resultado para o processo 0
    MPI_Reduce(&local_found, &global_found, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    // Processo 0 imprime o resultado
    if (rank == 0) {
        if (global_found) {
            printf("Elemento %d encontrado no vetor.\n", element_to_find);
        } else {
            printf("Elemento %d não encontrado no vetor.\n", element_to_find);
        }
    }

    // Liberar memória e finalizar MPI
    free(vector);
    MPI_Finalize();

    return 0;
}
