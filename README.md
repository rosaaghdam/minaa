# Microbiome Network Alignment

## Description

The Microbiome Network Alignment algorithm aligns two networks based their topologies and biologies.

## Compilation

Compile with `make` or `g++ -O2 -std=c++20 -o mna.exe mna.cpp hungarian.cpp gdvs_dist.cpp orca.cpp file_io.cpp util.cpp`

## Usage

This utility takes five command-line arguments: `./mna.exe <G> <H> [-a=alpha] [-B=bio] [-b=beta] [-g=gamma]`

### Required arguments (ordered)

1. G: A graph to align
   - Require: the graph is represented by an adjacency list or matrix in CSV format
2. H: A graph to align
   - Require: the graph is represented by an adjacency list or matrix in CSV format

### Optional arguments (unordered)

- alpha: GDV-edge weight balancer
  - Require: a real number in range [0, 1]
  - Default: 0.5
- bio: The path to the biological cost matrix file
  - Require: CSV file type
  - Default: the algorithm will run using only topological calculations
- beta: topological-biological cost matrix balancer
  - Require: a real number in range [0, 1]
  - Default: 0.5
- gamma: any alignment cost equal or greater than this will not be recorded as an alignment
  - Require: a real number in range [0, 1]
  - Default: 1

### Examples

`./mna.exe graph0.csv graph1.csv -a=1`

Here we align graph0 with graph1 using no biological data. '-a=1' sets alpha equal to 1, meaning 100% of the topological cost function comes from similarity calculated by GDVs, and none from simpler node degree data.'g=0.6' sets gamma equal to 0.6, meaning that after alignment, any aligned pair whose cost of alignment was greater than or equal to 0.6 will not be written to the output file as an aligned pair.

`./mna.exe graph0.csv graph1.csv bio_costs.csv -b=0.85`

Here we align graph0 with graph1 using topological information and the given biological cost matrix, bio_costs. Since alpha and gamma were unspecified, they default to 0.5 and 1 respectively. Since beta was set to 0.85, 85% of the cost weight is from the topological cost matrix, and 15% is from the given biological cost matrix.

## Data

No test data is available at this time.
