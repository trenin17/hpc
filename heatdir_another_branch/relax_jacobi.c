/*
 * relax_jacobi.c
 *
 * Jacobi Relaxation
 *
 */

#include "heat.h"
#include <omp.h>

/*
 * Combined Jacobi iteration and residual calculation
 */
double relax_jacobi_residual(double * restrict u, double * restrict uhelp, unsigned sizex, unsigned sizey) {
    unsigned i, j;
    double *urow, *urow_above, *urow_below, *uhelp_row;
    double diff = 0.0;
	double sum = 0.0;

	#pragma omp parallel for private(i, j, urow, urow_above, urow_below, uhelp_row, diff) reduction(+:sum)
    for (i = 1; i < sizex - 1; i++) {
        urow = u + i * sizex;
        urow_above = urow - sizex;
        urow_below = urow + sizex;
        uhelp_row = uhelp + i * sizex;
        for (j = 1; j < sizey - 1; j++) {
            uhelp_row[j] = 0.25 * (urow[j - 1] + urow[j + 1] + urow_above[j] + urow_below[j]);
            diff = uhelp_row[j] - urow[j];
            sum += diff * diff;
        }
    }

    return sum;
}
