/**
 * Mandelbulber v2, a 3D fractal generator       ,=#MKNmMMKmmßMNWy,
 *                                             ,B" ]L,,p%%%,,,§;, "K
 * Copyright (C) 2017 Mandelbulber Team        §R-==%w["'~5]m%=L.=~5N
 *                                        ,=mm=§M ]=4 yJKA"/-Nsaj  "Bw,==,,
 * This file is part of Mandelbulber.    §R.r= jw",M  Km .mM  FW ",§=ß., ,TN
 *                                     ,4R =%["w[N=7]J '"5=],""]]M,w,-; T=]M
 * Mandelbulber is free software:     §R.ß~-Q/M=,=5"v"]=Qf,'§"M= =,M.§ Rz]M"Kw
 * you can redistribute it and/or     §w "xDY.J ' -"m=====WeC=\ ""%""y=%"]"" §
 * modify it under the terms of the    "§M=M =D=4"N #"%==A%p M§ M6  R' #"=~.4M
 * GNU General Public License as        §W =, ][T"]C  §  § '§ e===~ U  !§[Z ]N
 * published by the                    4M",,Jm=,"=e~  §  §  j]]""N  BmM"py=ßM
 * Free Software Foundation,          ]§ T,M=& 'YmMMpM9MMM%=w=,,=MT]M m§;'§,
 * either version 3 of the License,    TWw [.j"5=~N[=§%=%W,T ]R,"=="Y[LFT ]N
 * or (at your option)                   TW=,-#"%=;[  =Q:["V""  ],,M.m == ]N
 * any later version.                      J§"mr"] ,=,," =="""J]= M"M"]==ß"
 *                                          §= "=C=4 §"eM "=B:m|4"]#F,§~
 * Mandelbulber is distributed in            "9w=,,]w em%wJ '"~" ,=,,ß"
 * the hope that it will be useful,                 . "K=  ,=RMMMßM"""
 * but WITHOUT ANY WARRANTY;                            .'''
 * without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with Mandelbulber. If not, see <http://www.gnu.org/licenses/>.
 *
 * ###########################################################################
 *
 * Authors: Krzysztof Marczak (buddhi1980@gmail.com), Robert Pancoast (RobertPancoast77@gmail.com),
 *  Sebastian Jennen (jenzebas@gmail.com)
 *
 * cOpenClEngineRenderFractal - prepares and executes fractal rendering on opencl
 */

#include "cimage.hpp"
#include "files.h"
#include "fractal_formulas.hpp"
#include "fractal_list.hpp"
#include "fractal.h"
#include "fractparams.hpp"
#include "global_data.hpp"
#include "nine_fractals.hpp"
#include "opencl_engine_render_fractal.h"

#include "camera_target.hpp"
#include "opencl_hardware.h"
#include "parameters.hpp"
#include "progress_text.hpp"

#ifdef USE_OPENCL
#include "../opencl/fractal_cl.h"
#include "../opencl/fractparams_cl.hpp"
#include "../opencl/material_cl.h"
#endif

cOpenClEngineRenderFractal::cOpenClEngineRenderFractal(cOpenClHardware *_hardware)
		: cOpenClEngine(_hardware)
{
#ifdef USE_OPENCL
	constantInBuffer = nullptr;
	inCLConstBuffer = nullptr;

	inCLBuffer = nullptr;

	rgbbuff = nullptr;
	outCL = nullptr;

	optimalJob.sizeOfPixel = sizeof(sClPixel);

#endif
}

cOpenClEngineRenderFractal::~cOpenClEngineRenderFractal()
{
#ifdef USE_OPENCL
	if (constantInBuffer) delete constantInBuffer;
	if (inCLConstBuffer) delete inCLConstBuffer;
	if (inCLBuffer) delete inCLBuffer;
	if (rgbbuff) delete rgbbuff;
	if (outCL) delete outCL;
#endif
}

