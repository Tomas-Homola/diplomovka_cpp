/*****************************************************************//**
 * \file   File.cpp
 * \brief  Generic input/output interface for the file.
 *
 * \author Michal Kollar (michalkollar27@gmail.com)
 * \date   March 2021
 *********************************************************************/
#pragma once
#include "File.h"

IO::File::~File()
{
	free();
}

void IO::File::setWorkingOffsetX(int id, int value)
{
	workingfOffsetX[id] = value;
}

void IO::File::setWorkingOffsetY(int id, int value)
{
	workingfOffsetY[id] = value;
}

void IO::File::free(std::set<int> keepChannelsIds)
{
	for (size_t i = 0; i < channels.size(); i++) {
		if ((keepChannelsIds.find(i) != keepChannelsIds.end()) && (loadedChannelsId.find(i) != loadedChannelsId.end())) {
			continue;
		}
		freeChannel(i);
	}
}

void IO::File::freeChannel(int channelNumber)
{
	auto it = loadedChannelsId.find(channelNumber);
	if (it != loadedChannelsId.end()) {
		loadedChannelsId.erase(it);
	}

	channels[channelNumber].freeDataSets();
}

bool IO::File::dataLoaded(int channelNumber, int datasetNumber)
{
	return channels[channelNumber].getDatasets()[datasetNumber].getData() != nullptr;
}

std::string IO::File::getChannelName(int channelNumber)
{
	return channels[channelNumber].getName();
}

void IO::File::open(std::string name, std::string type, std::string path)
{
	fileName = name;
	fileType = type;
	fileAbsolutePath = path;
}

void IO::DebugFile::saveToPPM(std::string fileName, int width, int height, int maxValue, unsigned char* r, unsigned char* g, unsigned char* b)
{
	FILE* fp;
	fp = fopen((fileName + ".ppm").c_str(), "w+");
	fprintf(fp, "P3\n%d %d\n%d\n", width, height, maxValue);
	for (size_t i = 0; i < width * height; i++)
	{
		fprintf(fp, "%d %d %d\n", static_cast<int>(r[i]), static_cast<int>(g[i]), static_cast<int>(b[i]));
	}
	fclose(fp);
}

void IO::DebugFile::saveToPGM(std::string fileName, int width, int height, int maxValue, float* d)
{
	FILE* fp;
	fp = fopen((fileName.substr(0, 3) + ".pgm").c_str(), "w+");
	fprintf(fp, "P2\n%d %d\n%d\n", width, height, maxValue);
	for (size_t i = 0; i < width * height; i++)
	{
		fprintf(fp, "%d\n", static_cast<int>(d[i] * maxValue + 0.5));
	}
	fclose(fp);
}