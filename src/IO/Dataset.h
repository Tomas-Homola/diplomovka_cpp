/*****************************************************************//**
 * \file   Dataset.h
 * \brief  Ggeneric input/output interface for dataset in channel of file.
 *
 * \author Michal Kollar (michalkollar27@gmail.com)
 * \date   March 2021
 *********************************************************************/
#pragma once
#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <set>
#include <map>
#include <filesystem>
#include <sstream>
#include <vector>
#include <array>

#include "../Bootstrap.h"
#include "DataCache.h"

 //! Namespace for interfaces and implementations for input and output
namespace IO {
	//!  Generic class for image dataset
	/*!
		Class for one image dataset from file channel
	*/
	class Dataset {
	public:
		//! Constructor.
		/*!
		 *  Empty constructor
		 */
		Dataset() {};
		//! Constructor.
		/*!
		 * Constructor with parameters
		 * \param path absolute path to dataset
		 */
		Dataset(std::string path) :fullPath(path) {
			std::size_t found = path.find_last_of("/\\");
			fileName = path.substr(found + 1);
		};

		//! Get dataset full path.
		/*!
		 * \return std::string full path
		 */
		std::string getFullPath() { return fullPath; }

		//!Get dataset file name
		/*!
		 * \return std::string file name
		 */
		std::string getFileName() { return fileName; }

		//! Add new data to dataset.
		/*!
		 *
		 * \param newdata dataType* data to fill
		 * \param size size of new data
		 * \param findMinMax if true, function look for minimal and maximal value and set them to rasterMin and rasterMax
		 * \param min minimum to set to rasterMin
		 * \param max maximum to set to rasterMax
		 * \tparam dataType type of new data
		 */
		template<typename dataType> void addToData(dataType* newdata, long size, bool findMinMax = true, double min = 0, double max = 0);

		//! Crop and scale data function.
		/*!
		 * Crop data according to parameters and scale them to [0,1]
		 * \param cropBottom minimum for crop histogramm function
		 * \param cropTop maximum for crop histogram function
		 */
		void crop(double cropBottom, double cropTop);

		//! Deallocate memory for data.
		/*!
		 * Delete data
		 */
		void freeData();

		//! Get data.
		/*!
		 * Get loaded data (if nullptr, data are empty)
		 * \return data
		 */
		float* getData() { return data; }

		//! Clear histogram.
		void clearHistogram() { histogram.clear(); histogram.shrink_to_fit(); }

		float getDataMin() { return dataMin; }
		float getDataMax() { return dataMax; }

		float getCroppedMin() { return croppedMin; }
		float getCroppedMax() { return croppedMax; }

		//-------Data Cache---------

		template<typename dataType>	void addToDataCache(std::string name, dataType* inputData, size_t size);
		void createDataCache(std::string name, size_t size);
		float* getFromDataCache(std::string name);
		size_t getDataChacheSize(std::string name);
		void clearDataCache(std::string name);
		void clearFullDataCache();

		//DataCache*& getDataCache(std::string name);
	private:
		//! Find and set croppedMin, croppedMax.
		/*!
		 * \param cropBottom minimum for crop histogramm function
		 * \param cropTop maximum for crop histogram function
		 */
		void findCroppedMinMax(double cropBottom, double cropTop);

		//! Set histogram.
		/*!
		 * Fill std::vector<int> histogram
		 */
		void setHistogram();

		std::string fileName = "";	/*!< dataset name */
		std::string fullPath = "";	/*!< dataset path (or name) */
		float* data = nullptr;		/*!< data */
		long dataSize = 0;			/*!< size of data */

		float dataMin = 0;			/*!< minimum of data */
		float dataMax = 0;			/*!< maximum of data */

		float croppedMin = 0;		/*!< new minimum after croping of data */
		float croppedMax = 0;		/*!< new maximum after cropping of data */
		std::vector<int> histogram = std::vector<int>(); /*!< histogram for data */

		std::map<std::string, DataCache*> dataCache;
	};

	template<typename dataType>
	inline void Dataset::addToData(dataType* newdata, long size, bool findMinMax, double min, double max)
	{
		dataSize = size;
		data = new float[dataSize];

		float fMin = FLT_MAX, fMax = FLT_MIN;
		for (size_t i = 0; i < dataSize; i++)
		{
			float val = static_cast<float>(newdata[i]);
			if (findMinMax) {
				if (val > fMax) fMax = val;
				if (val < fMin) fMin = val;
			}
			data[i] = val;
		}

		if (findMinMax) {
			dataMin = fMin;
			dataMax = fMax;
		}
		else {
			if (min != 0) {
				dataMin = min;
			}
			if (max != 0) {
				dataMax = max;
			}
		}

		std::cout << fileName + " data loaded, max: " << dataMax << std::endl;
	}

	template<typename dataType>
	inline void Dataset::addToDataCache(std::string name, dataType* inputData, size_t size)
	{
		std::cout << "Data loaded to cache " << name << " for " << fileName << std::endl;
		//key does not exists
		if (dataCache.find(name) == dataCache.end()) {
			dataCache[name] = new DataCache();
		}
		//key exists
		dataCache[name]->setData<dataType>(inputData, size);
	}
}
