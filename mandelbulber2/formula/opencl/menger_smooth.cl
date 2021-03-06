/**
 * Mandelbulber v2, a 3D fractal generator  _%}}i*<.        ____                _______
 * Copyright (C) 2017 Mandelbulber Team   _>]|=||i=i<,     / __ \___  ___ ___  / ___/ /
 *                                        \><||i|=>>%)    / /_/ / _ \/ -_) _ \/ /__/ /__
 * This file is part of Mandelbulber.     )<=i=]=|=i<>    \____/ .__/\__/_//_/\___/____/
 * The project is licensed under GPLv3,   -<>>=|><|||`        /_/
 * see also COPYING file in this folder.    ~+{i%+++
 *
 * Menger Smooth
 * http://www.fractalforums.com/fragmentarium/help-t22583/
 */

/* ### This file has been autogenerated. Remove this line, to prevent override. ### */

#ifndef DOUBLE_PRECISION
float4 MengerSmoothIteration(float4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	float sc1 = fractal->transformCommon.scale3 - 1.0f;							 // 3 - 1 = 2f, 2/3 = 0.6667f;
	float sc2 = native_divide(sc1, fractal->transformCommon.scale3); //  8 - 1 = 7f, 7/8 = 0.89ish;
	float OffsetS = fractal->transformCommon.offset0005;						 //

	if (fractal->transformCommon.functionEnabled)
	{
		// the closer to origin the greater the effect of OffsetSQ
		z = (float4){native_sqrt(mad(z.x, z.x, OffsetS)), native_sqrt(mad(z.y, z.y, OffsetS)),
			native_sqrt(mad(z.z, z.z, OffsetS)), z.w};
	}

	float t;
	float4 OffsetC = fractal->transformCommon.offset1105;

	t = z.x - z.y;
	t = 0.5f * (t - native_sqrt(mad(t, t, OffsetS)));
	z.x = z.x - t;
	z.y = z.y + t;

	t = z.x - z.z;
	t = 0.5f * (t - native_sqrt(mad(t, t, OffsetS)));
	z.x = z.x - t;
	z.z = z.z + t;

	t = z.y - z.z;
	t = 0.5f * (t - native_sqrt(mad(t, t, OffsetS)));
	z.y = z.y - t;
	z.z = z.z + t;

	z.z = mad(-sc2, OffsetC.z, z.z); // sc2 reduces C.z
	z.z = -native_sqrt(mad(z.z, z.z, OffsetS));
	z.z = mad(sc2, OffsetC.z, z.z);

	z.x = mad(fractal->transformCommon.scale3, z.x, -OffsetC.x * sc1); // sc1 scales up C.x
	z.y = mad(fractal->transformCommon.scale3, z.y, -OffsetC.y * sc1);
	z.z = fractal->transformCommon.scale3 * z.z;

	aux->DE *= fractal->transformCommon.scale3;

	if (fractal->transformCommon.rotationEnabled
			&& aux->i >= fractal->transformCommon.startIterationsR
			&& aux->i < fractal->transformCommon.stopIterationsR)
	{
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	}

	if (fractal->transformCommon.functionEnabledzFalse)
	{
		float4 zA = (aux->i == fractal->transformCommon.intA) ? z : (float4){};
		float4 zB = (aux->i == fractal->transformCommon.intB) ? z : (float4){};
		z = (z * fractal->transformCommon.scale1) + (zA * fractal->transformCommon.offsetA0)
				+ (zB * fractal->transformCommon.offsetB0);
		aux->DE *= fractal->transformCommon.scale1;
	}
	return z;
}
#else
double4 MengerSmoothIteration(double4 z, __constant sFractalCl *fractal, sExtendedAuxCl *aux)
{
	double sc1 = fractal->transformCommon.scale3 - 1.0;								// 3 - 1 = 2, 2/3 = 0.6667;
	double sc2 = native_divide(sc1, fractal->transformCommon.scale3); //  8 - 1 = 7, 7/8 = 0.89ish;
	double OffsetS = fractal->transformCommon.offset0005;							//

	if (fractal->transformCommon.functionEnabled)
	{
		// the closer to origin the greater the effect of OffsetSQ
		z = (double4){native_sqrt(mad(z.x, z.x, OffsetS)), native_sqrt(mad(z.y, z.y, OffsetS)),
			native_sqrt(mad(z.z, z.z, OffsetS)), z.w};
	}

	double t;
	double4 OffsetC = fractal->transformCommon.offset1105;

	t = z.x - z.y;
	t = 0.5 * (t - native_sqrt(mad(t, t, OffsetS)));
	z.x = z.x - t;
	z.y = z.y + t;

	t = z.x - z.z;
	t = 0.5 * (t - native_sqrt(mad(t, t, OffsetS)));
	z.x = z.x - t;
	z.z = z.z + t;

	t = z.y - z.z;
	t = 0.5 * (t - native_sqrt(mad(t, t, OffsetS)));
	z.y = z.y - t;
	z.z = z.z + t;

	z.z = mad(-sc2, OffsetC.z, z.z); // sc2 reduces C.z
	z.z = -native_sqrt(mad(z.z, z.z, OffsetS));
	z.z = mad(sc2, OffsetC.z, z.z);

	z.x = mad(fractal->transformCommon.scale3, z.x, -OffsetC.x * sc1); // sc1 scales up C.x
	z.y = mad(fractal->transformCommon.scale3, z.y, -OffsetC.y * sc1);
	z.z = fractal->transformCommon.scale3 * z.z;

	aux->DE *= fractal->transformCommon.scale3;

	if (fractal->transformCommon.rotationEnabled
			&& aux->i >= fractal->transformCommon.startIterationsR
			&& aux->i < fractal->transformCommon.stopIterationsR)
	{
		z = Matrix33MulFloat4(fractal->transformCommon.rotationMatrix, z);
	}

	if (fractal->transformCommon.functionEnabledzFalse)
	{
		double4 zA = (aux->i == fractal->transformCommon.intA) ? z : (double4){};
		double4 zB = (aux->i == fractal->transformCommon.intB) ? z : (double4){};
		z = (z * fractal->transformCommon.scale1) + (zA * fractal->transformCommon.offsetA0)
				+ (zB * fractal->transformCommon.offsetB0);
		aux->DE *= fractal->transformCommon.scale1;
	}
	return z;
}
#endif
