/*****************************************************************//**
 * \file   Bootstrap.h
 * \brief
 *
 *
 * \author Michal Kollar (michalkollar27@gmail.com)
 * \date   October 2021
 *********************************************************************/
#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cctype>
#include <algorithm>
#include <fstream>

namespace Bootstrap {
	void split(std::vector<std::string>& lineParts, const std::string& line, char delimiter);
	void trim(std::string& line);
	void pgmImage(std::string fileName, float* data, int imgWidth, int imgHeight, int min = 0, int max = 1);
}
