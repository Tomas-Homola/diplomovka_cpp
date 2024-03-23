/*****************************************************************//**
 * \file   Channel.h
 * \brief  Generic input/output interface for channel in file.
 *
 * \author Michal Kollar (michalkollar27@gmail.com)
 * \date   March 2021
 *********************************************************************/
#pragma once
#include "Dataset.h"

 //! Namespace for interfaces and implementations for input and output
namespace IO {
	//!  Generic class for image channel
	/*!
		Class for one image channel from file
	*/
	class Channel {
	public:
		//! Constructor .
		/*!
		 *	Empty constructor
		 */
		Channel() {};
		//! Constructor.
		/*!
		 * Constructor with parameters
		 * \param chName Channel name
		 */
		Channel(std::string chName) :name(chName) {};

		//! Get the channel name.
		/*!
		 * \return std::string name
		 */
		std::string getName() { return name; }

		//! Set the channel name.
		/*!
		 * \param chName Channel name
		 */
		void setName(std::string chName) { name = chName; }

		//! Get reference to the vector of data in the channel.
		/*!
		 * \return reference to the vector of data in the channel.
		 */
		std::vector<Dataset>& getDatasets() { return datasets; }

		//! Get size of the vector of data in channel.
		/*!
		 * \return int size of the vector of data in channel.
		 */
		int getDatasetsCount() { return datasets.size(); }

		//!  Deallocate memory for all datasets.
		/*!
		 * Delete data in all datasets
		 */
		void freeDataSets();

		//! Deallocate memory for dataset.
		/*!
		 * Delete data in dataset
		 * \param dataSetNumber id of datataset to deallocate
		 */
		void freeDataSet(int dataSetNumber);

		//! Set Reresolution ratio.
		/*!
		 *
		 * \param r input ratio
		 */
		void setResolutionRatio(double r) { resolutionRatio = r; }

		//! Get Resolution ratio.
		/*!
		 *
		 * \return ration resolution/target resolution
		 */
		double getResolutionRatio() { return resolutionRatio; }
	private:
		std::vector<Dataset> datasets = std::vector<Dataset>();		/*!< vector of datasets in the channel */
		std::string name = "";										/*!< channel name */
		double resolutionRatio = 1;										/*!< ration resolution/target resolution */
	};
}
