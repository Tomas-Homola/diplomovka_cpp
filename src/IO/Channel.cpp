/*****************************************************************//**
 * \file   Channel.cpp
 * \brief  Generic input/output interface for channel in file.
 *
 * \author Michal Kollar (michalkollar27@gmail.com)
 * \date   March 2021
 *********************************************************************/
#include "Channel.h"

void IO::Channel::freeDataSets()
{
	for (size_t j = 0; j < datasets.size(); j++)
	{
		freeDataSet(j);
	}
}

void IO::Channel::freeDataSet(int dataSetNumber)
{
	datasets[dataSetNumber].freeData();
	datasets[dataSetNumber].clearFullDataCache();
}