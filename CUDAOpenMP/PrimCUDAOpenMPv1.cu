/*
 * Course: High Performance Computing 2023/2024
 *
 * Lecturer: Francesco Moscato  fmoscato@unisa.it * 
 * Student: Giso  Alfonso  0622701842   a.giso@studenti.unisa.it
 * 
 * Provide a parallell version of the Prim's algorithm to find the minimum MST of a graph.
 * 
 * The implementation MUST use an hibrid  of CUDA and openMP.
 * 
 * This code was created from the following, visible through the link:
 * 
 * https://www.geeksforgeeks.org/prims-minimum-spanning-tree-mst-greedy-algo-5/
 * 
 * Copyright (C) 2024  Alfonso Giso
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

#define BLOCK_SIZE 256

typedef struct Edge {
    int start, end, weight;
} Edge;


__device__ int findEdgeWithMinKey(bool *mstSet, Edge *edges, int edgeCount, int rows) {
    int min = INT_MAX, min_index = -1;
    #pragma omp parallel for
    for (int i = 0; i < edgeCount; i++) {
        // Verifica se almeno uno dei nodi collegati è già nell'insieme MST
        if ((mstSet[edges[i].start] || mstSet[edges[i].end]) &&
            edges[i].weight < min && 
            ((mstSet[edges[i].start] && !mstSet[edges[i].end]) || 
             (mstSet[edges[i].end] && !mstSet[edges[i].start]))) 
             {
            min = edges[i].weight;
            min_index = i;
        }
    }
    //printf("Sono il thread: %d e l'indice minimo che ho trovato è:%d\n",threadIdx.x,min_index);

    return min_index;
}


__device__ int findGlobalMinIndex(Edge *edges, int *localMinIndices, int gridDim, int blockDim) {
    int globalMinIndex = -1;
    int globalMinWeight = INT_MAX;
    for (int i = 0; i <blockDim; ++i) {
        int localIndex = localMinIndices[i % blockDim];
        if (localIndex != -1) {
            int localWeight = edges[localIndex].weight;
            if (localWeight < globalMinWeight) {
                globalMinWeight = localWeight;
                globalMinIndex = localIndex;
            }
        }
    }

    return globalMinIndex;
}




__global__ void primMSTKernel(bool *mstSet, Edge *edges, int rows, int edgeCount, 
int *minIndices,int *d_globalMinWeight,int *d_currentIndex,int i) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    // Calcolo di quanti edge deve gestire ogni thread
    int edgesPerThread = (edgeCount + blockDim.x - 1) / blockDim.x;

    // Calcolo degli indici di inizio e fine per gli edge assegnati al thread
    int startIdx = tid * edgesPerThread;
    int endIdx = min(startIdx + edgesPerThread, edgeCount);

    // Variabile condivisa per l'indice del minimo locale all'interno del blocco
    __shared__ int blockMinIndex[BLOCK_SIZE];

    // Inizializza blockMinIndex con un valore non valido
    blockMinIndex[threadIdx.x] = -1;
    __syncthreads();

    // Ciascun thread trova il minimo locale solo nei suoi edge
    int minIndex = -1;
    int minWeight = INT_MAX;
    for (int i = startIdx; i < endIdx; ++i) {
        // Verifica se almeno uno dei nodi collegati è già nell'insieme MST
        if ((mstSet[edges[i].start] || mstSet[edges[i].end]) &&
            edges[i].weight < minWeight &&
            ((mstSet[edges[i].start] && !mstSet[edges[i].end]) ||
             (mstSet[edges[i].end] && !mstSet[edges[i].start]))) {
            minWeight = edges[i].weight;
            minIndex = i;
        }
    }
  // Ogni thread salva il suo indice di minimo locale nell'array minIndices
    minIndices[threadIdx.x] = minIndex;
    __syncthreads();

   //Il thread 0 di ciascun blocco trova il minimo tra gli indici di minimo locale
    if (threadIdx.x == 0) {
        int blockMinIndex = findGlobalMinIndex(edges, minIndices, gridDim.x, blockDim.x);

        // Se è stato trovato un indice valido, aggiorna il minimo globale e l'indice corrente
        if (blockMinIndex != -1) {
            atomicMin(d_globalMinWeight, edges[blockMinIndex].weight);
            *d_currentIndex = blockMinIndex;
        }
    }
    __syncthreads();

    // Il thread 0 di ciascun blocco contribuisce al calcolo del minimo globale
    if (threadIdx.x == 0 && blockMinIndex[0] != -1) {
        atomicMin(d_globalMinWeight, edges[blockMinIndex[0]].weight);
    }
    __syncthreads();

    // Il thread 0 del blocco con il minimo locale aggiorna l'indice corrente
    if (minWeight == edges[blockMinIndex[0]].weight && threadIdx.x == 0) {
        printf("Min Index:%d\n",blockMinIndex[0]);
        *d_currentIndex = blockMinIndex[0];
    }
    __syncthreads();
}







int main(int argc, char **argv) {
    char *inputFilePath = argv[1];

    FILE *file = fopen(inputFilePath, "r");
    if (file == NULL) {
        perror("Errore nell'apertura del file");

    }
    int rows= atoi(argv[2]);

    int total_elements = rows * rows;
    int *matrix = (int *)malloc(total_elements * sizeof(int));

    for (int i = 0; i < total_elements; i++) {
         int result = fscanf(file, "%d", &matrix[i]);
    if (result != 1) {
        perror("Errore nella lettura della matrice");
        return 1;
    }
    }

    fclose(file);

    struct Edge *edges = (struct Edge *)malloc((rows * (rows - 1) / 2) * sizeof(struct Edge));
    
    int edgeCount = 0;

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
    printf("Edge count:%d\n",edgeCount);

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    
    float elapsed;
    Edge *d_edges;
    bool *d_mstSet;
    int *d_minWeights;
    bool mstSet[rows];
    int h_minWeight=0;
    int h_globalMinWeight;
    int *d_globalMinWeight;
    int h_currentIndex;
    int *d_currentIndex;
    cudaMalloc((void **)&d_currentIndex,sizeof(int));
    cudaMalloc((void **)&d_globalMinWeight, sizeof(int));
    cudaMalloc((void **)&d_edges, edgeCount * sizeof(Edge));
    cudaMalloc((void **)&d_minWeights, BLOCK_SIZE * sizeof(int));
    cudaMalloc((void **)&d_mstSet, rows * sizeof(bool));

    

    // Inizializza la somma del peso dell'MST a 0
    int totalWeight = 0;
    
    for (int j = 0; j < rows; j++) {
            mstSet[j] = false;
        }
        mstSet[0] = true;
    int blockCount = (rows -1 + BLOCK_SIZE -1 ) / BLOCK_SIZE;
    printf("Block count:%d\n",blockCount);
    float times[10];
    
    for(int j =0;j<10;j++){
    cudaEventRecord(start,0);
    #pragma omp parallel sections
{

    #pragma omp section
    {
        cudaMemcpy(d_edges, edges, edgeCount * sizeof(Edge), cudaMemcpyHostToDevice);
    }

    #pragma omp section
    {
        cudaMemcpy(d_mstSet, mstSet, rows * sizeof(bool), cudaMemcpyHostToDevice);
    }

    #pragma omp section
    {
        cudaMemcpy(d_minWeights, &h_minWeight, sizeof(int), cudaMemcpyHostToDevice);
    }

}

    
    // Esegui il kernel più volte finché non vengono inseriti tutti i vertici in mstSet
    for (int i = 0; i < rows-1; i++) {

        primMSTKernel<<<blockCount, BLOCK_SIZE>>>(d_mstSet, d_edges, rows, edgeCount, 
        d_minWeights,d_globalMinWeight,d_currentIndex,i);
        
        //Aggiorno il peso totale dell'MST
        cudaMemcpy(&h_globalMinWeight, d_globalMinWeight, sizeof(int), cudaMemcpyDeviceToHost);
        cudaMemcpy(&h_currentIndex,d_currentIndex,sizeof(int),cudaMemcpyDeviceToHost);
        mstSet[edges[h_currentIndex].start]=true;
        mstSet[edges[h_currentIndex].end]=true;
        totalWeight += edges[h_currentIndex].weight;
        cudaMemcpy(d_mstSet, mstSet, rows * sizeof(bool), cudaMemcpyHostToDevice);
        cudaDeviceSynchronize();

    }
    
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    
    // execution time between events in ms
    cudaEventElapsedTime(&elapsed, start, stop);
    elapsed = elapsed/1000.f; // convert to seconds
    times[j]=elapsed;
    }
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    float total;

    for(int i=0;i<10;i++){
        total+=times[i];
    }
    printf("CUDA Time : %f", (double)(total/10));

    cudaFree(d_edges);
    cudaFree(d_mstSet);
    cudaFree(d_minWeights);
    cudaFree(d_globalMinWeight);

    free(edges);
    free(matrix);


    return 0;
}
