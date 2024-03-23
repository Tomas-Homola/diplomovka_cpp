#include "ImageProcessing.h"

ImageProcessing::ImageProcessing()
{
}

ImageProcessing::~ImageProcessing()
{
}

float* ImageProcessing::downsampleImage(float* originalImgData, int originalImgWidth, int originalImgHeight, int scaleDownBy, Algorithm algorithm, Filtration doFiltration)
{
	// sem budeme ukladat finalny vysledok z downsamplingu a potom ten tato funkcia vrati
	float* scaledImgData = nullptr;

	//run chosen algorithm for subsampling
	if (algorithm == Algorithm::Averaging)
	{
		scaledImgData = downsampleByAveraging(originalImgData, originalImgWidth, originalImgHeight, scaleDownBy, doFiltration);
		if (!scaledImgData) // check if downsampling successful
			return nullptr;
	}
	else if (algorithm == Algorithm::SkipPixels)
	{
		scaledImgData = downsampleBySkipPixels(originalImgData, originalImgWidth, originalImgHeight, scaleDownBy, doFiltration);
		if (!scaledImgData) // check if downsampling successful
			return nullptr;
	}
	// Tomas: tu uz netreba nic kontrolovat, lebo ak sa da nejaka ina hodnota, ktora nie je v Algorithm, tak sa do "scaledImgData" nic neulozi a pri deklaracii to uz je nastavene na nullptr, takze funkcia vrati presne nullptr

	return scaledImgData;
}

std::pair<int, int> ImageProcessing::findScaledImgSize(int originalImgWidth, int originalImgHeight, int scaleDownBy)
{
	int scaledImgWidth, scaledImgHeight;

	//get scaled image width
	if (originalImgWidth % scaleDownBy == 0)
	{
		scaledImgWidth = originalImgWidth / scaleDownBy;
	}
	else
	{
		scaledImgWidth = static_cast<int>((double)originalImgWidth / scaleDownBy + 1); //round up
	}

	//get scaled image height
	if (originalImgHeight % scaleDownBy == 0)
	{
		scaledImgHeight = originalImgHeight / scaleDownBy;
	}
	else
	{
		scaledImgHeight = static_cast<int>((double)originalImgHeight / scaleDownBy + 1); //round up
	}

	return std::pair<int, int>(scaledImgWidth, scaledImgHeight);
}

