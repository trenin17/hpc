/*
 * heat.h
 *
 * Global definitions for the iterative solver
 */

#ifndef JACOBI_H_INCLUDED
#define JACOBI_H_INCLUDED

#include <stdio.h>

// configuration

typedef struct
{
    float posx;
    float posy;
    float range;
    float temp;
} heatsrc_t;

typedef struct
{
    unsigned maxiter; // maximum number of iterations
    unsigned act_res;
    unsigned max_res; // spatial resolution
    unsigned initial_res;
    unsigned res_step_size;
    int algorithm; // 0=>Jacobi, 1=>Gauss

    unsigned visres; // visualization resolution

    double *u, *uhelp;
    double *uvis;

    unsigned numsrcs; // number of heat sources
    heatsrc_t *heatsrcs;

    // --- MPI-specific parameters for decomposition ---
    int rank, size;      // MPI rank and size of communicator
    int local_act_res;   // Number of rows for this process's sub-grid
    int start_y;         // Global starting row index for this process
    int top_neighbor;    // Rank of the process above (-1 if none)
    int bottom_neighbor; // Rank of the process below (-1 if none)
} algoparam_t;

// function declarations

// misc.c
int initialize(algoparam_t *param);
int finalize(algoparam_t *param);
void write_image(FILE *f, double *u,
                 unsigned sizex, unsigned sizey);
int coarsen(double *uold, unsigned oldx, unsigned oldy,
            double *unew, unsigned newx, unsigned newy, int start_y, int local_act_res, int stepy);

// Gauss-Seidel: relax_gauss.c
double residual_gauss(double *u, double *utmp, unsigned sizex, unsigned sizey, algoparam_t *param);
void relax_gauss(double *u, unsigned sizex, unsigned sizey, algoparam_t *param);

// Jacobi: relax_jacobi.c
double residual_jacobi(double *u, unsigned sizex, unsigned sizey, algoparam_t *param);
void relax_jacobi(double *u, double *utmp, unsigned sizex, unsigned sizey, algoparam_t *param);

#endif // JACOBI_H_INCLUDED/