/*****************************************************************//**
 * \file   DataCache.h
 * \brief
 *
 * \author Michal Kollar (michalkollar27@gmail.com)
 * \date   October 2021
 *********************************************************************/
#pragma once
#include <iostream>

namespace IO {
	class DataCache
	{
	public:
		DataCache();
		~DataCache();

		template<typename Tin>
		void setData(Tin* intput, size_t s);
		void freeData();
		float* getData() { return data; };
		size_t getSize() { return size; };
		void createData(size_t s);
	protected:
		size_t size = 0;
		float* data = nullptr;
	};

	template<typename Tin>
	void DataCache::setData(Tin* intput, size_t s)
	{
		createData(s);
		for (size_t i = 0; i < size; i++)
		{
			data[i] = static_cast<float>(intput[i]);
		}
	}

}
