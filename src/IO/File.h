/*****************************************************************//**
 * \file   File.h
 * \brief  Generic input/output interface for the file.
 *
 * \author Michal Kollar (michalkollar27@gmail.com)
 * \date   March 2021
 *********************************************************************/
#pragma once
#include "Channel.h"

 //! Namespace for interfaces and implementations for input and output
namespace IO {
	//!  Class for debugging
	class DebugFile {
	public:
		static void saveToPPM(std::string fileName, int width, int height, int maxValue, unsigned char* r, unsigned char* g, unsigned char* b);
		static void saveToPGM(std::string fileName, int width, int height, int maxValue, float* d);
	};

	//!  Generic image file class
	/*!
		Parent class for all file types proving multiple virtual functions for classes that inherit this class
	*/
	class File {
	public:
		//! General file error codes enum
		/*! Enum of all possible exception code thrown by IO interface function */
		enum class error
		{
			ok = 0,
			unsupportedFormat = 1,		/*!< Enum value for unsupported format error. */
			corruptedFile = 2,			/*!< Enum value for corrupted file error. */
			unableToOpen = 3,			/*!< Enum value for unable to open error. */
			missingMetaData = 4,		/*!< Enum value for missing meta data error. */
			corruptedMetaData = 5,		/*!< Enum value for corrupted meta data error. */
			unableTofillData = 6
		};

		//! Destructor.
		/*!
		 *  Deallocate all data
		 */
		~File();

		//! Get the file name.
		/*!
		 * \return std::string file name
		 */
		std::string getFileName() { return fileName; };

		//! Get the file type.
		/*!
		 * \return std::string file type (extension)
		 */
		std::string getFileType() { return fileType; };

		//! Get the file type name.
		/*!
		 * \return std::string name of the file type
		 */
		std::string getFileTypeName() { return fileTypeName; };

		//! Set the file type name.
		/*!
		 * \param ftName name of the file type
		 */
		void setFileTypeName(std::string ftName) { fileTypeName = ftName; };

		//! Get data width.
		/*!
		 * \return data width
		 */
		int getWidth() { return dataWidth; }

		//! Get data height.
		/*!
		 * \return data height
		 */
		int getHeight() { return dataHeight; }

		//!
		void setWorkingSize(int width, int height) { workingWidth = width; workingHeight = height; };

		//! Get working width.
		/*!
		 * \return data width
		 */
		int getWorkingWidth() { return workingWidth; }

		//! Get data height.
		/*!
		 * \return data height
		 */
		int getWorkingHeight() { return workingHeight; }

		//! .
		/*!
		 *
		 * \param id
		 * \param value
		 */
		void setWorkingOffsetX(int id, int value);

		//! .
		/*!
		 *
		 * \param id
		 * \param value
		 */
		void setWorkingOffsetY(int id, int value);

		//!
		int getWorkingOffsetX(int id) { return workingfOffsetX.at(id); };

		//!
		int getWorkingOffsetY(int id) { return workingfOffsetY.at(id); };

		//! Get data size.
		/*!
		 * \return long width*height
		 */
		long getSize() { return dataSize; }

		//! Get Preview img data.
		/*!
		 *
		 * \return pviData
		 */
		std::vector<std::vector<unsigned char>>& getPVIData() { return pviData; }

		//! Get Preview img width.
		/*!
		 *
		 * \return pviwidth
		 */
		int getPviWidth() { return pviWidth; }

		//! Get Preview img height.
		/*!
		 *
		 * \return pviheight
		 */
		int getPviHeight() { return pviHeight; }

		//! Free data in channels.
		/*!
		 * Deallocate data in all channels except listed one in param.
		 * \param keepChannelsIds channels to keep
		 */
		void free(std::set<int> keepChannelsIds = std::set<int>());

		//! Free data in channel.
		/*!
		 *
		 * \param channelNumber id of channel to free
		 */
		void freeChannel(int channelNumber);

		//! Get file channels.
		/*!
		 *	Get reference to the vector of all file channels.
		 * \return reference to vector of channels
		 */
		std::vector<Channel>& getChannels() { return channels; }

		//! .
		/*!
		 *
		 * \param channelNumber
		 * \param datasetNumer
		 * \return
		 */
		Dataset& getDataSet(int channelNumber, int datasetNumer) { return channels[channelNumber].getDatasets()[datasetNumer]; }

