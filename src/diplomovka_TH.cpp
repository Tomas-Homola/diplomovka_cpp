#include "diplomovka_TH.h"


DataHandler::DataHandler(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	int nprocs = 6;
	omp_set_num_threads(nprocs);

	QString infoFile = "E:/_school/_diplomovka/c++/BielyKriz_area.kml_data.txt";

	QFile inputFile = QFile(infoFile);
	if (!inputFile.open(QIODevice::ReadOnly))
	{
		std::cout << "Info file failed to open\n";
		exit(-1);
	}
	
	QTextStream inStream(&inputFile);

	readInputFile(inStream);

	inputFile.close();

	std::cout << "width_n: " << m_areaInfo.width_n << "\nheight_n: " << m_areaInfo.height_n << "\n";
	std::cout << "width: " << m_areaInfo.width << "\nheight: " << m_areaInfo.height << "\n";
	std::cout << "(x,y): [" << m_areaInfo.xLeft << ", " << m_areaInfo.yLeft << "]\n";
	std::cout << "laz files: " << m_areaInfo.lazFiles.size() << "\n";
	
	for (size_t i = 0; i < m_areaInfo.lazFiles.size(); i++)
	{
		std::cout << "file " << i << ": " << m_areaInfo.lazFiles[i] << "\n";
	}

	int nPixels = m_areaInfo.width_n * m_areaInfo.height_n;
	m_meshPixels.resize(nPixels, Points());


	if (!readLazFiles())
	{
		std::cout << "Failed to read laz files\n";
		exit(-2);
	}

	normalizePoints();

	//std::vector<double> a = { 1, 2, 3, 4 };
	//std::vector<double> b = { 5, 6, 7 };
	//
	//a.insert(std::end(a), std::begin(b), std::end(b));
	//b.clear();
	//b.shrink_to_fit();

	m_metrics.resize(1);
	m_metrics[0] = new double[nPixels] {};

	double maxZ = -1.0;
	for (int i = 0; i < nPixels; i++)
	{
		m_metrics[0][i] = NAN;
		if (m_meshPixels[i].zCoords.empty())
			continue;

		maxZ = *std::max_element(m_meshPixels[i].zCoords.begin(), m_meshPixels[i].zCoords.end());

		m_metrics[0][i] = maxZ;
	}

	exportMetrics("bielyKriz_namedBand_export.tif");
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
		std::cout << "reading file: " << fileName << "\n";

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

			m_meshPixels[pixelIdx].xCoords.push_back(ptX);
			m_meshPixels[pixelIdx].yCoords.push_back(ptY);
			m_meshPixels[pixelIdx].zCoords.push_back(ptZ);
			m_meshPixels[pixelIdx].ptClass.push_back(ptClass);

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

	auto start = std::chrono::high_resolution_clock::now();

#pragma omp parallel for private(minZ)
	for (int i = 0; i < m_meshPixels.size(); i++)
	{
		//std::cout << "pixel: " << i << "\n";
		if (m_meshPixels[i].zCoords.empty())
			continue;

		minZ = *std::min_element(m_meshPixels[i].zCoords.begin(), m_meshPixels[i].zCoords.end());

		for (auto& z : m_meshPixels[i].zCoords)
		{
			z -= minZ;
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

	printf("\nNormalization: %.4lf s\n", (double)duration.count() / 1000.0);

}

void DataHandler::computeMetrics()
{

}

void DataHandler::exportMetrics(std::string fileName)
{
	if (fileName.substr(fileName.length() - 4, 4) != ".tif")
	{
		fileName.append(".tif");
	}

	// load drivers
	GDALAllRegister();
	GDALDriver* poDriver;
	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

	GDALDataset* poDstDS = nullptr;
	char** papszOptions = NULL;
	poDstDS = poDriver->Create(fileName.c_str(), m_areaInfo.width_n, m_areaInfo.height_n, 1, GDT_Float64, papszOptions);

	// geotransform -> x,y lavy horny vrchol, dlzky v metroch pre pixely, rotacie (tie su 0)
	double adfGeoTransform[6] = { m_areaInfo.xLeft, 1, 0, m_areaInfo.yLeft, 0, -1 };
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
	poBand->SetDescription("1_Hmax");
	poBand->RasterIO(GF_Write, 0, 0, m_areaInfo.width_n, m_areaInfo.height_n,
		m_metrics[0], m_areaInfo.width_n, m_areaInfo.height_n, GDT_Float64, 0, 0);
	
	// Close file
	GDALClose((GDALDatasetH)poDstDS);
}
