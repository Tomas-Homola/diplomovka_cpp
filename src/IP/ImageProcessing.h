#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <utility>
#include <limits>

// Tomas: mozno do buducnosti kedy sa robila ta trieda Settings, tak k tomu pridat aj tieto enum class a uzavriet to do nejakeho namespace?

// Enum class for different downsampling algorithms.
enum class Algorithm
{
	Averaging = 0,
	SkipPixels = 1
};

// Enum class for different filtration algorithms;
enum class Filtration
{
	NoFiltration = 0,
	GaussianKernel = 1,
	ExplicitHeatEquation = 2
};

class ImageProcessing
{
public:
	ImageProcessing();
	~ImageProcessing();

	/// <summary>
	/// Performs downsampling on the provided image.
	/// </summary>
	/// <param name="originalImgData">-> pointer to original image data to be downsampled.</param>
	/// <param name="originalImgWidth">-> original image width.</param>
	/// <param name="originalImgHeight">-> original image height.</param>
	/// <param name="scaleDownBy">-> parameter for downsampling.</param>
	/// <param name="algorithm">-> specifies what downsampling algorithm to use.</param>
	/// <param name="doFiltration">-> specifies, whether to perform image filtration and what algorithm, 0 for no filtration, see ImageProcessing::Filtration.</param>
	/// <returns>Pointer to downsampled image if successful, nullptr otherwise.</returns>
	float* downsampleImage(float* originalImgData, int originalImgWidth, int originalImgHeight, int scaleDownBy, Algorithm algorithm, Filtration doFiltration = Filtration::ExplicitHeatEquation);

	/// <summary>
	/// Find size of scaled image after downsampling - ensure the right size even if the size was changed by mirroring
	/// </summary>
	/// <param name="originalImgWidth">-> width of the original image data.</param>
	/// <param name="originalImgHeight">-> height of the original image data.</param>
	/// <param name="scaleDownBy">-> parameter of downscaling.</param>
	/// <returns>Pair of scaled width and scaled height respectively.</returns>
	static std::pair<int, int> findScaledImgSize(int originalImgWidth, int originalImgHeight, int scaleDownBy);

private:

	/// <summary>
	/// Downsamples the image via averaging.
	/// </summary>
	/// <param name="originalImgData">-> pointer to original image data.</param>
	/// <param name="originalImgWidth">-> original image width.</param>
	/// <param name="originalImgHeight">-> original image height.</param>
	/// <param name="scaleDownBy">-> parameter for downscaling.</param>
	/// <param name="doFiltration">-> specifies, whether to perform image filtration and what algorithm, 0 for no filtration, see ImageProcessing::Filtration.</param>
	/// <returns>Pointer to downsampled image if successful, nullptr otherwise.</returns>
	float* downsampleByAveraging(float* originalImgData, int originalImgWidth, int originalImgHeight, int scaleDownBy, Filtration doFiltration = Filtration::ExplicitHeatEquation);

	/// <summary>
	/// Downsample algorithm based on skipping every second row and column, done repeatedly if scaleDownBy is greater than 2
	/// </summary>
	/// <param name="originalImgData">-> pointer to the original image data.</param>
	/// <param name="originalImgWidth">-> width of the original image data.</param>
	/// <param name="originalImgHeight">-> height of the original image data.</param>
	/// <param name="scaleDownBy">-> parameter of downscaling.</param>
	/// <param name="doFiltration">->  specifies, whether to perform image filtration and what algorithm, 0 for no filtration, see ImageProcessing::Filtration.</param>
	/// <returns>Pointer to downsampled image if successful, nullptr otherwise.</returns>
	float* downsampleBySkipPixels(float* originalImgData, int originalImgWidth, int originalImgHeight, int scaleDownBy, Filtration doFiltration = Filtration::ExplicitHeatEquation);

	/// <summary>
	/// Mirroring pixels over every edge to create extended image.
	/// </summary>
	/// <param name="originalImgData">-> pointer to the pixel values of the original image.</param>
	/// <param name="imgWidth">-> width of the original image.</param>
	/// <param name="imgHeight">-> height of the original image.</param>
	/// <param name="padding">-> number of pixels for mirroring.</param>
	/// <returns>Pointer to the extended image if successful, nullptr otherwise.</returns>
	float* pixelsMirror(float* originalImgData, const int imgWidth, const int imgHeight, const int padding);

	/// <summary>
	/// Image filtration with convolution using Gaussian kernel.
	/// </summary>
	/// <param name="originalImgData">-> pointer to the pixel values of the original image.</param>
	/// <param name="originalImgWidth">-> width of the original image.</param>
	/// <param name="originalImgHeight">-> height of the original image.</param>
	/// <param name="filteredImgData">-> pointer to the pixel values of the filtered image.</param>
	/// <returns>True, if the filtration is successful, false otherwise.</returns>
	bool filtrationGauss(float* originalImgData, int originalImgWidth, int originalImgHeight, float* filteredImgData);

	/// <summary>
	/// Image filtration via Explicit scheme for the Heat Conduction PDE.
	/// </summary>
	/// <param name="originalImgData">-> pointer to the pixel values of the original image.</param>
	/// <param name="originalImgWidth">-> width of the original image.</param>
	/// <param name="originalImgHeight">-> height of the original image.</param>
	/// <param name="filteredImgData">-> pointer to the pixel values of the filtered image.</param>
	/// <param name="iterNumber">-> number of iterations to perform.</param>
	/// <returns>True, if the filtration is successful, false otherwise.</returns>
	bool filtrationExplicitHeatEq(float* originalImgData, int originalImgWidth, int originalImgHeight, float* filteredImgData, int iterNumber);

	/// <summary>
	/// Function for finding minimum and maximum value of given data.
	/// </summary>
	/// <param name="data">-> pointer to the data.</param>
	/// <param name="dataSize">-> size of the data.</param>
	/// <returns>Pair of float numbers in order: (min, max).</returns>
	std::pair<float, float> findMinMax(float* data, long dataSize);

	/// <summary>
	/// Normalize given data - change its values to the requested range.
	/// </summary>
	/// <param name="data">-> pointer to the data.</param>
	/// <param name="dataSize">-> size of the data.</param>
	/// <param name="dataMin">-> current minimum of the data.</param>
	/// <param name="dataMax">-> current maximum of the data.</param>
	/// <param name="requestedMin">-> requested minimum for the data.</param>
	/// <param name="requestedMax">-> requested maximum for the data.</param>
	void normalizeData(float* data, long dataSize, float dataMin, float dataMax, float requestedMin, float requestedMax);

	/// <summary>
	/// Mirroring columns and rows over lower and right edges to create extended image.
	/// </summary>
	/// <param name="originalImgData">-> pointer to the pixel values of the original image.</param>
	/// <param name="imgWidth">-> width of the original image.</param>
	/// <param name="imgHeight">-> height of the original image.</param>
	/// <param name="paddingColumn">-> number of columns for mirroring.</param>
	/// <param name="paddingRow">-> number of rows for mirroring.</param>
	/// <returns>Pointer to the extended image if successful, nullptr otherwise.</returns>
	float* mirrorColumnsAndRows(float* originalImgData, const int imgWidth, const int imgHeight, const int paddingColumn, const int paddingRow);
};