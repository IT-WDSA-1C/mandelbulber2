/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Quaternion3DE - Quaternion fractal with extended controls
 * @reference http://www.fractalforums.com/3d-fractal-generation
 * /true-3d-mandlebrot-type-fractal/
 */

/* ### This file has been autogenerated. Remove this line, to prevent override. ### */

#ifndef DOUBLE_PRECISION
float4 Quaternion3dIteration(float4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{

	aux->r_dz = aux->r_dz * 2.0f * aux->r;
	z = (float4){z.x * z.x - z.y * z.y - z.z * z.z, z.x * z.y, z.x * z.z, z.w};

	float tempL = length(z);
	z *= fractal->transformCommon.constantMultiplier122;
	// if (tempL < 1e-21f) tempL = 1e-21f;
	float4 tempAvgScale = (float4){z.x, native_divide(z.y, 2.0f), native_divide(z.z, 2.0f), z.w};
	float avgScale = native_divide(length(tempAvgScale), tempL);
	float tempAux = aux->r_dz * avgScale;
	aux->r_dz = mad(fractal->transformCommon.scaleA1, (tempAux - aux->r_dz), aux->r_dz);

	if (fractal->transformCommon.rotationEnabled)
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);

	z += fractal->transformCommon.additionConstant000;
	return z;
}
#else
double4 Quaternion3dIteration(double4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{

	aux->r_dz = aux->r_dz * 2.0 * aux->r;
	z = (double4){z.x * z.x - z.y * z.y - z.z * z.z, z.x * z.y, z.x * z.z, z.w};

	double tempL = length(z);
	z *= fractal->transformCommon.constantMultiplier122;
	// if (tempL < 1e-21) tempL = 1e-21;
	double4 tempAvgScale = (double4){z.x, z.y / 2.0, z.z / 2.0, z.w};
	double avgScale = native_divide(length(tempAvgScale), tempL);
	double tempAux = aux->r_dz * avgScale;
	aux->r_dz = mad(fractal->transformCommon.scaleA1, (tempAux - aux->r_dz), aux->r_dz);

	if (fractal->transformCommon.rotationEnabled)
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);

	z += fractal->transformCommon.additionConstant000;
	return z;
}
#endif
