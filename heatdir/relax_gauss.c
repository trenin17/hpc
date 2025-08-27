/*
 * relax_gauss.c
 *
 * Gauss-Seidel Relaxation
 *
 */

#include "heat.h"
#include <mpi.h>
#include <math.h>

/*
 * Residual (length of error vector)
 * between current solution and next after a Gauss-Seidel step
 *
 * Temporary array utmp needed to not change current solution
 *
 * Flop count in inner body is 7
 */

double residual_gauss(double *u, double *utmp, unsigned sizex, unsigned sizey, algoparam_t *param)
{
	unsigned i, j;
	double unew, diff, sum = 0.0;

	// Halo exchange for the "old" right and bottom values
	MPI_Sendrecv(&u[1 * sizex], sizex, MPI_DOUBLE, param->top_neighbor, 0,
				 &u[0], sizex, MPI_DOUBLE, param->top_neighbor, 0,
				 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	MPI_Sendrecv(&u[(sizey - 2) * sizex], sizex, MPI_DOUBLE, param->bottom_neighbor, 0,
				 &u[(sizey - 1) * sizex], sizex, MPI_DOUBLE, param->bottom_neighbor, 0,
				 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	// first row (boundary condition) into utmp
	for (j = 0; j < sizex; j++)
		utmp[0 * sizex + j] = u[0 * sizex + j];
	// first column (boundary condition) into utmp
	for (i = 1; i < sizey - 1; i++)
		utmp[i * sizex + 0] = u[i * sizex + 0];

	for (i = 1; i < sizey - 1; i++)
	{
		for (j = 1; j < sizex - 1; j++)
		{
			unew = 0.25 * (utmp[i * sizex + (j - 1)] + // new left
						   u[i * sizex + (j + 1)] +	   // right
						   utmp[(i - 1) * sizex + j] + // new top
						   u[(i + 1) * sizex + j]);	   // bottom

			diff = unew - u[i * sizex + j];
			sum += diff * diff;

			utmp[i * sizex + j] = unew;
		}
	}

	return sum;
}

/*
 * One Gauss-Seidel iteration step
 *
 * Flop count in inner body is 4
 */
void relax_gauss(double *u, unsigned sizex, unsigned sizey, algoparam_t *param)
{
	unsigned i, j;

	// The dependency flows from top to bottom.
	// Each process must receive the updated boundary from its top neighbor before starting.

	for (i = 1; i < sizey - 1; i++)
	{
		// Receive updated halo from top neighbor before computing row i
		if (i == 1 && param->top_neighbor != -1)
		{
			MPI_Recv(&u[0], sizex, MPI_DOUBLE, param->top_neighbor, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		for (j = 1; j < sizex - 1; j++)
		{
			u[i * sizex + j] = 0.25 * (u[i * sizex + (j - 1)] + u[i * sizex + (j + 1)] + u[(i - 1) * sizex + j] + u[(i + 1) * sizex + j]);
		}

		// Send newly computed row i to bottom neighbor
		if (param->bottom_neighbor != -1)
		{
			MPI_Send(&u[i * sizex], sizex, MPI_DOUBLE, param->bottom_neighbor, 1, MPI_COMM_WORLD);
		}
	}
}