#ifdef USE_OPENCL
bool cOpenClEngineRenderFractal::LoadSourcesAndCompile(const cParameterContainer *params)
{
	programsLoaded = false;
	readyForRendering = false;

	emit updateProgressAndStatus(tr("OpenCl - initializing"), tr("Compiling sources"), 0.0);

	QByteArray progEngine;
	try
	{
		QString openclPath = systemData.sharedDir + "opencl" + QDir::separator();
		QString openclEnginePath = openclPath + "engines" + QDir::separator();

		// passthrough define constants
		progEngine.append("#define USE_OPENCL 1\n");
		progEngine.append("#define NUMBER_OF_FRACTALS " + QString::number(NUMBER_OF_FRACTALS) + "\n");

		progEngine.append("#define SQRT_1_3 " + QString::number(SQRT_1_3) + "\n");
		progEngine.append("#define SQRT_1_2 " + QString::number(SQRT_1_2) + "\n");
		progEngine.append("#define SQRT_2_3 " + QString::number(SQRT_2_3) + "\n");
		progEngine.append("#define SQRT_3_2 " + QString::number(SQRT_3_2) + "\n");
		progEngine.append("#define SQRT_3_4 " + QString::number(SQRT_3_4) + "\n");
		progEngine.append("#define SQRT_3_4d2 " + QString::number(SQRT_3_4d2) + "\n");
		progEngine.append("#define SQRT_3 " + QString::number(SQRT_3) + "\n");
		progEngine.append("#define FRAC_1_3 " + QString::number(FRAC_1_3) + "\n");
		progEngine.append("#define M_PI_180 " + QString::number(M_PI_180) + "\n");
		progEngine.append("#define M_PI_8 " + QString::number(M_PI_8) + "\n");

		progEngine.append("#define IFS_VECTOR_COUNT " + QString::number(IFS_VECTOR_COUNT) + "\n");
		progEngine.append("#define HYBRID_COUNT " + QString::number(HYBRID_COUNT) + "\n");
		progEngine.append("#define MANDELBOX_FOLDS " + QString::number(MANDELBOX_FOLDS) + "\n");
		progEngine.append("#define Q_UNUSED(x) (void)x;\n");

		QStringList clHeaderFiles;
		clHeaderFiles.append("opencl_typedefs.h");			 // definitions of common opencl types
		clHeaderFiles.append("opencl_algebra.h");				 // algebra for kernels
		clHeaderFiles.append("common_params_cl.hpp");		 // common parameters
		clHeaderFiles.append("image_adjustments_cl.h");	// image adjustments
		clHeaderFiles.append("fractal_cl.h");						 // fractal data structures
		clHeaderFiles.append("fractparams_cl.hpp");			 // rendering data structures
		clHeaderFiles.append("fractal_sequence_cl.h");	 // sequence of fractal formulas
		clHeaderFiles.append("material_cl.h");					 // materials
		clHeaderFiles.append("input_data_structures.h"); // main data structures

		for (int i = 0; i < clHeaderFiles.size(); i++)
		{
			progEngine.append("#include \"" + openclPath + clHeaderFiles.at(i) + "\"\n");
		}

		// fractal formulas - only actually used
		for (int i = 0; i < listOfUsedFormulas.size(); i++)
		{
			QString formulaName = listOfUsedFormulas.at(i);
			if (formulaName != "")
			{
				progEngine.append("#include \"" + systemData.sharedDir + "formula" + QDir::separator()
													+ "opencl" + QDir::separator() + formulaName + ".cl\"\n");
			}
		}

		// compute fractal
		progEngine.append("#include \"" + openclEnginePath + "compute_fractal.cl\"\n");

		// calculate distance
		progEngine.append("#include \"" + openclEnginePath + "calculate_distance.cl\"\n");

		if (params->Get<int>("gpu_mode") != clRenderEngineTypeFast)
		{
			// shaders
			progEngine.append("#include \"" + openclEnginePath + "shaders.cl\"\n");
		}

		// main engine
		QString engineFileName;
		switch (enumClRenderEngineMode(params->Get<int>("gpu_mode")))
		{
			case clRenderEngineTypeFast: engineFileName = "fast_engine.cl"; break;
			case clRenderEngineTypeLimited: engineFileName = "limited_engine.cl"; break;
			case clRenderEngineTypeFull: engineFileName = "full_engine.cl"; break;
		}
		QString engineFullFileName = openclEnginePath + engineFileName;
		progEngine.append(LoadUtf8TextFromFile(engineFullFileName));
	}
	catch (const QString &ex)
	{
		qCritical() << "OpenCl program error: " << ex;
		return false;
	}

	// building OpenCl kernel
	QString errorString;

	QElapsedTimer timer;
	timer.start();
	if (Build(progEngine, &errorString))
	{
		programsLoaded = true;
	}
	else
	{
		programsLoaded = false;
		WriteLog(errorString, 0);
	}
	qDebug() << "Opencl build time [s]" << timer.nsecsElapsed() / 1.0e9;

	return programsLoaded;
}

