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
#include <ctime>

#include "stat_functions.hpp"

#define GROUND 2
#define VEG_LOW 3
#define VEG_MEDIUM 4
#define VEG_HIGH 5

#define Hmax 0
#define Hmean 1
#define Hmedian 2
#define Hp25 3
#define Hp75 4
#define Hp95 5
#define PPR 6
#define DAM_z 7
#define BR_bellow_1 8
#define BR_1_2 9
#define BR_2_3 10
#define BR_above_3 11
#define BR_3_4 12
#define BR_4_5 13
#define BR_bellow_5 14
#define BR_5_20 15
#define BR_above_20 16
#define Coeff_var_z 17
#define Hkurt 18
#define Hskew 19
#define Hstd 20
#define Hvar 21
#define Shannon 22

struct AreaInfo
{
	int width_n = -1;
	int height_n = -1;
	int width = -1;
	int height = -1;
	int desiredPixelSize = -1;
	double xLeft = -1.0;
	double yLeft = -1.0;
	std::vector<std::string> lazFiles;
};

struct VegetationPoints
{
	std::vector<double> xCoords;
	std::vector<double> yCoords;
	std::vector<double> zCoords;
	std::vector<int> ptClass;
};

struct GroundPoints
{
	std::vector<double> xCoords;
	std::vector<double> yCoords;
	std::vector<double> zCoords;
};

struct PixelPoints
{
	VegetationPoints vegetationPts;
	GroundPoints groundPts;
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
	std::vector<PixelPoints> m_meshPixels;
	std::vector<PixelPoints> m_meshPixelsRedistributed;
	std::vector<double*> m_metrics;
	std::string m_areaName = "";

	std::vector<std::string> m_bandNames = { "Hmax","Hmean","Hmedian","Hp25","Hp75","Hp95","PPR","DAM_z","BR_bellow_1","BR_1_2","BR_2_3","BR_above_3","BR_3_4","BR_4_5","BR_bellow_5","BR_5_20","BR_above_20","Coeff_var_z","Hkurt","Hskew","Hstd","Hvar","Shannon" };

	void readInputFile(QTextStream& stream);

	bool readLazFiles();

	void normalizePoints();

	void redistributePoints();

	void computeMetrics();

	void exportMetrics(std::string fileName);

	void performCalculation(QString infoFile);
};
