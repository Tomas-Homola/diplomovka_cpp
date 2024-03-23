/*****************************************************************//**
 * \file   DataCache.cpp
 * \brief
 *
 * \author Michal Kollar (michalkollar27@gmail.com)
 * \date   October 2021
 *********************************************************************/
#include "DataCache.h"

IO::DataCache::DataCache()
{
}

IO::DataCache::~DataCache()
{
	if (data != nullptr) {
		std::cout << "DataCache destructor" << std::endl;
		freeData();
	}
}

void IO::DataCache::freeData()
{
	delete[] data;
	data = nullptr;
	size = 0;
}

void IO::DataCache::createData(size_t s)
{
	if (size != s) {
		if (data != nullptr) {
			delete[] data;
		}
		data = new float[s];
		size = s;
	}
}