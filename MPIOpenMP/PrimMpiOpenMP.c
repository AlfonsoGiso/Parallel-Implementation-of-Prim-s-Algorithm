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
#include <mpi.h>

typedef struct Edge {
    int start, end, weight;
} Edge;

void createMPIEdgeType(MPI_Datatype *MPI_EDGE) {

    int blocklengths[3] = {1, 1, 1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[3];

    offsets[0] = offsetof(Edge, weight);
    offsets[1] = offsetof(Edge, start);
    offsets[2] = offsetof(Edge, end);

    MPI_Type_create_struct(3, blocklengths, offsets, types, MPI_EDGE);
    MPI_Type_commit(MPI_EDGE);
}

void minWeightOp(void *in, void *inout, int *len, MPI_Datatype *dptr) {
    Edge *inEdges = (Edge *)in;
    Edge *inoutEdge = (Edge *)inout;

    // Se il peso dell'arco in inEdges è minore, sostituisci inoutEdge con inEdges
    // Se i pesi sono uguali, confronta i vertici di partenza per determinare l'ordine
    if (inEdges->weight < inoutEdge->weight ||
        (inEdges->weight == inoutEdge->weight && inEdges->start < inoutEdge->start)) {
        *inoutEdge = *inEdges;
    }
}



int findEdgeWithMinKey(bool mstSet[], Edge edges[], int edgeCount) {
    int min = INT_MAX, min_index = -1;

    #pragma omp parallel for
    for (int i = 0; i < edgeCount; i++) {
        if ((mstSet[edges[i].start] || mstSet[edges[i].end]) && edges[i].weight < min) {
            // Se uno dei vertici è nell'MST, verifica che entrambi i vertici non siano già nell'MST
            if (!(mstSet[edges[i].start] && mstSet[edges[i].end])) {
                #pragma omp critical
                {
                    if (edges[i].weight < min) {
                        min = edges[i].weight;
                        min_index = i;
                    }
                }
            }
        }
    }

    return min_index;
}


Edge* parallelPrimMST(Edge edges[], int edgeCount, int rows, int rank, int size) {
    MPI_Datatype MPI_EDGE;
    createMPIEdgeType(&MPI_EDGE);
    MPI_Op minWeightMPIOp;
    MPI_Op_create((MPI_User_function *)minWeightOp, 1, &minWeightMPIOp);
    Edge* mst = NULL;
    if (rank == 0) {
        mst = (Edge*)malloc((rows - 1) * sizeof(Edge));
        if (mst == NULL) {
            fprintf(stderr, "Errore nell'allocazione di memoria per MST\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }// an array of Edges containing the current Minimum Spanning Tree
    bool mstSet[rows];
    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));
    
    // initialize all components of mstSet to false
    for (int i = 0; i < rows; i++) {
        mstSet[i] = false;
    }
    // set the source of the MST as the Vertex 0
    mstSet[0] = true;

    int localEdgeCount = edgeCount / size;
    int extraSize = edgeCount % size;

    if(size == 1){
        sendcounts[0] = localEdgeCount;
        displs[0] = 0;
    }
    else{
        //I need to define the amount of edges to send to each process (*sendcount), and from where each process needs to start to read data (*displs)
        for (int i = 0; i < size - 1; i++){
            sendcounts[i] = localEdgeCount;

            if(i == 0){
                displs[i] = 0;
            }
            else{
                displs[i] = localEdgeCount + displs[i-1];
            }
        }

        //As follow, we see that I decided to give to the last process the extrasize over the usual localEdgeCount
        sendcounts[size-1] = localEdgeCount + extraSize;      
        displs[size-1] = localEdgeCount + displs[size-2];  
    }
    /*if(rank==0){
        printf("\nExtrasize to fill in the last rank:%d\n",extraSize);
    }*/
    
    Edge *localEdges = (Edge *)malloc(sendcounts[rank] * sizeof(Edge));
    MPI_Bcast(sendcounts, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(displs, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&mstSet, rows, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    Edge globalMinEdge;
    MPI_Scatterv(edges, sendcounts, displs, MPI_EDGE, localEdges, sendcounts[rank], MPI_EDGE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    for (int count = 0, i = 0; count < rows-1; count++) {
        
        // Now, each process works with its localEdges array

        int localMinIndex = findEdgeWithMinKey(mstSet, localEdges, sendcounts[rank]);
        MPI_Barrier(MPI_COMM_WORLD);


        MPI_Barrier(MPI_COMM_WORLD);
        if(localMinIndex!= -1){
            localMinIndex;
            //Find the global minimum
            MPI_Allreduce(&localEdges[localMinIndex], &globalMinEdge, 1, MPI_EDGE, minWeightMPIOp, MPI_COMM_WORLD);
        }
        else{
            Edge noEdgeFound;
            noEdgeFound.weight = INT_MAX;
            noEdgeFound.end = 0;
            noEdgeFound.start = 1; 
            MPI_Allreduce(&noEdgeFound,&globalMinEdge, 1, MPI_EDGE, minWeightMPIOp, MPI_COMM_WORLD);
        }
       
        

        if (rank == 0) {
            mstSet[globalMinEdge.start] = true;
            mstSet[globalMinEdge.end] = true;
            mst[i++] = globalMinEdge;

        }
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Bcast(&mstSet, rows, MPI_C_BOOL, 0, MPI_COMM_WORLD);

    }


    free(sendcounts);
    free(displs);
    free(localEdges);
    MPI_Op_free(&minWeightMPIOp);
    return mst;
}



int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    double start_time;
    double end_time;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    char *inputFilePath = argv[1];

    FILE *file = fopen(inputFilePath, "r");
    if (file == NULL) {
        perror("Errore nell'apertura del file");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    int rows= atoi(argv[2]);

    int total_elements = rows * rows;
    int *matrix = (int *)malloc(total_elements * sizeof(int));

    for (int i = 0; i < total_elements; i++) {
        if(fscanf(file, "%d", &matrix[i])!=1){
            perror("Errore nella lettura del numero di righe");
            return 1;
        };
    }

    fclose(file);


    Edge *edges = (Edge *)malloc((rows * (rows - 1) / 2) * sizeof(Edge));
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
    free(matrix);

    MPI_Barrier(MPI_COMM_WORLD);
    double times[10];
    int averageTime;
    double elapsed;
    for(int i=0;i<10;i++){
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    Edge* mst= parallelPrimMST(edges, edgeCount, rows, rank, size);

    
    if (rank == 0) {
        end_time = MPI_Wtime();
        
    }
    elapsed= end_time - start_time;
    
    if(rank==0){
        int weight = 0;
        for (int i = 0; i < rows - 1; i++) {
            weight += mst[i].weight;
        }
        printf("\nTotal Weight:%d\n", weight);
    }
    times[i]=elapsed;
    }
    double total=0;
    for(int i=0;i<10;i++){
        total+=times[i];
    }
    if(rank==0){
            printf("MPI Time : %f", (double)(total/10));
    }
    /*if (rank == 0) {
        printf("\nMST Edges \t\t\tWeight\n");
        for (int i = 0; i < rows - 1; i++) {
            printf("%d - %d \t\t\t%d \n", mst[i].start, mst[i].end, mst[i].weight);
        }

        
    }*/
    /*if (rank == 0) {
    // Apri un file in modalità scrittura
    FILE *outputFile = fopen(outputFilePath, "a");
    if (outputFile == NULL) {
        perror("Errore nell'apertura del file di output");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Scrivi il numero di rank e il tempo necessario nel file
    fprintf(outputFile,"%f\n",end_time - start_time);
    // Chiudi il file
    fclose(outputFile);
}*/
    free(edges);

    MPI_Finalize();

    return 0;
}