/*****************************************************************//**
 * \file   Bootstrap.cpp
 * \brief
 *
 *
 * \author Michal Kollar (michalkollar27@gmail.com)
 * \date   October 2021
 *********************************************************************/
#include "Bootstrap.h"

void Bootstrap::split(std::vector<std::string>& lineParts, const std::string& line, char delimiter)
{
	std::istringstream stringStream(line);
	std::string part;
	while (std::getline(stringStream, part, delimiter))
	{
		lineParts.push_back(part);
	}
}

void Bootstrap::trim(std::string& line)
{
	line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
}

void Bootstrap::pgmImage(std::string fileName, float* data, int imgWidth, int imgHeight, int min, int max)
{
	std::ofstream file;
	file.open(fileName + ".pgm");

	float diff = max - min;

	file << "P2 \n" << imgWidth << " " << imgHeight << "\n" << "255\n";
	int val = 0;

	for (int y = 0; y < imgHeight; y++) {
		int index = imgWidth * y;
		for (int x = 0; x < imgWidth; x++) {
			if (min != 0 || max != 1) {
				val = static_cast<int>(255 * ((data[index + x] - min) / diff) + 0.5);
			}
			else {
				val = static_cast<int>(255 * data[index + x] + 0.5);
			}
			file << val << "\n";
		}
	}
	file.close();
}