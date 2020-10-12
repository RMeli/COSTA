#pragma once
#include <costa/grid2grid/grid_layout.hpp>
#include <costa/grid2grid/scalapack_layout.hpp>
/*
#ifdef __cplusplus
extern "C" {
#endif
*/

namespace costa {
/**
 * A local block of the matrix.
 * data: a pointer to the start of the local matrix A_loc
 * ld: leading dimension or distance between two columns of A_loc
 * row: the global block row index
 * col: the global block colum index
 */
struct block_t {
    void *data;
    const int ld;
    const int row;
    const int col;
};

/**
 * Description of a distributed layout of a matrix
 * rowblocks: number of gobal blocks
 * colblocks: number of gobal blocks
 * rowsplit: [rowsplit[i], rowsplit[i+1]) is range of rows of block i
 * colsplit: [colsplit[i], colsplit[i+1]) is range of columns of block i
 */
struct grid_t {
    // global view
    int rowblocks;
    int colblocks;
    const int *rowsplit;
    const int *colsplit;
    const int *owners;
};

/**
 * Description of a distributed layout of a matrix
 * grid: describes the global matrix grid
 * nlocalblock: number of blocks owned by the current rank
 * localblcoks: an array of block descriptions of the current rank
 */
struct layout_t {
    // global_view
    grid_t* grid;
    // local view (local blocks)
    int nlocalblocks;
    block_t* localblocks;
};


/**
 * Description of a distributed layout of a matrix
 * grid: describes the global matrix grid
 * nlocalblock: number of blocks owned by the current rank
 * localblcoks: an array of block descriptions of the current rank
 */
template <typename T>
costa::grid_layout<T> custom_layout(grid_t* grid,
                                     int n_local_blocks,
                                     block_t* local_blocks);

/**
 * creates a block cyclic layout (scalapack data layout) of some matrix A
 * and represents a submatrix sub(A) of the global matrix. The submatrix 
 * starts at (i, j) and has dimensions (sub_m, sub_n).
 * The detailed arguments are described below:
 * (m, n): global matrix dimensions of matrix A
 * (block_m, block_n): block dimensions
 * (i, j): submatrix start, 0-based. By default can be set to (0, 0).
 * (sub_m, sub_n): size of the submatrix sub(A). By default can be set to (m, n).
 * (proc_m, proc_n): processor, i.e. MPI ranks grid
 * (ia, ja): coordinates of ranks owning the first row/col of the global matrix.
 *           This is 1-based, due to scalapack compatibility. 
 *           By default, should be set to (1, 1)
 * ptr: pointer to local data of the global matrix A.
 * lld: local leading dimension
 * rank: MPI rank
 */
template <typename T>
costa::grid_layout<T> block_cyclic_layout(
        const int m, const int n, // global matrix dimensions
        const int block_m, const int block_n, // block dimensions
        const int i, const int j, // submatrix start
        const int sub_m, const int sub_n, // submatrix size
        const int proc_m, const int proc_n, // processor grid dimension
        const char rank_grid_ordering, // rank grid ordering ('R' or 'C')
        const int ia, const int ja, // coordinates of ranks oweing 
                                    // the first row 
                                    // (1-based, scalapack-compatible)
        const T* ptr, // local data of matrix A (not the submatrix)
        const int lld, // local leading dimension
        const int rank // processor rank
);
}

/*
#ifdef __cplusplus
}
#endif
*/