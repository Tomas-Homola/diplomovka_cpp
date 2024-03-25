#include "diplomovka_TH.h"


DataHandler::DataHandler(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	int nprocs = 6;
	omp_set_num_threads(nprocs);

	QString infoFile = "E:/_school/_diplomovka/c++/91F0_VysokaPriMorave.kml_data - Copy.txt";
	
	//std::vector<double> v = { 1.0,2.0,3.0,6.0,6.0,7.0,11.0,12.0,15.0,16.0,19.0,19.0,20.0,21.0,23.0,27.0,30.0,34.0,36.0,40.0 };
	//std::vector<double> v2 = { 0.0 };

	//std::cout << "v ->  Bellow 5: " << (double)StatFunctions::pointsBellow(v, 5.0) / v.size() << "\n";
	//std::cout << "v2 -> Bellow 5: " << (double)StatFunctions::pointsBellow(v2, 5.0) / v2.size() << "\n";
	
	//int t = 0;
	//std::cout << "0.0 / NAN = " << 0.0 / NAN << "\n";
	//std::cout << "0.0 / 0 = " << 0.0 / t << "\n";

	performCalculation(infoFile);

	exit(0);
}

DataHandler::~DataHandler()
{

}

void DataHandler::readInputFile(QTextStream& stream)
{
	QString temp = "";
	QStringList parsed;
	size_t numOfLazFiles = -1;

	// read dimensions for normalization mesh
	temp = stream.readLine();
	parsed = temp.split(";");

	m_areaInfo.width_n  = parsed[0].toInt();
	m_areaInfo.height_n = parsed[1].toInt();

	// read dimensions for computation mesh
	temp = stream.readLine();
	parsed = temp.split(";");

	m_areaInfo.width  = parsed[0].toInt();
	m_areaInfo.height = parsed[1].toInt();

	// compute desired real-world pixel size [m]
	m_areaInfo.desiredPixelSize = m_areaInfo.width_n / m_areaInfo.width;

	// read coordinates for upper left corner of mesh
	temp = stream.readLine();
	parsed = temp.split(";");

	m_areaInfo.xLeft = parsed[0].toDouble();
	m_areaInfo.yLeft = parsed[1].toDouble();

	// read laz files
	numOfLazFiles = stream.readLine().toULongLong();
	m_areaInfo.lazFiles.resize(numOfLazFiles, std::string(""));

	for (size_t i = 0; i < numOfLazFiles; i++)
	{
		m_areaInfo.lazFiles[i] = stream.readLine().toStdString();
	}
}

