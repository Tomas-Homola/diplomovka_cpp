/*****************************************************************//**
 * \file   OrthoMosaicZipFile.h
 * \brief  Header for Orthomosaic zip file interface
 *
 * \author Tomas Homola
 * \date   ...
 *********************************************************************/
#pragma once
#define GEOGRAPHICLIB_SHARED_LIB 1

#include "../File.h"

#include "gdal_priv.h"
#include "cpl_conv.h"
#include <GeographicLib/GeoCoords.hpp>

 //! Namespace for interfaces and implementations for input and output
namespace IO
{
	//!  Orthomosaic zip file class
	/*!
		Class for managing the input Orthomosaic .zip file
	*/
	class OrthoMosaicZipFile : public File
	{
	public:
		
		enum ChannelsBinaryInfo : int
		{
			Red_1   = 19, // Beggining of RED channel of the original image in .bin file.
			Green_1 = 125000019, // Beggining of GREEN channel of the original image in .bin file.
			Blue_1  = 250000019, // Beggining of BLUE channel of the original image in .bin file.
			Red_2   = 375000019, // Beggining of RED channel of the image scaled down by 2.
			Green_2 = 406250019, // Beggining of GREEN channel of the image scaled down by 2.
			Blue_2  = 437500019, // Beggining of BLUE channel of the image scaled down by 2.
			Red_4   = 468750019, // Beggining of RED channel of the image scaled down by 4.
			Green_4 = 476562519, // Beggining of GREEN channel of the image scaled down by 4.
			Blue_4  = 484375019, // Beggining of BLUE channel of the image scaled down by 4.
			Red_8   = 492187519, // Beggining of RED channel of the image scaled down by 8.
			Green_8 = 494141269, // Beggining of GREEN channel of the image scaled down by 8.
			Blue_8  = 496095019, // Beggining of BLUE channel of the image scaled down by 8.

			ImgSize_1 = 125000000, // Size of the original image (how many bytes to read).
			ImgSize_2 = 31250000, // Size of the image scaled down by 2 (how many bytes to read). 
			ImgSize_4 = 7812500, // Size of the image scaled down by 4 (how many bytes to read).
			ImgSize_8 = 1953750, // Size of the image scaled down by 8 (how many bytes to read).

			ImgWidth_1  = 12500, // Width  of the original image (in pixels).
			ImgHeight_1 = 10000, // Height of the original image (in pixels).
			ImgWidth_2  =  6250, // Width  of the image scaled down by 2 (in pixels).
			ImgHeight_2 =  5000, // Height of the image scaled down by 2 (in pixels).
			ImgWidth_4  =  3125, // Width  of the image scaled down by 4 (in pixels).
			ImgHeight_4 =  2500, // Height of the image scaled down by 4 (in pixels).
			ImgWidth_8  =  1563, // Width  of the image scaled down by 8 (in pixels).
			ImgHeight_8 =  1250 // Height of the image scaled down by 8 (in pixels).

			// mozno pridat aj pixel size
		};
		
		
		//! Constructor
		/*!
		 *	Empty constructor for OrthoMosaicZipFile class.
		 *  
		 */
		OrthoMosaicZipFile();

		//!  Open file function for Orthomosaic .zip data type.
		/*!
		 *
		 * \param name base name of the file
		 * \param type extension of the file
		 * \param path full absolute path to file
		 * \return true, if the file is successfully opened
		 * \see IO::File::open
		 */
		void open(std::string name, std::string type, std::string path);

		//!  Read function for Orthomosaic .zip data type.
		/*!
		 *	Function fill full data to the selected dataset in the selected channel from the channels vector
		 * \param channelNumber id of channel to read
		 * \param datasetNumber	id of dataset to read
		 * \param cropBottom	minimum for crop histogram function
		 * \param cropTop		maximum for crop histogram function
		 * \param original		if true, data are not scaled to [0,1] interval or cropped
		 */
		void fillDataset(int channelNumber, int datasetNumber, double cropBottom, double cropTop, bool original);

		//! Destructor
		/*!
		 *	Destructor for OrthoMosaicZipFile class.
		 */
		~OrthoMosaicZipFile();

		//-----------------Get methods--------------------

		// Get x coordinate (easting) of the upper left corner of the orthomosaic [meters]
		double getUpperLeftCoordinate_X() { return upperLeftCoordinate_X; }

		// Get y coordinate (northing) of the upper left corner of the orthomosaic [meters]
		double getUpperLeftCoordinate_Y() { return upperLeftCoordinate_Y; }

		// Get real world size of 1 pixel in x direction
		double getPixelWorldSize_X() { return pixelWorldSize_X; }

		// Get real world size of 1 pixel in y direction
		double getPixelWorldSize_Y() { return pixelWorldSize_Y; }

		// Return absolute value of pixel size in real world size in x direction
		double pixelSize_X() { return ((pixelWorldSize_X < 0) ? -pixelWorldSize_X : pixelWorldSize_X); }

		// Return absolute value of pixel size in real world size in y direction
		double pixelSize_Y() { return ((pixelWorldSize_Y < 0) ? -pixelWorldSize_Y : pixelWorldSize_Y); }

		// Get rotation about x axis
		double getRotationAboutAxis_X() { return rotationAboutAxis_X; }

		// Get rotation about y axis
		double getRotationAboutAxis_Y() { return rotationAboutAxis_Y; }

		// Get projection description in WKT format
		std::string getProjectionReferenceWKT() { return projectionReferenceWKT; }


		// EXPORT FNCTIONS
		static bool ExportToPGM(std::string fileName, int width, int height, int maxValue, float* d);

		static bool ExportToPPM(std::string fileName, int width, int height, int maxValue, float* r, float* g, float* b);
	protected:
		double upperLeftCoordinate_X = 0.0; // X coordinate of the upper left point
		double upperLeftCoordinate_Y = 0.0; // Y coordinate of the upper left point

		double pixelWorldSize_X = 0.0; // Pixel size in the x-direction in map units
		double pixelWorldSize_Y = 0.0; // Pixel size in the y-direction in map units

		double rotationAboutAxis_X = 0.0; // Rotation about x-axis
		double rotationAboutAxis_Y = 0.0; // Rotation about y-axis

		std::string projectionReferenceWKT = ""; // File projection reference

		template <typename bufferDataType>
		bool read(bufferDataType outputData, long requestOffsetX, long requestOffsetY, long requestSizeX, long requestSizeY, long bufferSizeX, long bufferSizeY, GDALRasterBand* band, GDALDataType dataType);
	};


	template<typename bufferDataType>
	inline bool OrthoMosaicZipFile::read(bufferDataType outputData, long requestOffsetX, long requestOffsetY, long requestSizeX, long requestSizeY, long bufferSizeX, long bufferSizeY, GDALRasterBand* band, GDALDataType dataType)
	{
		//nPixelSpace: The byte offset from the start of one pixel value in pData to the start of the next pixel value within a scanline.If defaulted(0) the size of the datatype eBufType is used.
		int nPixelSpace = 0;

		//nLineSpace : The byte offset from the start of one scanline in pData to the start of the next. If defaulted(0) the size of the datatype eBufType* nBufXSize is used.
		int nLineSpace = 0;

		if (band->RasterIO(GF_Read, requestOffsetX, requestOffsetY, requestSizeX, requestSizeY, outputData, bufferSizeX, bufferSizeY, dataType, nPixelSpace, nLineSpace) != CE_None)
		{
			return false;
		}

		return true;
	}
}