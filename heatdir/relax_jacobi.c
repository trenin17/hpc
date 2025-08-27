/*
 * relax_jacobi.c
 *
 * Jacobi Relaxation
 *
 */

#include "heat.h"
#include <mpi.h>
#include <math.h>

/*
 * Residual (length of error vector)
 * between current solution and next after a Jacobi step
 */
double residual_jacobi(double *u, unsigned sizex, unsigned sizey, algoparam_t *param)
{
	unsigned i, j;
	double unew, diff, sum = 0.0;

	for (i = 1; i < sizey - 1; i++)
	{
		for (j = 1; j < sizex - 1; j++)
		{
			unew = 0.25 * (u[i * sizex + (j - 1)] + // left
						   u[i * sizex + (j + 1)] + // right
						   u[(i - 1) * sizex + j] + // top
						   u[(i + 1) * sizex + j]); // bottom

			diff = unew - u[i * sizex + j];
			sum += diff * diff;
		}
	}

	return sum;
}

/*
 * One Jacobi iteration step
 */
void relax_jacobi(double *u, double *utmp, unsigned sizex, unsigned sizey, algoparam_t *param)
{
	int i, j;

	// Halo exchange: send own boundary rows and receive ghost rows
	// Send row 1 to top neighbor, receive into row 0
	if (param->rank != 0)
	{
		MPI_Sendrecv(&u[1 * sizex], sizex, MPI_DOUBLE, param->top_neighbor, 0,
					 &u[0], sizex, MPI_DOUBLE, param->top_neighbor, 0,
					 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	// Send row sizey-2 to bottom neighbor, receive into row sizey-1
	if (param->rank != param->size - 1)
	{
		MPI_Sendrecv(&u[(sizey - 2) * sizex], sizex, MPI_DOUBLE, param->bottom_neighbor, 0,
					 &u[(sizey - 1) * sizex], sizex, MPI_DOUBLE, param->bottom_neighbor, 0,
					 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	for (i = 1; i < sizey - 1; i++)
	{
		for (j = 1; j < sizex - 1; j++)
		{
			utmp[i * sizex + j] = 0.25 * (u[i * sizex + (j - 1)] + // left
										  u[i * sizex + (j + 1)] + // right
										  u[(i - 1) * sizex + j] + // top
										  u[(i + 1) * sizex + j]); // bottom
		}
	}
}