void cOpenClEngineRenderFractal::SetParameters(
	const cParameterContainer *paramContainer, const cFractalContainer *fractalContainer)
{
	if (constantInBuffer) delete constantInBuffer;
	constantInBuffer = new sClInConstants;

	definesCollector.clear();

	sParamRender *paramRender = new sParamRender(paramContainer);
	cNineFractals *fractals = new cNineFractals(fractalContainer, paramContainer);

	// update camera rotation data (needed for simplified calculation in opencl kernel)
	cCameraTarget cameraTarget(paramRender->camera, paramRender->target, paramRender->topVector);
	paramRender->viewAngle = cameraTarget.GetRotation() * 180.0 / M_PI;
	paramRender->resolution = 1.0 / paramRender->imageHeight;

	// temporary code to copy general parameters
	constantInBuffer->params = clCopySParamRenderCl(*paramRender);

	// TODO
	constantInBuffer->params.viewAngle = toClFloat3(paramRender->viewAngle * M_PI / 180.0);

	for (int i = 0; i < NUMBER_OF_FRACTALS; i++)
	{
		constantInBuffer->fractal[i] = clCopySFractalCl(*fractals->GetFractal(i));
	}

	fractals->CopyToOpenclData(&constantInBuffer->sequence);

	// define distance estimation method
	fractal::enumDEType deType = fractals->GetDEType(0);
	fractal::enumDEFunctionType deFunctionType = fractals->GetDEFunctionType(0);

	if (deType == fractal::analyticDEType)
	{
		definesCollector += " -DANALYTIC_DE";
		switch (deFunctionType)
		{
			case fractal::linearDEFunction: definesCollector += " -DANALYTIC_LINEAR_DE"; break;
			case fractal::logarithmicDEFunction: definesCollector += " -DANALYTIC_LOG_DE"; break;
			case fractal::pseudoKleinianDEFunction:
				definesCollector += " -DANALYTIC_PSEUDO_KLEINIAN_DE";
				break;
			default: break;
		}
	}
	else if (deType == fractal::deltaDEType)
	{
		definesCollector += " -DDELTA_DE";
		switch (deFunctionType)
		{
			case fractal::linearDEFunction: definesCollector += " -DDELTA_LINEAR_DE"; break;
			case fractal::logarithmicDEFunction: definesCollector += " -DDELTA_LOG_DE"; break;
			case fractal::pseudoKleinianDEFunction:
				definesCollector += " -DDELTA_PSEUDO_KLEINIAN_DE";
				break;
			default: break;
		}
	}

	if (paramRender->limitsEnabled) definesCollector += " -DLIMITS_ENABLED";

	listOfUsedFormulas.clear();

	// creating list of used formuals
	for (int i = 0; i < NUMBER_OF_FRACTALS; i++)
	{
		fractal::enumFractalFormula fractalFormula = fractals->GetFractal(i)->formula;
		int listIndex = cNineFractals::GetIndexOnFractalList(fractalFormula);
		QString formulaName = fractalList.at(listIndex).internalName;
		listOfUsedFormulas.append(formulaName);
	}

	// adding #defines to the list
	for (int i = 0; i < listOfUsedFormulas.size(); i++)
	{
		QString internalID = toCamelCase(listOfUsedFormulas.at(i));
		if (internalID != "")
		{
			QString functionName = internalID.left(1).toUpper() + internalID.mid(1) + "Iteration";
			definesCollector += " -DFORMULA_ITER_" + QString::number(i) + "=" + functionName;
		}
		else
		{
			QString functionName = "DummyIteration";
			definesCollector += " -DFORMULA_ITER_" + QString::number(i) + "=" + functionName;
		}
	}

	listOfUsedFormulas = listOfUsedFormulas.toSet().toList(); // eliminate duplicates

	qDebug() << "Constant buffer size" << sizeof(sClInConstants);

	inBuffer.clear();
	QMap<int, cMaterial> materials;
	CreateMaterialsMap(paramContainer, &materials, true);
	BuildMaterialsData(&inBuffer, materials);

	delete paramRender;
	delete fractals;
}

