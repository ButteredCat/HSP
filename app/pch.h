#pragma once

// C++ standard
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>

// Boost
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

// GDAL
#include <gdal.h>
#include <gdal_priv.h>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

// spdlog
#include "bits/time.h"
#include "spdlog/cfg/env.h"  // support for loading levels from the environment variable
#include "spdlog/fmt/ostr.h"  // support for user defined types
#include "spdlog/spdlog.h"