float* ImageProcessing::downsampleByAveraging(float* originalImgData, int originalImgWidth, int originalImgHeight, int scaleDownBy, Filtration doFiltration)
{
	if (originalImgData == nullptr) // check if not nullptr
		return nullptr;

	// find scaled image size based on the "scaleDownBy" parameter
	auto scaledImgSize = findScaledImgSize(originalImgWidth, originalImgHeight, scaleDownBy);
	int scaledImgWidth = scaledImgSize.first;
	int scaledImgHeight = scaledImgSize.second;

	// allocate memory for the scaled-down image
	float* scaledImgData = new float[scaledImgWidth * scaledImgHeight] {0.0F};
	if (scaledImgData == nullptr) // check if allocation successful
		return nullptr;

	// choose what data to use onwards (original data by default)
	float* chosenImgData = originalImgData;
	int chosenImgWidth = originalImgWidth;
	int chosenImgHeight = originalImgHeight;

	// extend original image data if necessary
	float* extendedImgData = nullptr;

	// find by how many columns and rows to extend original image
	int addCols = scaledImgWidth * scaleDownBy - originalImgWidth;
	int addRows = scaledImgHeight * scaleDownBy - originalImgHeight;

	if ((addCols != 0) || (addRows != 0))
	{
		extendedImgData = mirrorColumnsAndRows(originalImgData, originalImgWidth, originalImgHeight, addCols, addRows);
		if (extendedImgData == nullptr) // check if image extention successful
		{
			delete[] scaledImgData;
			return nullptr;
		}

		// chose extended image data to use onwards
		chosenImgData = extendedImgData;
		chosenImgWidth = originalImgWidth + addCols;
		chosenImgHeight = originalImgHeight + addRows;
	}

	//#################### PERFORM SCALING ####################//
	int maskPixelCount = scaleDownBy * scaleDownBy;
	int I = -1, J = -1;
	int index = -1;
	float pixelSum = 0.0F;

	// iterate over scaled image pixels -> (i,j): scaled image
	for (int i = 0; i < scaledImgHeight; i++)
	{
		for (int j = 0; j < scaledImgWidth; j++)
		{
			// iterate over original pixels inside mask -> (I,J): original image, (k,l): mask
			for (int k = 0; k < scaleDownBy; k++)
			{
				for (int l = 0; l < scaleDownBy; l++)
				{
					// compute pixel indices in original image
					I = i * scaleDownBy + k;
					J = j * scaleDownBy + l;

					if ((I < chosenImgHeight) && (J < chosenImgWidth)) // check if inside original image
					{
						index = I * chosenImgWidth + J; // compute correct index
						pixelSum += chosenImgData[index]; // add pixel value
					}
				}
			} // END of mask iteration

			// write new pixel value to the scaled-down image
			index = i * scaledImgWidth + j;
			scaledImgData[index] = pixelSum / maskPixelCount; // save average value from the pixels inside mask
			pixelSum = 0.0F;
		} // proceed to new pixel
	}

	// free extended image data
	delete[] extendedImgData;

	// choose which data to return (either raw scaled or filtered scaled, raw scaled by default)
	float* chosenScaledImgData = scaledImgData;

	// filter scaled image data if required
	float* filteredScaledImgData = nullptr;

	if (doFiltration != Filtration::NoFiltration) // check if filtration should be performed
	{
		//printf("performing filtration...");
		// allocate memory for the filtered image data
		filteredScaledImgData = new float[scaledImgWidth * scaledImgHeight] {0.0F};
		if (filteredScaledImgData == nullptr) // check if allocation successful
		{
			delete[] scaledImgData; // free memory of the scaled image
			return nullptr;
		}

		// perform filtration via selected algorithm
		bool filtrationOK = false;

		if (doFiltration == Filtration::GaussianKernel)
		{
			filtrationOK = filtrationGauss(scaledImgData, scaledImgWidth, scaledImgHeight, filteredScaledImgData);
		}
		// Tomas: tu asi ani netreba davat "else if", lebo ked sa nesplni prva podmienka, tak automaticky ide program dalej na dalsiu podmienku a na raz by nemali nikdy nastat
		if (doFiltration == Filtration::ExplicitHeatEquation)
		{
			filtrationOK = filtrationExplicitHeatEq(scaledImgData, scaledImgWidth, scaledImgHeight, filteredScaledImgData, 1);
		}

		if (!filtrationOK) // check if filtration ended ok
		{
			// if filtration failed, free both allocated memory spaces
			delete[] filteredScaledImgData;
			delete[] scaledImgData;
			return nullptr;
		}

		delete[] scaledImgData; // free memory for just scaled image
		chosenScaledImgData = filteredScaledImgData; // choose filtered scaled image
		//printf(" done\n");
	}

	// find min/max values for original and scaled image data
	//std::pair<float, float> minmax = findMinMax(originalImgData, originalImgWidth * originalImgHeight);
	//float originalMin = minmax.first;
	//float originalMax = minmax.second;

	//minmax = findMinMax(chosenScaledImgData, scaledImgWidth * scaledImgHeight);
	//float scaledMin = minmax.first;
	//float scaledMax = minmax.second;

	// normalize data
	//normalizeData(chosenScaledImgData, scaledImgWidth * scaledImgHeight, scaledMin, scaledMax, originalMin, originalMax);

	// return scaled (and filtered, if filtration was done) image
	return chosenScaledImgData;
}