bool cOpenClEngineRenderFractal::PreAllocateBuffers(const cParameterContainer *params)
{
	cl_int err;

	if (hardware->ContextCreated())
	{

		if (inCLConstBuffer) delete inCLConstBuffer;
		inCLConstBuffer = new cl::Buffer(*hardware->getContext(),
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(sClInConstants), constantInBuffer, &err);
		if (!checkErr(err,
					"cl::Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, "
					"sizeof(sClInConstants), constantInBuffer, &err)"))
		{
			emit showErrorMessage(QObject::tr("Cannot create OpenCL buffer for constants"),
				cErrorMessage::errorMessage, nullptr);
			return false;
		}

		// this buffer will be used for color palettes, lights, etc...
		if (inCLBuffer) delete inCLBuffer;
		inCLBuffer = new cl::Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			size_t(inBuffer.size()), inBuffer.data(), &err);
		if (!checkErr(err,
					"Buffer::Buffer(*hardware->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, "
					"sizeof(sClInBuff), inBuffer, &err)"))
		{
			emit showErrorMessage(QObject::tr("Cannot create OpenCL buffer for variable data"),
				cErrorMessage::errorMessage, nullptr);
			return false;
		}

		size_t buffSize = optimalJob.stepSize * sizeof(sClPixel);
		if (rgbbuff) delete rgbbuff;
		rgbbuff = new sClPixel[buffSize];

		if (outCL) delete outCL;
		outCL = new cl::Buffer(
			*hardware->getContext(), CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buffSize, rgbbuff, &err);
		if (!checkErr(
					err, "*context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buffSize, rgbbuff, &err"))
		{
			emit showErrorMessage(
				QObject::tr("Cannot create OpenCL output buffer"), cErrorMessage::errorMessage, nullptr);
			return false;
		}
	}
	else
	{
		emit showErrorMessage(
			QObject::tr("OpenCL context is not ready"), cErrorMessage::errorMessage, nullptr);
		return false;
	}

	return true;
}

bool cOpenClEngineRenderFractal::ReAllocateImageBuffers()
{
	cl_int err;

	size_t buffSize = optimalJob.stepSize * sizeof(sClPixel);
	if (rgbbuff) delete rgbbuff;
	rgbbuff = new sClPixel[buffSize];

	if (outCL) delete outCL;
	outCL = new cl::Buffer(
		*hardware->getContext(), CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buffSize, rgbbuff, &err);
	if (!checkErr(err, "*context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buffSize, rgbbuff, &err"))
		return false;
	else
		return true;
}

