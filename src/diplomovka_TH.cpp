#include "diplomovka_TH.h"


DataHandler::DataHandler(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	int nprocs = 6;
	omp_set_num_threads(nprocs);

	QString infoFile = "E:/_school/_diplomovka/c++/91E0_area.kml_data.txt";
	
	//std::vector<double> v = { 1.0,2.0,3.0,6.0,6.0,7.0,11.0,12.0,15.0,16.0,19.0,19.0,20.0,21.0,23.0,27.0,30.0,34.0,36.0,40.0 };

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
	std::cout << "nPixels:" << nPixels << "\n";

	auto start = std::chrono::high_resolution_clock::now();

	m_metrics.resize(1);
	for (int i = 0; i < m_metrics.size(); i++)
	{
		m_metrics[i] = new double[nPixels] {};
	}

	double maxZ = -1.0;
	int nAllPts = -1;
	int nGroundPts = -1;
	int nVegetationPts = -1;
	std::vector<double>* zValues = nullptr;

#pragma omp parallel for private(zValues, nGroundPts, nVegetationPts, nAllPts, maxZ)
	for (int i = 0; i < nPixels; i++)
	{
		m_metrics[0][i] = NAN;

		zValues = &m_meshPixelsRedistributed[i].vegetationPts.zCoords;

		nGroundPts = m_meshPixelsRedistributed[i].groundPts.zCoords.size();
		nVegetationPts = zValues->size();
		nAllPts = nGroundPts + nVegetationPts;


		if (nAllPts == 0)
			continue;

		// PPR
		//m_metrics[1][i] = static_cast<double>(nGroundPts) / nAllPts;

		if (nVegetationPts == 0)
			maxZ = 0.0;
		else
			maxZ = *std::max_element(zValues->begin(), zValues->end());

		
		// Hmax
		m_metrics[0][i] = maxZ;
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	printf("\nCompute metrics: %.4lf s\n", (double)duration.count() / 1000.0);
}

void DataHandler::exportMetrics(std::string fileName)
{
	time_t now = time(0);
	tm* ltm = localtime(&now);
	
	std::string timeStamp = QString("%1_%2_%3_%4-%5-%6_").arg(ltm->tm_year + 1900,4).arg(ltm->tm_mon,2, 10, QLatin1Char('0')).arg(ltm->tm_mday, 2, 10, QLatin1Char('0')).arg(ltm->tm_hour, 2, 10, QLatin1Char('0')).arg(ltm->tm_min, 2, 10, QLatin1Char('0')).arg(ltm->tm_sec, 2, 10, QLatin1Char('0')).toStdString();
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
								  m_areaInfo.desiredPixelSize,
		                          0,
		                          m_areaInfo.yLeft,
		                          0,
		                         -m_areaInfo.desiredPixelSize };
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

	/*for (size_t i = 0; i < m_areaInfo.lazFiles.size(); i++)
	{
		std::cout << "file " << i << ": " << m_areaInfo.lazFiles[i] << "\n";
	}*/

	if (!readLazFiles())
	{
		std::cout << "failed to read laz files\n";
		exit(-2);
	}

	normalizePoints();

	redistributePoints();

	computeMetrics();

	exportMetrics(m_areaName);
}