bool DataHandler::readLazFiles()
{
	LASreadOpener lasReadOpener;
	LASreader* reader = nullptr;
	std::string fileName = "";
	int nPointsInMesh = 0;

	int nPixels = m_areaInfo.width_n * m_areaInfo.height_n;
	m_meshPixels.resize(nPixels, PixelPoints());
	m_DTM = new double[nPixels] { 0.0 };

	auto start = std::chrono::high_resolution_clock::now();

	for (int laz = 0; laz < m_areaInfo.lazFiles.size(); laz++)
	{
		lasReadOpener.add_file_name(m_areaInfo.lazFiles[laz].c_str());

		if (!lasReadOpener.active())
		{
			std::cout << "Laz file " << laz << "not active\n";
			return false;
		}

		fileName = lasReadOpener.get_file_name();
		std::cout << "reading file (" << laz + 1 << "/" << m_areaInfo.lazFiles.size() << "): " << fileName << "\n";

		reader = lasReadOpener.open();
		if (reader == nullptr)
		{
			std::cout << "Failed to open laz file" << laz << "\n";
			return false;
		}

		//printf("xBounds: [%.2lf, %.2lf]\n", reader->get_min_x(), reader->get_max_x());
		//printf("yBounds: [%.2lf, %.2lf]\n", reader->get_min_y(), reader->get_max_y());
		//printf("zBounds: [%.2lf, %.2lf]\n", reader->get_min_z(), reader->get_max_z());
		//std::cout << "nPoints: " << reader->npoints << "\n";

		double xLeft = m_areaInfo.xLeft;
		double yLeft = m_areaInfo.yLeft;
		double ptX = 0, ptY = 0, ptZ = 0;
		int ptClass = 0;
		double deltaX = 0, deltaY = 0;
		int i = -1; // row index
		int j = -1; // column index
		int pixelIdx = -1;

		while (reader->read_point())
		{
			ptX = reader->point.get_x();
			ptY = reader->point.get_y();
			ptZ = reader->point.get_z();
			ptClass = reader->point.get_classification();

			// check if point is from class 2, 3, 4 or 5 (ground or vegetation)
			if (ptClass < GROUND || ptClass > VEG_HIGH)
				continue;

			deltaX = ptX - xLeft;
			deltaY = ptY - yLeft;

			i = static_cast<int>(std::floor(std::abs(deltaY)));
			j = static_cast<int>(std::floor(std::abs(deltaX)));

			// check if point is inside mesh
			if (deltaY > 0.0 || i > m_areaInfo.height_n - 1)
				continue;
			
			if (deltaX < 0.0 || j > m_areaInfo.width_n - 1)
				continue;

			// save point to the corresponding mesh pixel
			pixelIdx = i * m_areaInfo.width_n + j;

			if (ptClass == GROUND)
			{
				m_meshPixels[pixelIdx].groundPts.xCoords.push_back(ptX);
				m_meshPixels[pixelIdx].groundPts.yCoords.push_back(ptY);
				m_meshPixels[pixelIdx].groundPts.zCoords.push_back(ptZ);

				nPointsInMesh++;

				continue;
			}

			m_meshPixels[pixelIdx].vegetationPts.xCoords.push_back(ptX);
			m_meshPixels[pixelIdx].vegetationPts.yCoords.push_back(ptY);
			m_meshPixels[pixelIdx].vegetationPts.zCoords.push_back(ptZ);
			m_meshPixels[pixelIdx].vegetationPts.ptClass.push_back(ptClass);

			nPointsInMesh++;
		}

		reader->close();
		delete reader;
	}

	std::cout << "nPointsInMesh: " << nPointsInMesh << "\n";
	
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	printf("\nReading LAZ files: %.4lf s\n", (double)duration.count() / 1000.0);

	return true;
}

