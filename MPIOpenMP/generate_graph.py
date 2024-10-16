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
import sys
import pandas as pd
import matplotlib.pyplot as plt

if __name__ == "__main__":
    # Check if the user provided a command-line argument for string length
    if len(sys.argv) != 3:
        print("Usage: python generate_graph.py <string_length> <optimization>")
        sys.exit(1)

    # Get the desired length of the random string from the command line
    size = sys.argv[1]
    opt = sys.argv[2]

    # Read the CSV file into a Pandas DataFrame
    csv_file_path = f'./output/{opt}/output_{size}.csv'
    df = pd.read_csv(csv_file_path)
    
    df['Speedup'] = df['Time'][0] / df['Time']

    # Save the modified DataFrame back to the CSV file
    df.to_csv(csv_file_path, index=False)

    df = df[df['Version'] == 'Parallel']

    threads_values = sorted(df['OpenMP Threads'].unique())

    # Create a figure and axis
    fig, ax = plt.subplots()

    # Plot curves for each OpenMP Threads value
    for thread_value in threads_values:
        subset_data = df[df['OpenMP Threads'] == thread_value]
        subset_data = subset_data.copy()

        # Insert entry (0,0) at the beginning just for plotting purpose
        subset_data.loc[-1] = {'MPI Processes': 0, 'OpenMP Threads': thread_value, 'Speedup': 0}
        subset_data.index = subset_data.index + 1
        subset_data = subset_data.sort_index()

        # Plot the curve given a fixed number of omp_threads
        ax.plot(subset_data['MPI Processes'], subset_data['Speedup'], label=f'OMP Threads = {thread_value}', linewidth = '2')
    
    # Plot ideal speedup
    ax.plot(subset_data['MPI Processes'], subset_data['MPI Processes'], label=f'Ideal speedup', linewidth = '2', linestyle='--')

    # Set labels and title
    ax.set_xlabel('MPI Processes')
    ax.set_ylabel('Speedup')
    ax.set_title(f'Speedup Comparison (Optimization -{opt})')

    # Add a legend
    ax.legend()
    ax.grid(True)

    # Save the plot
    plt.savefig(f'./output/{opt}/graphs/size_{size}_speedup.png')

