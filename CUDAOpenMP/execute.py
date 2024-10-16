"""
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
"""
import os

# Array contenente i diversi numeri di vertici
vertices = ["500", "750", "1000", "2000"]

# Array contenente i diversi numeri di OMP_THREADS
omp_threads = ["1", "2"]

os.system("rm -rf input")
os.system("rm -rf output")
os.system("mkdir input")
os.system("mkdir output")
os.system(f"nvcc -Xcompiler  -fopenmp PrimCUDAOpenMPv1.cu -o PrimCUDAOpenMPv1")
os.system(f"gcc -o PrimMST PrimMST.c")
  

for vertice in vertices:
    os.system(f"mkdir -p output/")
    os.system(f"mkdir -p output/graphs")
    output_file = f"./output/output_{vertice}.csv"
    with open(output_file, "w") as f:
      f.write("Version,OpenMP Threads,Time\n")

    input_file = f"./input/{vertice}_graph.txt"
    os.system(f"./graphGenerator {vertice} {input_file}")

    execution_time = os.popen(f"./PrimMST {input_file} {vertice} | grep 'Sequential Time' | awk '{{print $4}}'").read().strip()
    with open(output_file, "a") as f:
        f.write(f"Serial,0,{execution_time}\n")

    for omp_thread in omp_threads:
        os.environ["OMP_NUM_THREADS"] = omp_thread
        execution_time = os.popen(f"./PrimCUDAOpenMPv1 {input_file} {vertice} | grep 'CUDA Time' | awk '{{print $4}}'").read().strip()
        with open(output_file, "a") as f:
            f.write(f"Parallel,{omp_thread},{execution_time}\n")
    os.system(f"python plot_cuda_graph.py {vertice}")
