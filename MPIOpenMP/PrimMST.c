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
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Edge {
    int start, end, weight;
} Edge;

int findEdgeWithMinKey(bool mstSet[], Edge edges[], int edgeCount) {
    int min = INT_MAX, min_index = -1;

    for (int i = 0; i < edgeCount; i++) {
        if ((mstSet[edges[i].start] || mstSet[edges[i].end]) && edges[i].weight < min) {
            if (!(mstSet[edges[i].start] && mstSet[edges[i].end])) {
                min = edges[i].weight;
                min_index = i;
            }
        }
    }

    return min_index;
}

void printMST(Edge parent[], int rows) {
    int weight = 0;
    printf("MST Edges \t\t\tWeight\n");
    for (int i = 1; i < rows; i++)
        printf("%d - %d \t\t\t%d \n", parent[i].start, parent[i].end, parent[i].weight);

    for (int i = 1; i < rows; i++) {
        weight += parent[i].weight;
    }
    printf("\nTotal Weight:%d\n", weight);
}

Edge *primMST(Edge edges[], int edgeCount, int rows) {
    Edge* mst;
    mst = (Edge*)malloc((rows - 1) * sizeof(Edge));
    bool mstSet[rows];

    for (int i = 0; i < rows; i++) {
        mstSet[i] = false;
    }

    mstSet[0] = true;

    for (int count = 0, i = 0; count < rows - 1; count++) {
        int minIndex = findEdgeWithMinKey(mstSet, edges, edgeCount);
        mstSet[edges[minIndex].start] = true;
        mstSet[edges[minIndex].end] = true;
        mst[i++] = edges[minIndex];
    }

    //printf("\nTotal Weight:%d\n", weight);
    return mst;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file_path> <output_file_path>\n", argv[0]);
        return 1;
    }
    char *inputFilePath = argv[1];

    FILE *file = fopen(inputFilePath, "r");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        return 1;
    }

    int rows= atoi(argv[2]);

    int total_elements = rows * rows;
    int *matrix = (int *)malloc(total_elements * sizeof(int));

    for (int i = 0; i < total_elements; i++) {
        if(fscanf(file, "%d", &matrix[i])!=1){
            perror("Errore nell'apertura del file");
            return 1;
        };
    }

    fclose(file);

    struct Edge *edges = (struct Edge *)malloc((rows * (rows - 1) / 2) * sizeof(struct Edge));
    int edgeCount = 0,weigth;

    for (int i = 0; i < rows; i++) {
        for (int j = i + 1; j < rows; j++) {
            if (matrix[i * rows + j] != 0) {
                edges[edgeCount].start = i;
                edges[edgeCount].end = j;
                edges[edgeCount].weight = matrix[i * rows + j];
                edgeCount++;
            }
        }
    }
    free(matrix);

    /*FILE *outputFile = fopen(outputFilePath, "a+");
    if (outputFile == NULL) {
        perror("Errore nell'apertura del file di output");
        return 1;
    }*/
    double times[10];
    double elapsed;
    double total=0;
    Edge* mst;
    for(int i=0;i<10;i++){
    volatile clock_t start_time = clock();
    mst=primMST(edges, edgeCount, rows);
    volatile clock_t end_time = clock();
    elapsed=end_time-start_time;
    times[i]=elapsed;
    }
    for(int i=0;i<10;i++){
        total+=times[i];
    }
    double cpu_time_used = ((double)(total/10)) / CLOCKS_PER_SEC;
    printf("Sequential Time : %f\n", cpu_time_used);
    printf("Total weigth: %d",weigth);
    free(edges);

    return 0;
}
