#pragma once

#include <vector>

namespace StatFunctions
{
	double mean(const std::vector<double>& data);

	double median(std::vector<double> data);

	double percentile(std::vector<double> data);
}