		//! .
		/*!
		 *
		 * \param channelNumber
		 * \return
		 */
		Channel& getChannel(int channelNumber) { return channels[channelNumber]; }

		//! Are data in selected channel and loaded.
		/*!
		 * \param channelNumber channel id
		 * \param datasetNumber dataset id
		 * \return true, if data are loaded
		 */
		bool dataLoaded(int channelNumber, int datasetNumber);

		//! Get channel name.
		/*!
		 * \param channelNumber channel id
		 * \return std::string of channel name
		 */
		std::string getChannelName(int channelNumber);

		//! Generic open file function for all data types.
		/*!
		 *
		 * \param name base name of the file
		 * \param type extension of the file
		 * \param path full absolute path to file
		 * \see IO::JP2ZipFile::open
		 */
		virtual void open(std::string name, std::string type, std::string path);

		//! Generic fill dataset function for all data types.
		/*!
		 *
		 * \param channelNumber id of channel to read
		 * \param datasetNumber	id of dataset to read
		 * \param cropBottom	minimum for crop histogramm function
		 * \param cropTop		maximum for crop histogram function
		 * \param original		if true, data are not scaled to [0,1] interval or cropped
		 */
		virtual void fillDataset(int channelNumber, int datasetNumber, double cropBottom = 0, double cropTop = 1, bool original = false) {};

		//! Set preview map from image data.
		/*!
		 *
		 * \param pviW preview map width
		 * \param pviH preview map height
		 * \see IO::JP2ZipFile::setPreviewMap
		 */
		virtual void setPreviewMap(int pviW = 0, int pviH = 0) {};

		//! Load curve
		/*!
		 *
		 * \param filename path to curve
		 * \param suffix suffix of curve file
		 * \param curveSet vector of multiple curves in curveSet consist of vector of coords of curve
		 * \param curveSetNames vector of names of curves from curveSet
		 */
		virtual void loadCurveFromFile(std::string filename, std::string suffix, std::vector<std::vector<std::array<double, 2>>>& curveSet, std::vector<std::string>& curveSetNames) {  };

		//! Save curve
		/*!
		 *
		 * \param filename path to curve
		 * \param suffix suffix of curve file
		 * \param curveSet vector of multiple curves in curveSet consist of vector of coords of curve
		 * \param curveSetNames vector of names of curves from curveSet
		 */
		virtual void saveCurveToFile(std::string filename, std::string suffix, std::vector<std::vector<std::array<double, 2>>>& curveSet, std::vector<std::string>& curveSetNames, std::vector<std::string>& habitat, std::vector<std::string>& segmType, std::vector<std::vector<double>>& segmParam) {  };
		
		//!
		virtual bool geoRefCompareWithFile(File* file) { return false; };

		//!
		virtual void getDataInsideArea(int channelNumber, int datasetNumber,
			int areaOffsetX, int areaOffsetY, int areaWidth, int areaHeight, int outputDataWidth, int outputDataHeight, std::vector<double>& outputData) {};

	protected:
		std::string fileName = std::string("");					/*!< file name */
		std::string fileType = std::string("");					/*!< file type (extesion) */
		std::string fileTypeName = std::string("");				/*!< name of file type */
		std::string fileAbsolutePath = std::string("");			/*!< file absolute path */
		int dataWidth = 0;										/*!< width of file data */
		int dataHeight = 0;										/*!< height of file data */
		long dataSize = 0;										/*!< size of file data */
		std::vector<Channel> channels = std::vector<Channel>();	/*!< vector of all image channels in file */
		std::set<int> loadedChannelsId = std::set<int>();		/*!< set of ids of channels with loaded data */
		std::vector<std::vector<unsigned char>> pviData = std::vector<std::vector<unsigned char>>(); /*!< vector of channels of the image in the preview map */
		int pviWidth = 343;											/*!< default width for the image for the preview map */
		int pviHeight = 343;										/*!< default height for the image for the preview map */
		int workingWidth = 0;										/*!< */
		int workingHeight = 0;										/*!< */
		std::vector<int> workingfOffsetX = { 0, 0 };				/*!< */
		std::vector<int> workingfOffsetY = { 0, 0 }; 				/*!< */
	};
}
