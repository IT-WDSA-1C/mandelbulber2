/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * platonic solid
 * @reference
 * http://www.fractalforums.com/3d-fractal-generation/platonic-dimensions/msg36528/#msg36528
 */

/* ### This file has been autogenerated. Remove this line, to prevent override. ### */

#ifndef DOUBLE_PRECISION
float4 TransfPlatonicSolidIteration(float4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	Q_UNUSED(aux);

	float rho = native_sqrt(length(z)); // the radius
	float theta = mad(native_cos(fractal->platonicSolid.frequency * z.x),
									native_sin(fractal->platonicSolid.frequency * z.y),
									native_cos(fractal->platonicSolid.frequency * z.y)
										* native_sin(fractal->platonicSolid.frequency * z.z))
								+ native_cos(fractal->platonicSolid.frequency * z.z)
										* native_sin(fractal->platonicSolid.frequency * z.x);
	float r = mad(theta, fractal->platonicSolid.amplitude, rho * fractal->platonicSolid.rhoMul);
	z *= r;
	return z;
}
#else
double4 TransfPlatonicSolidIteration(double4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	Q_UNUSED(aux);

	double rho = native_sqrt(length(z)); // the radius
	double theta = mad(native_cos(fractal->platonicSolid.frequency * z.x),
									 native_sin(fractal->platonicSolid.frequency * z.y),
									 native_cos(fractal->platonicSolid.frequency * z.y)
										 * native_sin(fractal->platonicSolid.frequency * z.z))
								 + native_cos(fractal->platonicSolid.frequency * z.z)
										 * native_sin(fractal->platonicSolid.frequency * z.x);
	double r = mad(theta, fractal->platonicSolid.amplitude, rho * fractal->platonicSolid.rhoMul);
	z *= r;
	return z;
}
#endif
