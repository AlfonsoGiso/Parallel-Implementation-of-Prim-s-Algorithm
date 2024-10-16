#!/bin/bash

compile_with_optimization() {
    local OPT_LEVEL=$1
    mpicc -fopenmp -${OPT_LEVEL} PrimMpiOpenMP.c -o PrimMpiOpenMP
    gcc -${OPT_LEVEL} -o PrimMST PrimMST.c
}

#Array contentente i vari livelli di ottimizzazione
OPTIMIZATION_LEVELS=("O0" "O1" "O2" "O3")

# Array contenente i diversi numeri di vertici
VERTICES=("500" "750" "1000" "2000")

# Array contenente i diversi numeri di rank
RANKS=("1" "2" "3" "4")
# Array contenente i diversi numeri di OMP_THREADS
OMP_THREADS=("1" "2")

rm -rf input
rm -rf output
mkdir input
mkdir output

for OPT_LEVEL in "${OPTIMIZATION_LEVELS[@]}"; do
    mkdir -p output/${OPT_LEVEL}
    mkdir -p output/${OPT_LEVEL}/graphs
    
    echo $OPT_LEVEL
    compile_with_optimization $OPT_LEVEL
    
    for VERTICE in "${VERTICES[@]}"; do

        OUTPUT_FILE="./output/${OPT_LEVEL}/output_${VERTICE}.csv"
        echo "Version,MPI Processes,OpenMP Threads,Time" >> $OUTPUT_FILE

        INPUT_FILE="./input/${VERTICE}_graph.txt"

        ./graphGenerator $VERTICE $INPUT_FILE

        EXECUTION_TIME=$(./PrimMST $INPUT_FILE $VERTICE| grep "Sequential Time" | awk '{print $4}')
        echo "Serial,0,0,${EXECUTION_TIME}" >> $OUTPUT_FILE

        for RANK in "${RANKS[@]}"; do
            for OMP_THREAD in "${OMP_THREADS[@]}"; do
            
                export OMP_NUM_THREADS=$OMP_THREAD
                EXECUTION_TIME=$(mpirun -np $RANK PrimMpiOpenMP $INPUT_FILE $VERTICE| grep "MPI Time" | awk '{print $4}')

                echo "Parallel,${RANK},${OMP_THREAD},${EXECUTION_TIME}" >> $OUTPUT_FILE

            done
	    done

	    python3 generate_graph.py $VERTICE $OPT_LEVEL
	
    done
done