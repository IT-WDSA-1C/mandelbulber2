/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * amazing surf from Mandelbulber3D. Formula proposed by Kali, with features added by Darkbeam
 *
 * This formula has a c.x c.y SWAP
 *
 * @reference
 * http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/
 */

/* ### This file has been autogenerated. Remove this line, to prevent override. ### */

#ifndef DOUBLE_PRECISION
float4 AmazingSurfIteration(float4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	float4 c = aux->const_c;

	aux->actualScale = mad(
		(fabs(aux->actualScale) - 1.0f), fractal->mandelboxVary4D.scaleVary, fractal->mandelbox.scale);

	z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
				- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
	z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
				- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
	// no z fold

	float rr = dot(z, z);
	if (fractal->transformCommon.functionEnabledFalse) // force cylinder fold
		rr -= z.z * z.z;

	float sqrtMinR = native_sqrt(fractal->transformCommon.minR05);
	float dividend = rr < sqrtMinR ? sqrtMinR : min(rr, 1.0f);
	float m = native_divide(aux->actualScale, dividend);
	z *= mad(m, fractal->transformCommon.scale1, 1.0f * (1.0f - fractal->transformCommon.scale1));
	aux->DE = mad(aux->DE, fabs(m), 1.0f);

	if (fractal->transformCommon.addCpixelEnabledFalse)
		z += (float4){c.y, c.x, c.z, c.w} * fractal->transformCommon.constantMultiplier111;

	z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	return z;
}
#else
double4 AmazingSurfIteration(double4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	double4 c = aux->const_c;

	aux->actualScale = mad(
		(fabs(aux->actualScale) - 1.0), fractal->mandelboxVary4D.scaleVary, fractal->mandelbox.scale);

	z.x = fabs(z.x + fractal->transformCommon.additionConstant111.x)
				- fabs(z.x - fractal->transformCommon.additionConstant111.x) - z.x;
	z.y = fabs(z.y + fractal->transformCommon.additionConstant111.y)
				- fabs(z.y - fractal->transformCommon.additionConstant111.y) - z.y;
	// no z fold

	double rr = dot(z, z);
	if (fractal->transformCommon.functionEnabledFalse) // force cylinder fold
		rr -= z.z * z.z;

	double sqrtMinR = native_sqrt(fractal->transformCommon.minR05);
	double dividend = rr < sqrtMinR ? sqrtMinR : min(rr, 1.0);
	double m = native_divide(aux->actualScale, dividend);
	z *= m * fractal->transformCommon.scale1 + 1.0 * (1.0 - fractal->transformCommon.scale1);
	aux->DE = aux->DE * fabs(m) + 1.0;

	if (fractal->transformCommon.addCpixelEnabledFalse)
		z += (double4){c.y, c.x, c.z, c.w} * fractal->transformCommon.constantMultiplier111;

	z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	return z;
}
#endif
