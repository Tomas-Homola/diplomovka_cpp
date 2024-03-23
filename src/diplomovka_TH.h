#pragma once

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <QtWidgets/QMainWindow>

#include "ui_diplomovka_TH.h"

#include "../libs/gdal_include/gdal_priv.h"
#include "../libs/gdal_include/gdal.h"
#include "cpl_string.h"

#include "LASlib/lasreader.hpp"
#include "LASlib/laswriter.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <omp.h>
#include <vector>
#include <fstream>
#include <qfile.h>
#include <cmath>
#include <algorithm>
#include <random>

#define GROUND 2
#define VEG_LOW 3
#define VEG_MEDIUM 4
#define VEG_HIGH 5


struct AreaInfo
{
	int width_n = -1;
	int height_n = -1;
	int width = -1;
	int height = -1;
	double xLeft = -1.0;
	double yLeft = -1.0;
	std::vector<std::string> lazFiles;
};

struct Points
{
	std::vector<double> xCoords;
	std::vector<double> yCoords;
	std::vector<double> zCoords;
	std::vector<int> ptClass;
};

class DataHandler : public QMainWindow
{
	Q_OBJECT

public:
	DataHandler(QWidget* parent = nullptr);
	~DataHandler();

private:
	Ui::NaturaSat_TSPClass ui;

	AreaInfo m_areaInfo;
	std::vector<Points> m_meshPixels;
	std::vector<double*> m_metrics;

	void readInputFile(QTextStream& stream);

	bool readLazFiles();

	void normalizePoints();

	void computeMetrics();

	void exportMetrics(std::string fileName);
};
