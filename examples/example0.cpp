#include <costa/layout.hpp>
#include <costa/grid2grid/transform.hpp>

#include <mpi.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

/*
 In this example we transform the matrix between two simple block-cyclic layouts.
 Run as: mpirun -np 4 ./build/examples/example1.cpp

 Initial Matrix Layout:
 ======================
 - matrix dimension = 10x10
 - number of processors (MPI ranks): 4
 - distributed as below

           5           5
      -----------------------
     |           |           |
     |           |           |
  5  |   rank 0  |   rank 1  |
     |           |           |
     |           |           |
      -----------------------
     |           |           |
     |           |           |
  5  |   rank 2  |   rank 3  |
     |           |           |
     |           |           |
      -----------------------
           5           5

 Target Matrix Layout:
 ======================
 - matrix dimension = 10x10
 - number of processors (MPI ranks): 4
 - distributed as below

           5           5
      -----------------------
     |           |           |
     |           |           |
  5  |   rank 0  |   rank 2  |
     |           |           |
     |           |           |
      -----------------------
     |           |           |
     |           |           |
  5  |   rank 1  |   rank 3  |
     |           |           |
     |           |           |
      -----------------------
           5           5


 Both layouts are block-cyclic.
 Both layouts are represented with a function costa::block_cyclic_layout
 which is much simpler since it's specialized for the scalapack block-cyclic layout.
 */
int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    MPI_Comm comm = MPI_COMM_WORLD;

    int P, rank;
    MPI_Comm_size(comm, &P);
    MPI_Comm_rank(comm, &rank);

    if (P != 4) {
        std::cout << "[ERROR] The test runs with 4 processes!\n";
        MPI_Abort(comm, 0);
    }

    // ***************************************
    // DESCRIBING THE GLOBAL MATRIX GRID
    // ***************************************
    // global matrix dimension (square matrix)
    int mat_dim = 4;
    // block size
    int block_size = 2;
    // number of blocks in a row/col
    int n_blocks = mat_dim / block_size;

    // ***************************************
    // CREATING THE INITIAL LAYOUT OBJECT
    // ***************************************
    // local data
    std::vector<double> initial_data(block_size * block_size);
    // data layout
    auto init_layout = costa::block_cyclic_layout(
            mat_dim, mat_dim, // global matrix dimension
            block_size, block_size, // block sizes
            1, 1, // submatrix start 
                  // (1-based, because of scalapack.
                  // Since we take full matrix, it's 1,1)
            mat_dim, mat_dim, // submatrix size (since full matrix, it's mat dims)
            2, 2, // processor grid
            'R', // processor grid ordering row-major
            0, 0, // coords or ranks oweing the first row (0-based)
            &initial_data[0], // local data of full matrix
            block_size, // local leading dimension
            rank // current rank
    );

    // ***************************************
    // INITIALIZE THE INITIAL MATRIX
    // ***************************************
    // function f(i, j) := value of element (i, j) in the global matrix
    // an arbitrary function
    auto f = [](int i, int j) -> double {
        return i + j; 
    };
    init_layout.initialize(f);

    // ***************************************
    // CREATING THE FINAL LAYOUT OBJECT
    // ***************************************
    // local data
    std::vector<double> final_data(block_size * block_size);
    // data layout
    auto final_layout = costa::block_cyclic_layout(
            mat_dim, mat_dim, // global matrix dimension
            block_size, block_size, // block sizes
            1, 1, // submatrix start 
                  // (1-based, because of scalapack.
                  // Since we take full matrix, it's 1,1)
            mat_dim, mat_dim, // submatrix size (since full matrix, it's mat dims)
            2, 2, // processor grid
            'C', // processor grid ordering col-major
            0, 0, // coords or ranks oweing the first row (0-based)
            &final_data[0], // local data of full matrix
            block_size, // local leading dimension
            rank // current rank
    );

    // ***************************************
    // TRANSFORMING: INITIAL->FINAL
    // ***************************************
    // transform the initial layout -> final layout (out-of-place)
    costa::transform<double>(init_layout, final_layout, comm);

    // if we want to also scale or transpose the data, we can specify 
    // the transpose flag ('N', 'T' or 'C') and the scaling flags
    // to compute: 
    //     final = beta * final + alpha * initial;
    // char trans = 'N'; // do not transpose the initial layout
    // double alpha = 0.5;
    // double beta = 0.5;
    // costa::transform<double>(init_layout, final_layout, trans, alpha, beta, comm);

    // check if the values in the final layout correspond to function f
    // that was used for the initialization of the initial layout
    bool ok = final_layout.validate(f, 1e-12); // the second argument is tolerance

    if (!ok) {
        std::cout << "[ERROR] Result incorrect on rank " << rank << std::endl;
        MPI_Abort(comm, 0);
    }

    MPI_Barrier(comm);

    // if MPI was not aborted, results are correct
    if (rank == 0) {
        std::cout << "[PASSED] Results are correct!" << std::endl;
    }

    MPI_Finalize();
    return 0;
}