float* ImageProcessing::downsampleBySkipPixels(float* originalImgData, int originalImgWidth, int originalImgHeight, int scaleDownBy, Filtration doFiltration)
{
	if (originalImgData == nullptr)
	{
		return nullptr;
	}

	float* chosenImgData = originalImgData;
	float* filteredImgData = nullptr;

	//------------IDENTIFY SCALE-DOWN PARAMETER------------

	//check, if scaleDownBy is the power of 2 - if yes, do repeated downsampling by 2, if no, do downsampling by scaleDownBy once
	double check = log(scaleDownBy) / log(2);
	int numRepeat, scaleParameter;

	if (check - static_cast<int>(check) == 0) //scaleDownBy is the power of 2
	{
		numRepeat = static_cast<int>(check);
		scaleParameter = 2;
	}
	else //scaleDownBy is not the power of 2
	{
		numRepeat = 1;
		scaleParameter = scaleDownBy;
	}

	int scaledImgWidth = 0;
	int scaledImgHeight = 0;
	int prevWidth = originalImgWidth;
	int prevHeight = originalImgHeight;
	float* scaledImgData = nullptr;

	//---------------------DOWNSAMPLING--------------------

	//repeated downsampling by 2 or by scaleDownBy
	for (int i = 0; i < numRepeat; i++)
	{
		//-------------------FILTRATION------------------

		if (doFiltration != Filtration::NoFiltration) // check if filtration should be done
		{
			printf("performing filtration...");

			// allocate memory for the filtered image data
			filteredImgData = new float[prevWidth * prevHeight] {0.0F};
			if (filteredImgData == nullptr) // check if allocation successful
				return nullptr;

			// perform filtration
			if (doFiltration == Filtration::ExplicitHeatEquation)
			{
				if (!filtrationExplicitHeatEq(chosenImgData, prevWidth, prevHeight, filteredImgData, 3)) // check if filtration successful
				{
					delete[] filteredImgData;
					return nullptr;
				}
			}
			else if (doFiltration == Filtration::GaussianKernel)
			{
				if (!filtrationGauss(chosenImgData, prevWidth, prevHeight, filteredImgData)) // check if filtration successful
				{
					delete[] filteredImgData;
					return nullptr;
				}
			}

			// choose filtered data to use next
			chosenImgData = filteredImgData;
			printf("filtration done\n");

			if (scaledImgData != nullptr)
			{
				delete[] scaledImgData;
			}
		}

		//check if the size of data is suitable for downsampling by 2
		int addRows = 0, addCols = 0;
		if (prevWidth % 2 == 1)
		{
			addCols = 1;
		}
		if (prevHeight % 2 == 1)
		{
			addRows = 1;
		}

		//if mirroring is necessary, call the function
		float* mirroredImgData = nullptr;
		if (!(addRows == 0 && addCols == 0))
		{
			mirroredImgData = mirrorColumnsAndRows(chosenImgData, prevWidth, prevHeight, addCols, addRows);
			chosenImgData = mirroredImgData;

			prevWidth += addCols;
			prevHeight += addRows;

			/*FILE* fp = nullptr;
			fp = fopen("skuskaMirroring.pgm", "w+");

			unsigned char scaledValue = 0;
			size_t dataSize = prevWidth * prevHeight;
			fprintf(fp, "P2\n%d %d\n%d\n", prevWidth, prevHeight, 255);
			for (size_t i = 0; i < dataSize; i++)
			{
				scaledValue = static_cast<unsigned char>(mirroredImgData[i] * 255 + 0.5);
				fprintf(fp, "%d ", scaledValue);

				if ((i + 1) % 70 == 0)
					fprintf(fp, "\n");

				if ((i + 1) % (dataSize / 10) == 0)
					printf("\rExporting image to pgm... %lu%% done", 10 * (i + 1) / (dataSize / 10));
			}
			fclose(fp);*/
		}

		//allocate memory for scaled-down image
		scaledImgWidth = prevWidth / scaleParameter;
		scaledImgHeight = prevHeight / scaleParameter;
		scaledImgData = new float[scaledImgWidth * scaledImgHeight];

		//skip every second row and column
		for (int i = 0; i < scaledImgHeight; i++)
		{
			for (int j = 0; j < scaledImgWidth; j++)
			{
				scaledImgData[i * scaledImgWidth + j] = chosenImgData[scaleParameter * i * prevWidth + scaleParameter * j];
			}
		}

		//assign new data to chosenImgData array for repeating of this process
		chosenImgData = scaledImgData;
		prevWidth = scaledImgWidth;
		prevHeight = scaledImgHeight;

		//deallocate filtered data
		if (filteredImgData != nullptr)
		{
			delete[] filteredImgData;
		}
	}

	//---------------------NORMALIZATION--------------------

	/*//find minimum and maximum of original data to preserve it in scaled data
	std::pair<float, float> minmax;
	minmax = findMinMax(originalImgData, originalImgWidth * originalImgHeight);
	float originalMin = minmax.first;
	float originalMax = minmax.second;

	//find minimum and maximum of scaled data
	minmax = findMinMax(scaledImgData, scaledImgWidth * scaledImgHeight);
	float scaledMin = minmax.first;
	float scaledMax = minmax.second;

	//normalize the scaled data to be in the same range as original data
	normalizeData(scaledImgData, scaledImgWidth * scaledImgHeight, scaledMin, scaledMax, originalMin, originalMax);*/

	//return scaled-down data
	if (scaledImgData != nullptr)
	{
		return scaledImgData;
	}
}