// TODO:
// This is the hotspot for heterogenous execution
// requires opencl for all compute resources
bool cOpenClEngineRenderFractal::Render(cImage *image)
{
	if (programsLoaded)
	{
		// The image resolution determines the total amount of work
		int width = image->GetWidth();
		int height = image->GetHeight();

		cProgressText progressText;
		progressText.ResetTimer();

		emit updateProgressAndStatus(tr("OpenCl - rendering image"), progressText.getText(0.0), 0.0);

		QElapsedTimer timer;
		timer.start();

		QList<int> lastRenderedLines;

		// TODO:
		// insert device for loop here
		// requires initialization for all opencl devices
		// requires optimalJob for all opencl devices
		for (int pixelIndex = 0; pixelIndex < width * height; pixelIndex += optimalJob.stepSize)
		{
			size_t pixelsLeft = width * height - pixelIndex;
			UpdateOptimalJobStart(pixelsLeft);

			ReAllocateImageBuffers();

			// assign parameters to kernel
			if (!AssingParametersToKernel(pixelIndex)) return false;

			// writing data to queue
			if (!WriteDataBuffertsToQueue()) return false;

			// processing queue
			if (!ProcessQueue()) return false;

			// update image when OpenCl kernel is working
			if (lastRenderedLines.size() > 0)
			{
				QElapsedTimer timerImageRefresh;
				timerImageRefresh.start();
				image->NullPostEffect(&lastRenderedLines);
				image->CompileImage(&lastRenderedLines);
				image->ConvertTo8bit();
				image->UpdatePreview(&lastRenderedLines);
				image->GetImageWidget()->update();
				lastRenderedLines.clear();
				optimalJob.optimalProcessingCycle = 2.0 * timerImageRefresh.elapsed() / 1000.0;
				if (optimalJob.optimalProcessingCycle < 0.1) optimalJob.optimalProcessingCycle = 0.1;
			}

			if (!ReadBuffersFromQueue()) return false;

			UpdateOptimalJobEnd();

			// Collect Pixel information from the rgbbuff
			// Populate the data into image->Put
			for (unsigned int i = 0; i < optimalJob.stepSize; i++)
			{
				unsigned int a = pixelIndex + i;
				sClPixel pixelCl = rgbbuff[i];
				sRGBFloat pixel = {pixelCl.R, pixelCl.G, pixelCl.B};
				sRGB8 color = {pixelCl.colR, pixelCl.colG, pixelCl.colB};
				unsigned short opacity = pixelCl.opacity;
				unsigned short alpha = pixelCl.alpha;
				int x = a % width;
				int y = a / width;

				image->PutPixelImage(x, y, pixel);
				image->PutPixelZBuffer(x, y, rgbbuff[i].zBuffer);
				image->PutPixelColor(x, y, color);
				image->PutPixelOpacity(x, y, opacity);
				image->PutPixelAlpha(x, y, alpha);
			}

			for (unsigned int i = 0; i < optimalJob.stepSize; i += width)
			{
				unsigned int a = pixelIndex + i;
				int y = a / width;
				lastRenderedLines.append(y);
			}

			double percentDone = double(pixelIndex) / (width * height);
			emit updateProgressAndStatus(
				tr("OpenCl - rendering image"), progressText.getText(percentDone), percentDone);
			gApplication->processEvents();
		}

		qDebug() << "GPU jobs finished";
		qDebug() << "OpenCl Rendering time [s]" << timer.nsecsElapsed() / 1.0e9;

		// refresh image at end
		image->NullPostEffect();

		WriteLog("image->CompileImage()", 2);
		image->CompileImage();

		if (image->IsPreview())
		{
			WriteLog("image->ConvertTo8bit()", 2);
			image->ConvertTo8bit();
			WriteLog("image->UpdatePreview()", 2);
			image->UpdatePreview();
			WriteLog("image->GetImageWidget()->update()", 2);
			image->GetImageWidget()->update();
		}

		emit updateProgressAndStatus(tr("OpenCl - rendering finished"), progressText.getText(1.0), 1.0);

		return true;
	}
	else
	{
		return false;
	}
}

QString cOpenClEngineRenderFractal::toCamelCase(const QString &s)
{
	QStringList upperCaseLookup({"Vs", "Kifs", "De", "Xyz", "Cxyz", "Vcl", "Chs"});
	QStringList parts = s.split('_', QString::SkipEmptyParts);
	for (int i = 1; i < parts.size(); ++i)
	{
		parts[i].replace(0, 1, parts[i][0].toUpper());

		// rewrite to known capital names in iteration function names
		if (upperCaseLookup.contains(parts[i]))
		{
			parts[i] = parts[i].toUpper();
		}
	}

	return parts.join("");
}

