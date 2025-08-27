/*
 * misc.c
 *
 * Helper functions for
 * - initialization
 * - finalization,
 * - writing out a picture
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "heat.h"

/*
 * Initialize the iterative solver
 * - allocate memory for matrices
 * - set boundary conditions according to configuration
 */
int initialize(algoparam_t *param)
{
	int i, j;
	double dist;

	// determine local grid size and neighbors
	int base_rows = param->act_res / param->size;
	int extra_rows = param->act_res % param->size;
	param->local_act_res = base_rows + (param->rank < extra_rows ? 1 : 0);
	param->start_y = param->rank * base_rows + (param->rank < extra_rows ? param->rank : extra_rows);
	param->top_neighbor = (param->rank > 0) ? param->rank - 1 : -1;
	param->bottom_neighbor = (param->rank < param->size - 1) ? param->rank + 1 : -1;

	// total number of points in x direction (including border)
	const int sizex = param->act_res + 2;
	// total number of points in y direction for local grid (including border)
	const int sizey_local = param->local_act_res + 2;

	//
	// allocate memory
	//
	(param->u) = (double *)calloc(sizeof(double), sizex * sizey_local);
	(param->uhelp) = (double *)calloc(sizeof(double), sizex * sizey_local);
	if (param->rank == 0)
	{
		(param->uvis) = (double *)calloc(sizeof(double),
										 (param->visres + 2) *
											 (param->visres + 2));
		if (!(param->uvis))
		{
			fprintf(stderr, "Error: Cannot allocate memory\n");
			return 0;
		}
	}

	if (!(param->u) || !(param->uhelp))
	{
		fprintf(stderr, "Error: Cannot allocate memory\n");
		return 0;
	}

	for (i = 0; i < param->numsrcs; i++)
	{
		/* top row (handled by rank 0) */
		if (param->rank == 0)
		{
			for (j = 0; j < sizex; j++)
			{
				dist = sqrt(pow((double)j / (double)(sizex - 1) -
									param->heatsrcs[i].posx,
								2) +
							pow(param->heatsrcs[i].posy, 2));

				if (dist <= param->heatsrcs[i].range)
				{
					(param->u)[j] +=
						(param->heatsrcs[i].range - dist) /
						param->heatsrcs[i].range *
						param->heatsrcs[i].temp;
				}
			}
		}

		/* bottom row (handled by last rank) */
		if (param->rank == param->size - 1)
		{
			for (j = 0; j < sizex; j++)
			{
				dist = sqrt(pow((double)j / (double)(sizex - 1) -
									param->heatsrcs[i].posx,
								2) +
							pow(1 - param->heatsrcs[i].posy, 2));

				if (dist <= param->heatsrcs[i].range)
				{
					(param->u)[(sizey_local - 1) * sizex + j] +=
						(param->heatsrcs[i].range - dist) /
						param->heatsrcs[i].range *
						param->heatsrcs[i].temp;
				}
			}
		}

		/* leftmost column */
		for (j = 1; j < sizey_local - 1; j++)
		{
			double global_y = (double)(param->start_y + j - 1) / (double)(param->act_res);
			dist = sqrt(pow(param->heatsrcs[i].posx, 2) +
						pow(global_y -
								param->heatsrcs[i].posy,
							2));

			if (dist <= param->heatsrcs[i].range)
			{
				(param->u)[j * sizex] +=
					(param->heatsrcs[i].range - dist) /
					param->heatsrcs[i].range *
					param->heatsrcs[i].temp;
			}
		}

		/* rightmost column */
		for (j = 1; j < sizey_local - 1; j++)
		{
			double global_y = (double)(param->start_y + j - 1) / (double)(param->act_res);
			dist = sqrt(pow(1 - param->heatsrcs[i].posx, 2) +
						pow(global_y -
								param->heatsrcs[i].posy,
							2));

			if (dist <= param->heatsrcs[i].range)
			{
				(param->u)[j * sizex + (sizex - 1)] +=
					(param->heatsrcs[i].range - dist) /
					param->heatsrcs[i].range *
					param->heatsrcs[i].temp;
			}
		}
	}

	// copy boundary conditions to uhelp
	for (i = 0; i < sizex * sizey_local; i++)
	{
		param->uhelp[i] = param->u[i];
	}

	return 1;
}

/*
 * free used memory
 */
int finalize(algoparam_t *param)
{
	if (param->u)
	{
		free(param->u);
		param->u = 0;
	}

	if (param->uhelp)
	{
		free(param->uhelp);
		param->uhelp = 0;
	}

	return 1;
}

/*
 * write the given temperature u matrix to rgb values
 * and write the resulting image to file f
 */
void write_image(FILE *f, double *u,
				 unsigned sizex, unsigned sizey)
{
	// RGB table
	unsigned char r[1024], g[1024], b[1024];
	int i, j, k;

	double min, max;

	j = 1023;

	// prepare RGB table
	for (i = 0; i < 256; i++)
	{
		r[j] = 255;
		g[j] = i;
		b[j] = 0;
		j--;
	}
	for (i = 0; i < 256; i++)
	{
		r[j] = 255 - i;
		g[j] = 255;
		b[j] = 0;
		j--;
	}
	for (i = 0; i < 256; i++)
	{
		r[j] = 0;
		g[j] = 255;
		b[j] = i;
		j--;
	}
	for (i = 0; i < 256; i++)
	{
		r[j] = 0;
		g[j] = 255 - i;
		b[j] = 255;
		j--;
	}

	min = DBL_MAX;
	max = -DBL_MAX;

	// find minimum and maximum
	for (i = 0; i < sizey; i++)
	{
		for (j = 0; j < sizex; j++)
		{
			if (u[i * sizex + j] > max)
				max = u[i * sizex + j];
			if (u[i * sizex + j] < min)
				min = u[i * sizex + j];
		}
	}

	fprintf(f, "P3\n");
	fprintf(f, "%u %u\n", sizex, sizey);
	fprintf(f, "%u\n", 255);

	for (i = 0; i < sizey; i++)
	{
		for (j = 0; j < sizex; j++)
		{
			k = (int)(1024.0 * (u[i * sizex + j] - min) / (max - min));
			if (k == 1024)
				k = 1023;

			fprintf(f, "%d %d %d  ", r[k], g[k], b[k]);
		}
		fprintf(f, "\n");
	}
}

int coarsen(double *uold, unsigned oldx, unsigned oldy,
			double *unew, unsigned newx, unsigned newy, int start_y, int local_act_res, int stepy)
{
	int global_row, j, local_row, local_coarse_row;

	int stepx;
	int stopx = newx;

	if (oldx > newx)
		stepx = oldx / newx;
	else
	{
		stepx = 1;
		stopx = oldx;
	}

	// NOTE: this only takes the top-left corner,
	// and doesnt' do any real coarsening
	local_coarse_row = 0;
	// loop through global rows at global coarsening steps
	for (global_row = start_y; global_row < start_y + local_act_res; global_row += stepy)
	{

		local_row = global_row - start_y;

		for (j = 0; j < stopx - 1; j++)
		{
			unew[local_coarse_row * newx + j] = uold[local_row * oldx + j * stepx];
		}
		local_coarse_row++;
	}
	return 1;
}
