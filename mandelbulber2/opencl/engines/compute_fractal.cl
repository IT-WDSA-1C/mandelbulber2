/*
 * compute_fractal.h
 *
 *  Created on: 21 maj 2017
 *      Author: krzysztof
 */

#ifndef MANDELBULBER2_OPENCL_ENGINES_COMPUTE_FRACTAL_CL_
#define MANDELBULBER2_OPENCL_ENGINES_COMPUTE_FRACTAL_CL_

typedef struct
{
	float4 z;
	float iters;
	float distance;
	float colourIndex;
	bool maxiter;
} formulaOut;

void DummyIteration(float4 *z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
}

formulaOut Fractal(__constant sClInConstants *consts, float3 point, sClCalcParams *calcParam)
{
	// begin
	float dist = 0.0f;
	int N = calcParam->N;
	float4 z;
	z.x = point.x;
	z.y = point.y;
	z.z = point.z;
	z.w = 0.0f;

	float w = 0;

	float4 c = z;
	int i;
	formulaOut out;
	float colourMin = 1e8f;

	// formula init
	sExtendedAuxCl aux;
	// TODO copy aux
	aux.r_dz = 1.0f;
	aux.r = length(z);
	aux.color = 1.0f;
	aux.actualScale = 2.0f; // fractals.GetFractal(fractalIndex)->mandelbox.scale;
	aux.DE = 1.0f;
	aux.c = c;
	aux.const_c = c;
	aux.cw = 0.0f;
	aux.foldFactor = 0.0f;
	aux.minRFactor = 0.0f;
	aux.scaleFactor = 0.0f;
	aux.pseudoKleinianDE = 1.0f;

	// loop
	for (i = 0; i < N; i++)
	{
		int formulaIndex = consts->sequence.hybridSequence[i];
		__constant sFractalCl *fractal = &consts->fractal[formulaIndex];
		aux.i = i;

		switch (formulaIndex)
		{
			case 0: FORMULA_ITER_0(&z, fractal, &aux); break;
			case 1: FORMULA_ITER_1(&z, fractal, &aux); break;
			case 2: FORMULA_ITER_2(&z, fractal, &aux); break;
			case 3: FORMULA_ITER_3(&z, fractal, &aux); break;
			case 4: FORMULA_ITER_4(&z, fractal, &aux); break;
			case 5: FORMULA_ITER_5(&z, fractal, &aux); break;
			case 6: FORMULA_ITER_6(&z, fractal, &aux); break;
			case 7: FORMULA_ITER_7(&z, fractal, &aux); break;
			case 8: FORMULA_ITER_8(&z, fractal, &aux); break;
		}

		if (consts->sequence.addCConstant[formulaIndex])
		{
			if (consts->sequence.juliaEnabled[formulaIndex])
			{
				z += consts->sequence.juliaConstant[formulaIndex]
						 * consts->sequence.constantMultiplier[formulaIndex];
			}
			else
			{
				z += c * consts->sequence.constantMultiplier[formulaIndex];
			}
		}

		aux.r = length(z);

		if (aux.r < colourMin) colourMin = aux.r;

		if (aux.r > consts->sequence.bailout[formulaIndex] || any(isinf(z)))
		{
#ifdef ANALYTIC_LOG_DE
			dist = 0.5f * aux.r * native_log(aux.r) / (aux.r_dz);
#elif ANALYTIC_LINEAR_DE
			dist = (aux.r - 2.0f) / fabs(aux.DE);
#elif ANALYTIC_PSEUDO_KLEINIAN_DE

#else
			dist = length(z);
#endif
			out.colourIndex = colourMin * 5000.0f;
			break;
		}
	}

	// end
	if (dist < 0.0f) dist = 0.0f;
	out.distance = dist;
	out.iters = i;
	out.z = z;

	return out;
}

#endif /* MANDELBULBER2_OPENCL_ENGINES_COMPUTE_FRACTAL_CL_ */