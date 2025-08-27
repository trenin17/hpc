/*
 * heat.c
 *
 * Iterative solver for heat distribution
 */

#include "heat.h"
#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "input.h"
#include "timing.h"

void usage(char *s)
{
	fprintf(stderr, "Usage: %s <input file> [result file]\n\n", s);
}

int main(int argc, char *argv[])
{
	int rank, size;
	unsigned iter;
	FILE *infile, *resfile;
	char *resfilename;

	// algorithmic parameters
	algoparam_t param;
	int np, i;

	double runtime, flop;
	double residual, global_residual;
	double time[1000];
	double floprate[1000];
	int resolution[1000];
	int experiment = 0;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// check arguments
	if (argc < 2)
	{
		if (rank == 0)
			usage(argv[0]);
		MPI_Finalize();
		return 1;
	}

	// check input file
	if (!(infile = fopen(argv[1], "r")))
	{
		fprintf(stderr, "\nRank %d: Error: Cannot open \"%s\" for reading.\n\n", rank, argv[1]);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}

	// check result file
	if (rank == 0)
	{
		resfilename = (argc >= 3) ? argv[2] : "heat.ppm";
		if (!(resfile = fopen(resfilename, "w")))
		{
			fprintf(stderr, "\nError: Cannot open \"%s\" for writing.\n\n", resfilename);
			MPI_Abort(MPI_COMM_WORLD, 1);
		}
	}

	// check input
	if (!read_input(infile, &param))
	{
		fprintf(stderr, "\nRank %d: Error: Error parsing input file.\n\n", rank);
		MPI_Abort(MPI_COMM_WORLD, 1);
	}
	fclose(infile);

	if (rank == 0)
	{
		print_params(&param);
	}

	// store MPI parameters for other functions
	param.rank = rank;
	param.size = size;

	// set the visualization resolution
	param.visres = 1024;

	param.u = 0;
	param.uhelp = 0;
	param.uvis = 0;

	// allocate memory for visualization
	if (rank == 0)
	{
		param.uvis = (double *)calloc(sizeof(double), (param.visres + 2) * (param.visres + 2));
	}

	param.act_res = param.initial_res;

	// loop over different resolutions
	while (1)
	{

		// free allocated memory of previous experiment
		if (param.u != 0)
			finalize(&param);

		// Recalculate local resolution based on the new act_res, give additional rows for uneven division
		param.local_act_res = param.act_res / param.size + (param.rank < param.act_res % param.size);

		if (!initialize(&param))
		{
			fprintf(stderr, "Rank %d: Error in Jacobi initialization.\n\n", rank);
			MPI_Abort(MPI_COMM_WORLD, 1);
		}

		if (rank == 0)
			fprintf(stderr, "Resolution: %5u\r", param.act_res);

		// full size (param.act_res are only the inner points)
		np = param.act_res + 2;

		// starting time
		MPI_Barrier(MPI_COMM_WORLD);
		runtime = wtime();
		residual = 999999999;

		iter = 0;
		while (1)
		{

			switch (param.algorithm)
			{

			case 0: // JACOBI

				relax_jacobi(param.u, param.uhelp, np, param.local_act_res + 2, &param);
				if (iter > 0) // skip first iteration
				{
					residual = residual_jacobi(param.uhelp, np, param.local_act_res + 2, &param);
				}
				// swap u and uhelp
				double *tmp = param.u;
				param.u = param.uhelp;
				param.uhelp = tmp;
				break;

			case 1: // GAUSS

				relax_gauss(param.u, np, param.local_act_res + 2, &param);
				residual = residual_gauss(param.u, param.uhelp, np, param.local_act_res + 2, &param);
				break;
			}

			iter++;

			MPI_Allreduce(&residual, &global_residual, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
			global_residual = sqrt(global_residual);

			// solution good enough ?
			if (global_residual < 0.000005)
				break;

			// max. iteration reached ? (no limit with maxiter=0)
			if (param.maxiter > 0 && iter >= param.maxiter)
				break;
		}

		// Flop count after <i> iterations
		flop = iter * 11.0 * param.act_res * param.act_res;
		// stopping time
		runtime = wtime() - runtime;

		if (rank == 0)
		{
			fprintf(stderr, "Resolution: %5u, ", param.act_res);
			fprintf(stderr, "Time: %04.3f ", runtime);
			fprintf(stderr, "(%3.3f GFlop => %6.2f MFlop/s, ", flop / 1000000000.0, flop / runtime / 1000000);
			fprintf(stderr, "residual %f, %d iterations)\n", global_residual, iter);

			// for plot...
			time[experiment] = runtime;
			floprate[experiment] = flop / runtime / 1000000;
			resolution[experiment] = param.act_res;
			experiment++;
		}

		if (param.act_res + param.res_step_size > param.max_res)
			break;
		param.act_res += param.res_step_size;
	}

	// --- GATHERING PHASE ---
	int *counts = NULL;
	int *displs = NULL;
	double *uvis_local = NULL;
	int stepy;

	if (param.act_res > param.visres)
		stepy = param.act_res / param.visres;
	else
	{
		stepy = 1;
	}

	int local_coarse_rows = 0;
	// exchange by direct calculation??
	for (int i = param.start_y; i < param.start_y + param.local_act_res; i += stepy)
		local_coarse_rows++;
	uvis_local = malloc(local_coarse_rows * (param.visres + 2) * sizeof(double));

	coarsen(param.u + np, np, param.local_act_res, uvis_local, param.visres + 2, param.visres + 2, param.start_y, param.local_act_res, stepy);

	if (rank == 0)
	{
		// Root process allocates memory for the global grid and description arrays
		counts = (int *)malloc(sizeof(int) * size);
		displs = (int *)malloc(sizeof(int) * size);

		// Calculate the counts and displacements for MPI_Gatherv
		int sum = 0;
		int int_act_res = param.act_res;
		for (i = 0; i < size; i++)
		{
			int rows = int_act_res / size + (i < int_act_res % size);
			counts[i] = rows * np;
			displs[i] = sum;
			sum += counts[i];
		}
	}

	// Compute recvcounts and displs
	if (0 == rank)
	{
		int offset = 0;
		for (int r = 0; r < param.size; r++)
		{
			int base_rows = param.act_res / param.size;
			int extra = param.act_res % param.size;
			int start_y = r * base_rows + (r < extra ? r : extra);

			int rows = 0;
			for (int i = start_y; i < start_y + base_rows + (r < extra); i += stepy)
				rows++;

			counts[r] = rows * (param.visres + 2);
			displs[r] = offset;
			offset += counts[r];
		}
	}

	// Gather local coarsenings in rank 0
	MPI_Gatherv(
		uvis_local, local_coarse_rows * (param.visres + 2), MPI_DOUBLE, param.uvis, counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// --- FINALIZATION ---
	free(uvis_local);
	if (rank == 0)
	{
		for (i = 0; i < experiment; i++)
		{
			printf("%5d; %5.3f; %5.3f\n", resolution[i], time[i], floprate[i]);
		}

		write_image(resfile, param.uvis, param.visres + 2, param.visres + 2);

		// Clean up buffers
		fclose(resfile);
		free(counts);
		free(displs);
	}

	finalize(&param);

	if (rank == 0)
		free(param.uvis);

	MPI_Finalize();

	return 0;
}