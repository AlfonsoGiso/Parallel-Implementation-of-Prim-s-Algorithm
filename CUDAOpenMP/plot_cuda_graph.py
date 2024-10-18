"""
 * Course: High Performance Computing 2023/2024
 *
 * Lecturer: Francesco Moscato  fmoscato@unisa.it * 
 * Student: Giso  Alfonso 0622701842   a.giso@studenti.unisa.it
 * 
 * Provide a parallell version of the Prim's algorithm to find the minimum MST of a graph.
 * 
 * The implementation MUST use an hibrid  of MPI and openMP.
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
"""
import sys
import pandas as pd
import matplotlib.pyplot as plt

def create_histogram(file_csv):
    # Leggi il file CSV utilizzando pandas
    df = pd.read_csv(file_csv)

    # Estrai i dati
    version = df['Version']
    omp_threads = df['OpenMP Threads']
    time = df['Time']

    # Crea un istogramma con barre più sottili
    plt.figure(figsize=(12, 6))  # Aumenta la larghezza del grafico

    # Imposta la larghezza delle barre e lo spaziamento tra le etichette
    bar_width = 0.5
    bar_spacing = 1.5  # Aumenta questo valore se le etichette sono tagliate

    # Calcola la posizione iniziale delle barre sull'asse x
    bar_positions = [i * bar_spacing for i in range(len(time))]

    # Disegna le barre
    plt.bar(bar_positions, time, color='blue', alpha=0.7, width=bar_width)

    # Imposta i titoli e le etichette
    plt.title(f'Time for size: {size}')
    plt.xlabel('Configurazione')
    plt.ylabel('Tempo')
    
    # Aggiungi etichette sull'asse x con maggiore spaziatura
    plt.xticks(bar_positions, [f'{v}, {t} OMP Threads' for v, t in zip(version, omp_threads)], rotation=45, ha='right')

    # Mostra l'istogramma
    plt.tight_layout()  # Regola automaticamente la disposizione per evitare sovrapposizioni
    plt.savefig(f'./output/graphs/size_{size}_speedup.png')

if __name__ == "__main__":
    # Check if the user provided a command-line argument for string length
    if len(sys.argv) != 2:
        print("Usage: python generate_graph.py <string_length> <optimization>")
        sys.exit(1)

    # Get the desired length of the random string from the command line
    size = sys.argv[1]
    csv_file_path = f'./output/output_{size}.csv'

    # Chiamata alla funzione principale con il percorso del singolo file fornito come argomento
    create_histogram(csv_file_path)
