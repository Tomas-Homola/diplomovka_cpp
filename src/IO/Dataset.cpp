/*****************************************************************//**
 * \file   Dataset.cpp
 * \brief  Ggeneric input/output interface for dataset in channel of file.
 *
 * \author Michal Kollar (michalkollar27@gmail.com)
 * \date   March 2021
 *********************************************************************/
#include "Dataset.h"

void IO::Dataset::crop(double cropBottom, double cropTop)
{
	bool crop = false;

	if (cropBottom != 0 || cropTop != 1) {
		crop = true;
	}

	float cMin = dataMin;
	float cMax = dataMax;

	if (crop) {
		findCroppedMinMax(cropBottom, cropTop);
		cMin = croppedMin;
		cMax = croppedMax;
	}

	float cdiff = cMax - cMin;
	if (cdiff == 0) {
		for (size_t i = 0; i < dataSize; i++) {
			data[i] = 1;
		}
	}
	else {
		//#pragma omp parallel for firstprivate (cdiff, cMin)
		for (long long i = 0; i < dataSize; i++)
		{
			float croppedVal = (data[i] - cMin) / cdiff;
			if (croppedVal > 1) croppedVal = 1;
			if (croppedVal < 0) croppedVal = 0;
			data[i] = croppedVal;
		}
	}
}

void IO::Dataset::freeData()
{
	if (data != nullptr) {
		std::cout << fileName + " data deleted" << std::endl;
		delete[] data;
		data = nullptr;
	}
}

void IO::Dataset::findCroppedMinMax(double cropBottom, double cropTop)
{
	if (histogram.size() == 0) {
		setHistogram();
	}

	croppedMin = dataMin;
	croppedMax = dataMax;

	if (cropBottom != 0) {
		int cropBottomPosition = static_cast<int>(dataSize * cropBottom);
		int counterBottom = 0;

		for (int i = 0; i < dataSize; i++)
		{
			counterBottom += histogram[i];
			if (counterBottom >= cropBottomPosition) {
				croppedMin = i;
				break;
			}
		}
	}

	if (cropTop != 1) {
		int cropTopPosition = static_cast<int>(dataSize * cropTop);
		int counterTop = 0;

		for (int i = 0; i < dataSize; i++)
		{
			counterTop += histogram[i];
			if (counterTop >= cropTopPosition) {
				croppedMax = i;
				break;
			}
		}
	}
}

void IO::Dataset::setHistogram()
{
	histogram.resize(dataMax + 1);
	for (long i = 0; i < dataSize; i++) {
		const float& d = data[i];
		histogram[d]++;
	}
}

//----------------Data cache--------------

void IO::Dataset::createDataCache(std::string name, size_t size)
{
	std::cout << "Cache crated: " << name << " for " << fileName << std::endl;
	//key does not exists
	if (dataCache.find(name) == dataCache.end()) {
		dataCache[name] = new DataCache();
	}
	//key exists
	dataCache[name]->createData(size);
}

float* IO::Dataset::getFromDataCache(std::string name)
{
	auto d = dataCache.find(name);
	if (d != dataCache.end()) {
		return d->second->getData();
	}
	return nullptr;
}

size_t IO::Dataset::getDataChacheSize(std::string name)
{
	auto d = dataCache.find(name);
	if (d != dataCache.end()) {
		return d->second->getSize();
	}
	return 0;
}

void IO::Dataset::clearDataCache(std::string name)
{
	auto d = dataCache.find(name);
	if (d != dataCache.end()) {
		std::cout << name << "dataCache clear";
		delete d->second;
		dataCache.erase(d);
	}
}

void IO::Dataset::clearFullDataCache()
{
	if (dataCache.size()) {
		std::cout << "Full dataCache clear" << " for " << fileName << std::endl;

		for (auto it = dataCache.begin(); it != dataCache.end(); it++) {
			std::cout << "Delete cache:" << it->first << " for " << fileName << " ";
			delete it->second;
		}

		dataCache.clear();
	}
}