bool cOpenClEngineRenderFractal::AssingParametersToKernel(int pixelIndex)
{
	cl_int err = kernel->setArg(0, *outCL); // output image

	if (!checkErr(err, "kernel->setArg(0, *outCL)"))
	{
		emit showErrorMessage(QObject::tr("Cannot set OpenCL argument for output data"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = kernel->setArg(1, *inCLBuffer); // input data in global memory
	if (!checkErr(err, "kernel->setArg(1, *inCLBuffer)"))
	{
		emit showErrorMessage(QObject::tr("Cannot set OpenCL argument for input data"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = kernel->setArg(2, *inCLConstBuffer); // input data in constant memory (faster than global)
	if (!checkErr(err, "kernel->setArg(2, *inCLConstBuffer)"))
	{
		emit showErrorMessage(QObject::tr("Cannot set OpenCL argument for constant data"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = kernel->setArg(3, pixelIndex); // pixel offset
	if (!checkErr(err, "kernel->setArg(3, pixelIndex)"))
	{
		emit showErrorMessage(QObject::tr("Cannot set OpenCL argument for pixel index"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	return true;
}

bool cOpenClEngineRenderFractal::WriteDataBuffertsToQueue()
{
	cl_int err = queue->enqueueWriteBuffer(*inCLBuffer, CL_TRUE, 0, inBuffer.size(), inBuffer.data());

	size_t usedGPUdMem = optimalJob.sizeOfPixel * optimalJob.stepSize;
	qDebug() << "Used GPU mem (KB): " << usedGPUdMem / 1024;

	if (!checkErr(err, "ComamndQueue::enqueueWriteBuffer(inCLBuffer)"))
	{
		emit showErrorMessage(QObject::tr("Cannot enqueue writing OpenCL input buffers"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = queue->finish();
	if (!checkErr(err, "ComamndQueue::finish() - inCLBuffer"))
	{
		emit showErrorMessage(QObject::tr("Cannot finish writing OpenCL input buffers"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = queue->enqueueWriteBuffer(
		*inCLConstBuffer, CL_TRUE, 0, sizeof(sClInConstants), constantInBuffer);
	if (!checkErr(err, "ComamndQueue::enqueueWriteBuffer(inCLConstBuffer)"))
	{
		emit showErrorMessage(QObject::tr("Cannot enqueue writing OpenCL constant buffers"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = queue->finish();
	if (!checkErr(err, "ComamndQueue::finish() - inCLConstBuffer"))
	{
		emit showErrorMessage(QObject::tr("Cannot finish writing OpenCL constant buffers"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	return true;
}

bool cOpenClEngineRenderFractal::ProcessQueue()
{
	cl_int err = queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(optimalJob.stepSize),
		cl::NDRange(optimalJob.workGroupSize));
	if (!checkErr(err, "ComamndQueue::enqueueNDRangeKernel()"))
	{
		emit showErrorMessage(
			QObject::tr("Cannot enqueue OpenCL rendering jobs"), cErrorMessage::errorMessage, nullptr);
		return false;
	}

	return true;
}

bool cOpenClEngineRenderFractal::ReadBuffersFromQueue()
{
	size_t buffSize = optimalJob.stepSize * sizeof(sClPixel);

	cl_int err = queue->enqueueReadBuffer(*outCL, CL_TRUE, 0, buffSize, rgbbuff);
	if (!checkErr(err, "ComamndQueue::enqueueReadBuffer()"))
	{
		emit showErrorMessage(QObject::tr("Cannot enqueue reading OpenCL output buffers"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	err = queue->finish();
	if (!checkErr(err, "ComamndQueue::finish() - ReadBuffer"))
	{
		emit showErrorMessage(QObject::tr("Cannot finish reading OpenCL output buffers"),
			cErrorMessage::errorMessage, nullptr);
		return false;
	}

	return true;
}

#endif