float* ImageProcessing::pixelsMirror(float* originalImgData, const int imgWidth, const int imgHeight, const int padding)
{
	int indexNew = 0, indexOld = 0;
	int temp = 0;

	// compute new size
	int extendedImgWidth = imgWidth + 2 * padding;
	int extendedImgHeight = imgHeight + 2 * padding;
	int size = extendedImgWidth * extendedImgHeight;

	float* extendedImgData = new float[size] {0.0F}; // allocate memory

	if (extendedImgData == nullptr) // check, if allocation was successful
		return nullptr;

	// copy old image data
	for (int i = 0; i < imgHeight; i++)
	{
		for (int j = 0; j < imgWidth; j++)
		{
			indexNew = (i + padding) * extendedImgWidth + (j + padding);
			indexOld = i * imgWidth + j;

			extendedImgData[indexNew] = originalImgData[indexOld];
		}
	}

	// mirror over Upper and Lower edges
	temp = 1;
	for (int i = 0; i < padding; i++)
	{
		for (int j = padding; j < extendedImgWidth - padding; j++)
		{
			// upper egde
			indexOld = (i + padding) * extendedImgWidth + j;
			indexNew = (i + padding - temp) * extendedImgWidth + j;
			extendedImgData[indexNew] = extendedImgData[indexOld];

			// lower edge
			indexOld = (extendedImgHeight - i - padding - 1) * extendedImgWidth + j;
			indexNew = (extendedImgHeight - i - padding + temp - 1) * extendedImgWidth + j;
			extendedImgData[indexNew] = extendedImgData[indexOld];
		}
		temp += 2;
	}

	// mirror over Left and Right edges
	for (int i = 0; i < extendedImgHeight; i++)
	{
		temp = 1;
		for (int j = 0; j < padding; j++)
		{
			// left edge
			indexOld = i * extendedImgWidth + (j + padding);
			indexNew = i * extendedImgWidth + (j + padding - temp);
			extendedImgData[indexNew] = extendedImgData[indexOld];

			// right edge
			indexOld = i * extendedImgWidth + (extendedImgWidth - padding - 1 - j);
			indexNew = i * extendedImgWidth + (extendedImgWidth - padding - 1 - j + temp);
			extendedImgData[indexNew] = extendedImgData[indexOld];

			temp += 2;
		}
	}

	return extendedImgData;
}

bool ImageProcessing::filtrationGauss(float* originalImgData, int originalImgWidth, int originalImgHeight, float* filteredImgData)
{
	if (originalImgData == nullptr || filteredImgData == nullptr) {
		return false;
	}
	// mirrored image
	float* extendedImg = pixelsMirror(originalImgData, originalImgWidth, originalImgHeight, 2);
	if (extendedImg == nullptr) {	// check if mirroring successful
		return false;
	}

	// compute extended size
	int extendedImgHeight = originalImgHeight + 4;
	int extendedImgWidth = originalImgWidth + 4;

	int N = 2;	// number of mirrored pixels

	// values for the matrix with the Gaussian
	double w1 = 0.234237;
	double w2 = 0.112116;
	double w3 = 0.0542418;
	double w4 = 0.01241;
	double w5 = 0.00600398;
	double w6 = 0.000664574;

	// matrix for the Gaussian filtration
	double kernel[25] = { w6,w5,w4,w5,w6,w5,w3,w2,w3,w5,w4,w2,w1,w2,w4,w5,w3,w2,w3,w5,w6,w5,w4,w5,w6 };

	float total = 0.;	// value of the filtered image pixel
	int m = 0;	// index in the matrix

	// iterate over extended image pixels
	for (int i = N; i < extendedImgHeight - N; i++) {
		for (int j = N; j < extendedImgWidth - N; j++) {
			// calculate value in the mask
			for (int k = i - N; k <= i + N; k++) {
				for (int l = j - N; l <= j + N; l++) {
					total = total + extendedImg[k * extendedImgWidth + l] * kernel[m];
					m++;
				}
			}
			//set pixel value
			filteredImgData[(i - N) * originalImgWidth + (j - N)] = total;

			total = 0;	// set value to 0
			m = 0;	// set the matrix index to 0
		}
	}

	delete[] extendedImg;	// deallocate
	return true;
}

