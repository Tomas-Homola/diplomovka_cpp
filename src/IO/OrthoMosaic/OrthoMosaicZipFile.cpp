/*****************************************************************//**
 * \file   OrthoMosaicZipFile.cpp
 * \brief  Source codes for Orthomosaic zip file interface
 *
*********************************************************************/
#include "OrthoMosaicZipFile.h"

IO::OrthoMosaicZipFile::OrthoMosaicZipFile()
{

}

void IO::OrthoMosaicZipFile::open(std::string name, std::string type, std::string path)
{
	//File::open(name, type, path); // save file name, type and full path
	this->fileName = name; // save selected file name
	this->fileType = type; // save selected file type

	std::string zipPreffix = std::string("/vsizip/");
	std::string zipPath	   = std::string(zipPreffix + path + "/" + name + "." + type);

	this->fileAbsolutePath = zipPath; // save absolute file path

	GDALAllRegister();
	GDALDataset* poDataset = static_cast<GDALDataset*>(GDALOpen(zipPath.c_str(), GA_ReadOnly));

	if (poDataset == nullptr)
		throw File::error::unableToOpen;

	this->dataWidth	 = poDataset->GetRasterXSize(); // number of pixels in x direction (image width)
	this->dataHeight = poDataset->GetRasterYSize(); // number of pixels in y direction (image height)
	this->dataSize = this->dataWidth * this->dataHeight * poDataset->GetRasterCount(); // size of tif image * number of bands = total data size

	// get geo transform data
	double adGeoTransform[6] = { 0.0, 0.0, 0.0, 0.0, 0.0,0.0 };
	if (poDataset->GetGeoTransform(adGeoTransform) != CE_None)
		throw File::error::missingMetaData;

	// save geo transform parameters
	this->upperLeftCoordinate_X = adGeoTransform[0];
	this->upperLeftCoordinate_Y = adGeoTransform[3];
	this->pixelWorldSize_X		= adGeoTransform[1];
	this->pixelWorldSize_Y		= adGeoTransform[5];
	this->rotationAboutAxis_X	= adGeoTransform[2];
	this->rotationAboutAxis_Y   = adGeoTransform[4];

	// save projections reference
	this->projectionReferenceWKT = std::string(poDataset->GetProjectionRef());

	int channelsCount = poDataset->GetRasterCount(); // get number of channels
	this->channels.resize(channelsCount); // resize vector for channels accordingly

	// initial setting of channels
	int channelID = 1;
	std::string channelName = "", datasetFullPath = zipPath;
	GDALRasterBand* rasterBand = nullptr;
	for (Channel& chanel : this->channels)
	{
		rasterBand = poDataset->GetRasterBand(channelID);
		channelName = std::string(GDALGetColorInterpretationName(rasterBand->GetColorInterpretation()));
		chanel = Channel(channelName); // create new object of class Channel with the given name
		
		std::vector<Dataset>& datasets = chanel.getDatasets(); // get vector of Dataset from current channel
		datasets.resize(1); // each channel will have only 1 dataset
		datasets[0] = Dataset(datasetFullPath); // create new object of class Dataset with full path to the file
		GDALDataType datatype = rasterBand->GetRasterDataType();

		channelID++;
	}

	GDALClose(poDataset);
}

void IO::OrthoMosaicZipFile::fillDataset(int channelNumber, int datasetNumber, double cropBottom, double cropTop, bool original)
{
	Dataset& refSelectedDataset = this->channels[channelNumber].getDatasets()[datasetNumber];

	GDALDataset* poDataset = nullptr;
	GDALAllRegister();
	
	// open file for the selected dataset
	poDataset = static_cast<GDALDataset*>(GDALOpen(refSelectedDataset.getFullPath().c_str(), GA_ReadOnly));
	if (poDataset == nullptr)
		throw File::error::unableTofillData;

	GDALRasterBand* rasterBand = nullptr;
	rasterBand = poDataset->GetRasterBand(channelNumber + 1);
	if (rasterBand == nullptr)
		throw File::error::unableTofillData;

	long bufferSize_X = rasterBand->GetXSize();
	long bufferSize_Y = rasterBand->GetYSize();
	long dataSize = bufferSize_X * bufferSize_Y;
	long requestSize_X = bufferSize_X;
	long requestSize_Y = bufferSize_Y;
	GDALDataType rasterDataType = rasterBand->GetRasterDataType();

	if (rasterDataType == GDT_Byte)
	{
		float* dataBuffer = nullptr;
		// allocate memory for band data
		dataBuffer = static_cast<float*>(CPLMalloc(dataSize * sizeof(float)));
		if (dataBuffer == nullptr)
			throw File::error::unableTofillData;

		// read data from the selected band ...
		bool dataSaved = read<float*>(dataBuffer, 0, 0, requestSize_X, requestSize_Y, bufferSize_X, bufferSize_Y, rasterBand, GDT_Float32);

		//for (int i = 0; i < 10; i++)
		//{
		//	printf("%.2f\n", dataBuffer[i]);
		//}

		if (!dataSaved) // ... if not successful
		{
			// release allocated memory
			CPLFree(dataBuffer);
			throw File::error::unableTofillData;
		}

		// ... if successful, copy data do dataset ...
		refSelectedDataset.addToData<float>(dataBuffer, dataSize, false, 0.0, 255.0);
		CPLFree(dataBuffer); // ... and release data buffer from memory
	}
	else
	{
		throw File::error::unsupportedFormat;
	}

	GDALClose(poDataset);

	// scale loaded data to [0,1] interval
	if (!original) {
		refSelectedDataset.crop(cropBottom, cropTop);
	}

	//IO::DebugFile::saveToPGM(std::string("../../testExport"), this->dataWidth, this->dataHeight, 255, refSelectedDataset.getData());
}