void DataHandler::normalizePoints()
{
	double minZ = -1.0;
	double minV = -1.0;
	double minG = -1.0;

	bool noGroundPts = true;
	bool noVegetationPts = true;

	auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel for private(minZ, minV, minG, noGroundPts, noVegetationPts)
	for (int i = 0; i < m_meshPixels.size(); i++)
	{
		m_DTM[i] = NAN;

		// Check if there are points in pixel
		noGroundPts = m_meshPixels[i].groundPts.zCoords.empty();
		noVegetationPts = m_meshPixels[i].vegetationPts.zCoords.empty();
		
		if (noGroundPts && noVegetationPts)
			continue;

		if (noGroundPts)
			minG = DBL_MAX;
		else
			minG = *std::min_element(m_meshPixels[i].groundPts.zCoords.begin(), m_meshPixels[i].groundPts.zCoords.end());

		if (noVegetationPts)
			minV = DBL_MAX;
		else
			minV = *std::min_element(m_meshPixels[i].vegetationPts.zCoords.begin(), m_meshPixels[i].vegetationPts.zCoords.end());

		minZ = (minG < minV) ? minG : minV;

		m_DTM[i] = StatFunctions::mean(m_meshPixels[i].groundPts.zCoords);

		for (auto& z : m_meshPixels[i].vegetationPts.zCoords)
		{
			z -= minZ;
		}

		for (auto& z : m_meshPixels[i].groundPts.zCoords)
		{
			z -= minZ;
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	printf("\nNormalization: %.4lf s\n", (double)duration.count() / 1000.0);

}

void DataHandler::redistributePoints()
{
	int nPixels = m_areaInfo.width * m_areaInfo.height;
	m_meshPixelsRedistributed.resize(nPixels, PixelPoints());

	int desiredPixelSize = m_areaInfo.desiredPixelSize;
	int I = -1, J = -1;
	int index = -1;
	int INDEX = -1;
	int j = -1, k = -1, l = -1;

	std::vector<double>* v1 = nullptr;
	std::vector<double>* v2 = nullptr;

	std::vector<int>* v1i = nullptr;
	std::vector<int>* v2i = nullptr;

	auto start = std::chrono::high_resolution_clock::now();

	// iterate over computation mesh with desired pixel size -> (i,j)
#pragma omp parallel for private(j,k,l,I,J,v1,v2,v1i,v2i,index,INDEX)
	for (int i = 0; i < m_areaInfo.height; i++)
	{
		for (j = 0; j < m_areaInfo.width; j++)
		{
			// iterate over normalization mesh -> (I,J): normalization mesh, (k,l): mask
			for (k = 0; k < desiredPixelSize; k++)
			{
				for (l = 0; l < desiredPixelSize; l++)
				{
					// compute pixel indices in normalization mesh
					I = i * desiredPixelSize + k;
					J = j * desiredPixelSize + l;

					if ((I < m_areaInfo.height_n) && (J < m_areaInfo.width_n)) // check if inside normalization mesh
					{
						INDEX = I * m_areaInfo.width_n + J; // compute correct INDEX in normalization mesh
						index = i * m_areaInfo.width   + j; // compute correct index in desired computational mesh
						
						// move vegetation points
						v1 = &m_meshPixelsRedistributed[index].vegetationPts.xCoords;
						v2 = &m_meshPixels[INDEX].vegetationPts.xCoords;
						v1->insert(v1->end(), v2->begin(), v2->end()); // insert v2 to v1
						v2->clear(); v2->shrink_to_fit(); // clear v2 contents

						v1 = &m_meshPixelsRedistributed[index].vegetationPts.yCoords;
						v2 = &m_meshPixels[INDEX].vegetationPts.yCoords;
						v1->insert(v1->end(), v2->begin(), v2->end()); // insert v2 to v1
						v2->clear(); v2->shrink_to_fit(); // clear v2 contents

						v1 = &m_meshPixelsRedistributed[index].vegetationPts.zCoords;
						v2 = &m_meshPixels[INDEX].vegetationPts.zCoords;
						v1->insert(v1->end(), v2->begin(), v2->end()); // insert v2 to v1
						v2->clear(); v2->shrink_to_fit(); // clear v2 contents

						v1i = &m_meshPixelsRedistributed[index].vegetationPts.ptClass;
						v2i = &m_meshPixels[INDEX].vegetationPts.ptClass;
						v1i->insert(v1i->end(), v2i->begin(), v2i->end()); // insert v2 to v1
						v2i->clear(); v2i->shrink_to_fit(); // clear v2 contents

						// move ground points
						v1 = &m_meshPixelsRedistributed[index].groundPts.xCoords;
						v2 = &m_meshPixels[INDEX].groundPts.xCoords;
						v1->insert(v1->end(), v2->begin(), v2->end()); // insert v2 to v1
						v2->clear(); v2->shrink_to_fit(); // clear v2 contents

						v1 = &m_meshPixelsRedistributed[index].groundPts.yCoords;
						v2 = &m_meshPixels[INDEX].groundPts.yCoords; // insert v2 to v1
						v1->insert(v1->end(), v2->begin(), v2->end()); // clear v2 contents
						v2->clear(); v2->shrink_to_fit(); // clear v2 contents

						v1 = &m_meshPixelsRedistributed[index].groundPts.zCoords;
						v2 = &m_meshPixels[INDEX].groundPts.zCoords; // insert v2 to v1
						v1->insert(v1->end(), v2->begin(), v2->end()); // clear v2 contents
						v2->clear(); v2->shrink_to_fit(); // clear v2 contents

					}
				}
			} // END of mask iteration

		} // proceed to new pixel
	}

	m_meshPixels.clear();
	m_meshPixels.shrink_to_fit();

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	printf("\nPoints redistribution: %.4lf s\n", (double)duration.count() / 1000.0);
}

void DataHandler::computeMetrics()
{
	int nPixels = m_areaInfo.width * m_areaInfo.height;

	std::cout << "Compute metrics started...\n";
	auto start = std::chrono::high_resolution_clock::now();

	m_metrics.resize(nMetrics);
	for (int i = 0; i < nMetrics; i++)
	{
		m_metrics[i] = new double[nPixels] {};
	}

	double meanZ = -1.0, varZ = -1.0, stdZ = -1.0;
	int nAllPts = -1;
	int nGroundPts = -1;
	int nVegetationPts = -1;
	double temp = -1.0;
	std::vector<double>* zValues = nullptr;
	int m = 0;

#pragma omp parallel for private(zValues, nGroundPts, nVegetationPts, nAllPts, meanZ, varZ, stdZ, temp, m)
	for (int i = 0; i < nPixels; i++)
	{
		std::vector<double> emptyZ = { 0.0 };

		// set default metrics value to NAN
		for (m = 0; m < nMetrics; m++)
			m_metrics[m][i] = NAN;
		//std::cout << "nans assigned\n";

		zValues = &m_meshPixelsRedistributed[i].vegetationPts.zCoords;

		nGroundPts = m_meshPixelsRedistributed[i].groundPts.zCoords.size();
		nVegetationPts = zValues->size();
		nAllPts = nGroundPts + nVegetationPts;

		//std::cout << "nPoints found\n";

		if (nAllPts == 0)
			continue;

		if (nVegetationPts == 0)
			zValues = &emptyZ;

		// ECOSYSTEM HEIGHT METRICS
		m_metrics[Hmax][i] = *std::max_element(zValues->begin(), zValues->end());

		meanZ = StatFunctions::mean(*zValues);
		m_metrics[Hmean][i] = meanZ;

		m_metrics[Hmedian][i] = StatFunctions::percentile(*zValues, 50.0);

		m_metrics[Hp25][i] = StatFunctions::percentile(*zValues, 25.0);

		m_metrics[Hp75][i] = StatFunctions::percentile(*zValues, 75.0);

		m_metrics[Hp95][i] = StatFunctions::percentile(*zValues, 95.0);

		m_metrics[PPR][i] = static_cast<double>(nGroundPts) / nAllPts;

		temp = StatFunctions::pointsAbove(*zValues, meanZ);
		m_metrics[DAM_z][i] = (nVegetationPts == 0) ? 0.0 : temp;

		//std::cout << "Metrics 1/3 done\n";

		// ECOSYSTEM COVER METRICS
		temp = StatFunctions::pointsBellow(*zValues, 1.0);
		m_metrics[BR_bellow_1][i] = (nVegetationPts == 0) ? 0.0 : temp / nVegetationPts;

		temp = StatFunctions::pointsBetween(*zValues, 1.0, 2.0);
		m_metrics[BR_1_2][i] = (nVegetationPts == 0) ? 0.0 : temp / nVegetationPts;
		
		temp = StatFunctions::pointsBetween(*zValues, 2.0, 3.0);
		m_metrics[BR_2_3][i] = (nVegetationPts == 0) ? 0.0 : temp / nVegetationPts;
		
		temp = StatFunctions::pointsAbove(*zValues, 3.0);
		m_metrics[BR_above_3][i] = (nVegetationPts == 0) ? 0.0 : temp / nVegetationPts;
		
		temp = StatFunctions::pointsBetween(*zValues, 3.0, 4.0);
		m_metrics[BR_3_4][i] = (nVegetationPts == 0) ? 0.0 : temp / nVegetationPts;
		
		temp = StatFunctions::pointsBetween(*zValues, 4.0, 5.0);
		m_metrics[BR_4_5][i] = (nVegetationPts == 0) ? 0.0 : temp / nVegetationPts;
		
		temp = StatFunctions::pointsBellow(*zValues, 5.0);
		m_metrics[BR_bellow_5][i] = (nVegetationPts == 0) ? 0.0 : temp / nVegetationPts;
		
		temp = StatFunctions::pointsBetween(*zValues, 5.0, 20.0);
		m_metrics[BR_5_20][i] = (nVegetationPts == 0) ? 0.0 : temp / nVegetationPts;
		
		temp = StatFunctions::pointsAbove(*zValues, 20.0);
		m_metrics[BR_above_20][i] = (nVegetationPts == 0) ? 0.0 : temp / nVegetationPts;
		
		//std::cout << "Metrics 2/3 done\n";

		// ECOSYSTEM STRUCTURAL COMPLEXITY METRICS
		m_metrics[Hkurt][i] = StatFunctions::kurtosis(*zValues, meanZ);

		m_metrics[Hskew][i] = StatFunctions::skewness(*zValues, meanZ);

		varZ = StatFunctions::variance(*zValues, meanZ);
		m_metrics[Hvar][i] = varZ;

		stdZ = std::sqrt(varZ);
		m_metrics[Hstd][i] = stdZ;

		m_metrics[Coeff_var_z][i] = stdZ / meanZ;

		m_metrics[Shannon][i] = StatFunctions::shannonIndex(*zValues);

		//std::cout << "Metrics 3/3 done\n";

	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	printf("done: %.4lf s\n", (double)duration.count() / 1000.0);
}

void DataHandler::exportMetrics(std::string fileName)
{
	time_t now = time(0);
	tm* ltm = localtime(&now);
	
	std::string timeStamp = QString("../../%1_%2_%3_%4-%5-%6_").arg(ltm->tm_year + 1900,4).arg(ltm->tm_mon,2, 10, QLatin1Char('0')).arg(ltm->tm_mday, 2, 10, QLatin1Char('0')).arg(ltm->tm_hour, 2, 10, QLatin1Char('0')).arg(ltm->tm_min, 2, 10, QLatin1Char('0')).arg(ltm->tm_sec, 2, 10, QLatin1Char('0')).toStdString();
	timeStamp.append(fileName);
	fileName = timeStamp;
	fileName.append(QString("_h=%1m").arg(m_areaInfo.desiredPixelSize).toStdString());
	
	if (fileName.substr(fileName.length() - 4, 4) != ".tif")
	{
		fileName.append(".tif");
	}
	
	std::cout << "Exporting metrics...";
	// load drivers
	GDALAllRegister();
	GDALDriver* poDriver;
	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

	GDALDataset* poDstDS = nullptr;
	char** papszOptions = NULL;
	poDstDS = poDriver->Create(fileName.c_str(), m_areaInfo.width, m_areaInfo.height, m_metrics.size(), GDT_Float64, papszOptions);

	// geotransform -> x,y lavy horny vrchol, dlzky v metroch pre pixely, rotacie (tie su 0)
	double adfGeoTransform[6] = { m_areaInfo.xLeft,
								  static_cast<double>(m_areaInfo.desiredPixelSize),
		                          0.0,
		                          m_areaInfo.yLeft,
		                          0.0,
		                         static_cast<double>(- m_areaInfo.desiredPixelSize)};
	poDstDS->SetGeoTransform(adfGeoTransform);

	// Spatial reference system for tif
	OGRSpatialReference oSRS;
	char* pszSRS_WKT = NULL;
	oSRS.importFromEPSG(8353);
	oSRS.exportToWkt(&pszSRS_WKT);
	poDstDS->SetProjection(pszSRS_WKT);
	CPLFree(pszSRS_WKT);

	// Export bands
	GDALRasterBand* poBand;

	for (int i = 0; i < m_metrics.size(); i++)
	{
		poBand = poDstDS->GetRasterBand(i + 1);
		poBand->SetDescription(m_bandNames[i].c_str());
		poBand->RasterIO(GF_Write, 0, 0, m_areaInfo.width, m_areaInfo.height,
			m_metrics[i], m_areaInfo.width, m_areaInfo.height, GDT_Float64, 0, 0);
	}

	//poDstDS->SetMetadata();
	// Close file
	GDALClose((GDALDatasetH)poDstDS);

	std::cout << " done\n";
}

bool DataHandler::exportLAS(std::string fileName)
{
	if (fileName.substr(fileName.length() - 4, 4) != ".las")
	{
		fileName.append(".las");
	}

	LASwriteOpener lasWriteOpener;
	lasWriteOpener.set_file_name(fileName.c_str());

	if (!lasWriteOpener.active())
	{
		std::cout << "Las file " << lasWriteOpener.get_file_name() << "not active\n";
		return false;
	}

	LASheader lasHeader;

	lasHeader.x_scale_factor = 1.0;
	lasHeader.y_scale_factor = 1.0;
	lasHeader.z_scale_factor = 1.0;
	lasHeader.x_offset = 0.0;
	lasHeader.y_offset = 0.0;
	lasHeader.z_offset = 0.0;
	lasHeader.point_data_format;

	return true;
}

void DataHandler::exportDTM(std::string fileName)
{
	time_t now = time(0);
	tm* ltm = localtime(&now);

	std::string timeStamp = QString("../../DTM_%1_%2_%3_%4-%5-%6_").arg(ltm->tm_year + 1900, 4).arg(ltm->tm_mon, 2, 10, QLatin1Char('0')).arg(ltm->tm_mday, 2, 10, QLatin1Char('0')).arg(ltm->tm_hour, 2, 10, QLatin1Char('0')).arg(ltm->tm_min, 2, 10, QLatin1Char('0')).arg(ltm->tm_sec, 2, 10, QLatin1Char('0')).toStdString();
	timeStamp.append(fileName);
	fileName = timeStamp;

	if (fileName.substr(fileName.length() - 4, 4) != ".tif")
	{
		fileName.append(".tif");
	}

	std::cout << "Exporting DTM...";
	// load drivers
	GDALAllRegister();
	GDALDriver* poDriver;
	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

	GDALDataset* poDstDS = nullptr;
	char** papszOptions = NULL;
	poDstDS = poDriver->Create(fileName.c_str(), m_areaInfo.width_n, m_areaInfo.height_n, 1, GDT_Float64, papszOptions);

	// geotransform -> x,y lavy horny vrchol, dlzky v metroch pre pixely, rotacie (tie su 0)
	double adfGeoTransform[6] = { m_areaInfo.xLeft,
								  1.0,
								  0.0,
								  m_areaInfo.yLeft,
								  0.0,
								  -1.0 };
	poDstDS->SetGeoTransform(adfGeoTransform);

	// Spatial reference system for tif
	OGRSpatialReference oSRS;
	char* pszSRS_WKT = NULL;
	oSRS.importFromEPSG(8353);
	oSRS.exportToWkt(&pszSRS_WKT);
	poDstDS->SetProjection(pszSRS_WKT);
	CPLFree(pszSRS_WKT);

	// Export bands
	GDALRasterBand* poBand;

	poBand = poDstDS->GetRasterBand(1);
	poBand->SetDescription("Digital Terrain Model");
	poBand->RasterIO(GF_Write, 0, 0, m_areaInfo.width_n, m_areaInfo.height_n,
		m_DTM, m_areaInfo.width_n, m_areaInfo.height_n, GDT_Float64, 0, 0);
	
	//poDstDS->SetMetadata();
	// Close file
	GDALClose((GDALDatasetH)poDstDS);

	std::cout << " done\n";
}

void DataHandler::performCalculation(QString infoFile)
{
	QFile inputFile = QFile(infoFile);
	if (!inputFile.open(QIODevice::ReadOnly))
	{
		std::cout << "Info file failed to open\n";
		exit(-1);
	}
	
	QTextStream inStream(&inputFile);

	QString temp = inputFile.fileName().mid(inputFile.fileName().lastIndexOf("/") + 1);
	m_areaName = temp.left(temp.length() - 4).toStdString();

	readInputFile(inStream);

	inputFile.close();

	std::cout << "width_n: " << m_areaInfo.width_n << "\nheight_n: " << m_areaInfo.height_n << "\n";
	std::cout << "width: " << m_areaInfo.width << "\nheight: " << m_areaInfo.height << "\n";
	std::cout << "(x,y): [" << m_areaInfo.xLeft << ", " << m_areaInfo.yLeft << "]\n";
	std::cout << "laz files: " << m_areaInfo.lazFiles.size() << "\n";

	if (!readLazFiles())
	{
		std::cout << "failed to read laz files\n";
		exit(-2);
	}

	normalizePoints();

	exportDTM(m_areaName);

	redistributePoints();

	computeMetrics();

	exportMetrics(m_areaName);
}