bool ImageProcessing::filtrationExplicitHeatEq(float* originalImgData, int originalImgWidth, int originalImgHeight, float* filteredImgData, int iterNumber)
{
	if (originalImgData == nullptr || filteredImgData == nullptr) {
		return false;
	}
	// mirrored image
	float* extendedImg = pixelsMirror(originalImgData, originalImgWidth, originalImgHeight, 1);
	if (extendedImg == nullptr) {	// check if mirroring successful
		return false;
	}

	// compute extended size
	int extendedImgHeight = originalImgHeight + 2;
	int extendedImgWidth = originalImgWidth + 2;

	// set parameters
	int N = 1;	// number of mirrored pixels
	int m = 0;	// index for the filtered pixel
	float tau = 0.15;	// parameter for HCE

	// iterate over number of iterations
	for (int t = 0; t < iterNumber; t++) {
		// compute the pixel value for the filtered image
		for (int i = N; i < extendedImgHeight - N; i++) {
			for (int j = N; j < extendedImgWidth - N; j++) {
				filteredImgData[m] = (1 - 4 * tau) * extendedImg[i * extendedImgWidth + j] + tau * (extendedImg[(i - 1) * extendedImgWidth + j] + extendedImg[(i + 1) * extendedImgWidth + j] + extendedImg[i * extendedImgWidth + j - 1] + extendedImg[i * extendedImgWidth + j + 1]);
				m++;
			}
		}

		delete[] extendedImg;	// deallocate
		//	if neccessary, do the mirroring again
		if (t != iterNumber - 1) {
			extendedImg = pixelsMirror(filteredImgData, originalImgWidth, originalImgHeight, 1);
			// check if mirroring successful
			if (extendedImg == nullptr) {
				return false;
			}
			// set the index of the pixel for the filtered image to 0
			m = 0;
		}
	}

	return true;
}

std::pair<float, float> ImageProcessing::findMinMax(float* data, long dataSize)
{
	float min = FLT_MAX, max = FLT_MIN;

	for (size_t i = 0; i < dataSize; i++)
	{
		//check for maximum
		if (data[i] > max)
		{
			max = data[i];
		}

		//check for minimum
		if (data[i] < min)
		{
			min = data[i];
		}
	}

	return std::pair<float, float>(min, max);
}

void ImageProcessing::normalizeData(float* data, long dataSize, float dataMin, float dataMax, float requestedMin, float requestedMax)
{
	for (size_t i = 0; i < dataSize; i++)
	{
		data[i] = requestedMin + (data[i] - dataMin) / (dataMax - dataMin) * (requestedMax - requestedMin);
	}
}

float* ImageProcessing::mirrorColumnsAndRows(float* originalImgData, const int imgWidth, const int imgHeight, const int paddingColumn, const int paddingRow)
{
	int indexNew = 0, indexOld = 0;

	// compute new size
	int extendedImgWidth = imgWidth + paddingColumn;
	int extendedImgHeight = imgHeight + paddingRow;
	int size = extendedImgWidth * extendedImgHeight;

	float* extendedImgData = new float[size] {0.0F}; // allocate memory

	if (extendedImgData == nullptr) // check, if allocation was successful
		return nullptr;

	// copy old image data
	for (int i = 0; i < imgHeight; i++)
	{
		for (int j = 0; j < imgWidth; j++)
		{
			indexNew = i * extendedImgWidth + j;
			indexOld = i * imgWidth + j;
			extendedImgData[indexNew] = originalImgData[indexOld];
		}
	}

	// mirror over Lower edge
	for (int i = 0; i < paddingRow; i++)
	{
		for (int j = 0; j < extendedImgWidth - paddingColumn; j++)
		{
			indexOld = (imgHeight - i - 1) * extendedImgWidth + j;
			indexNew = (imgHeight + i) * extendedImgWidth + j;
			extendedImgData[indexNew] = extendedImgData[indexOld];
		}
	}

	// mirror over Right edge
	for (int i = 0; i < extendedImgHeight; i++)
	{
		for (int j = 0; j < paddingColumn; j++)
		{
			indexOld = i * extendedImgWidth + (imgWidth - j - 1);
			indexNew = i * extendedImgWidth + (imgWidth + j);
			extendedImgData[indexNew] = extendedImgData[indexOld];
		}
	}

	return extendedImgData;
}