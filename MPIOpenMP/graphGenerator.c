/*
 * Course: High Performance Computing 2023/2024
 *
 * Lecturer: Francesco Moscato  fmoscato@unisa.it * 
 * Student: Giso Alfonso 0622701842   a.giso@studenti.unisa.it
 * 
 * Provide a parallell version of the Prim's algorithm to find the minimum MST of a graph.
 * 
 * The implementation MUST use an hibrid  of MPI and openMP.
 * 
 * This code was created from the following, visible through the link:
 * 
 * https://www.geeksforgeeks.org/prims-minimum-spanning-tree-mst-greedy-algo-5/
 * 
 * Copyright (C) 2024  Sabato Fasulo
 * 
 * This file is part of CommonAssignmentMPIOpenMP
 * 
 * CommonAssignmentMPIOpenMP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * CommonAssignmentMPIOpenMP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CommonAssignmentMPIOpenMP . If not, see <http://www.gnu.org/licenses/>.
 * 
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generateGraph(int vertices, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file '%s'\n", filename);
        return;
    }
    srand(time(NULL));

    // Creazione della matrice di adiacenza
    int** adjacencyMatrix = (int**)malloc(vertices * sizeof(int*));
    for (int i = 0; i < vertices; ++i) {
        adjacencyMatrix[i] = (int*)calloc(vertices, sizeof(int));
    }

    // Generazione del grafo non orientato aciclico con pesi casuali
    for (int i = 0; i < vertices; ++i) {
        // Impostazione di un set per tenere traccia dei nodi già connessi
        int* connectedNodes = (int*)calloc(vertices, sizeof(int));

        // Modifica: scegli un numero casuale di nodi connessi tra 1 e vertices/2
        int adjSize = 1 + rand() % (vertices / 2);

        for (int j = 0; j < adjSize; ++j) {
            // Scegli un nodo casuale che non è già stato connesso
            int connectedNode;
            do {
                connectedNode = rand() % vertices;
            } while (connectedNode == i || connectedNodes[connectedNode]);

            // Verifica se l'aggiunta dell'arco forma un ciclo
            while (connectedNodes[i] || connectedNodes[connectedNode]) {
                connectedNode = rand() % vertices;
            }

            // Assegna un peso casuale all'arco
            int weight = rand() % 10 + 1; // Ad esempio, pesi da 1 a 10

            // Aggiungi l'arco al grafo con il peso assegnato
            adjacencyMatrix[i][connectedNode] = weight;
            adjacencyMatrix[connectedNode][i] = weight;

            // Segna il nodo come connesso
            connectedNodes[connectedNode] = 1;
        }

        free(connectedNodes);
    }
    // Scrittura della matrice di adiacenza nel file
    for (int i = 0; i < vertices; ++i) {
        for (int j = 0; j < vertices; ++j) {
            fprintf(file, "%d ", adjacencyMatrix[i][j]);
        }
        fprintf(file, "\n");
    }

    // Deallocazione della memoria
    for (int i = 0; i < vertices; ++i) {
        free(adjacencyMatrix[i]);
    }
    free(adjacencyMatrix);

    fclose(file);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage:\n\t%s [vertices] [fileName]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int vertices = atoi(argv[1]);
    char* fileName = argv[2];

    generateGraph(vertices, fileName);

    exit(EXIT_SUCCESS);
}