IO::OrthoMosaicZipFile::~OrthoMosaicZipFile()
{

}

bool IO::OrthoMosaicZipFile::ExportToPGM(std::string fileName, int width, int height, int maxValue, float* d)
{
	printf("Exporting image to pgm...\n");
	FILE* fp = nullptr;
	fp = fopen((fileName + ".pgm").c_str(), "w+");
	if (fp == nullptr)
		return false;

	unsigned char scaledValue = 0;
	size_t dataSize = width * height;
	fprintf(fp, "P2\n%d %d\n%d\n", width, height, maxValue);
	for (size_t i = 0; i < dataSize; i++)
	{
		scaledValue = static_cast<unsigned char>(d[i] * maxValue + 0.5);
		fprintf(fp, "%d ", scaledValue);

		if ((i + 1) % 70 == 0)
			fprintf(fp, "\n");

		if ((i + 1) % (dataSize / 10) == 0)
			printf("\rExporting image to pgm... %lu%% done", 10 * (i + 1) / (dataSize / 10));
	}
	fclose(fp);

	return true;
}

bool IO::OrthoMosaicZipFile::ExportToPPM(std::string fileName, int width, int height, int maxValue, float* r, float* g, float* b)
{
	printf("Exporting image to ppm...\n");
	FILE* fp = nullptr;
	fp = fopen((fileName + ".ppm").c_str(), "w+");
	if (fp == nullptr)
		return false;

	unsigned char scaledR = 0, scaledG = 0, scaledB = 0;
	size_t dataSize = width * height;
	fprintf(fp, "P3\n%d %d\n%d\n", width, height, maxValue);
	for (size_t i = 0; i < dataSize; i++)
	{
		scaledR = static_cast<unsigned char>(r[i] * maxValue + 0.5);
		scaledG = static_cast<unsigned char>(g[i] * maxValue + 0.5);
		scaledB = static_cast<unsigned char>(b[i] * maxValue + 0.5);

		fprintf(fp, "%d %d %d\t", scaledR, scaledG, scaledB);

		if ((i + 1) % 70 == 0)
			fprintf(fp, "\n");

		if ((i + 1) % (dataSize / 10) == 0)
			printf("\rExporting image to ppm... %d%% done", 10 * (i + 1) / (dataSize / 10));
	}
	fclose(fp);

	return true;
}

//template<typename bufferDataType>
//bool IO::OrthoMosaicZipFile::read(bufferDataType outputData, long requestOffsetX, long requestOffsetY, long requestSizeX, long requestSizeY, long bufferSizeX, long bufferSizeY, GDALRasterBand* band, GDALDataType dataType)
//{
//	//nPixelSpace: The byte offset from the start of one pixel value in pData to the start of the next pixel value within a scanline.If defaulted(0) the size of the datatype eBufType is used.
//	int nPixelSpace = 0;
//
//	//nLineSpace : The byte offset from the start of one scanline in pData to the start of the next. If defaulted(0) the size of the datatype eBufType* nBufXSize is used.
//	int nLineSpace = 0;
//
//	if (band->RasterIO(GF_Read, requestOffsetX, requestOffsetY, requestSizeX, requestSizeY, outputData, bufferSizeX, bufferSizeY, dataType, nPixelSpace, nLineSpace) != CE_None) {
//		return false;
//	}
//	return true;
//}