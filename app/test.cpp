#include <cmath>
#include <ctime>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "bits/time.h"

#define TestCUDA true

int main() {
    std::clock_t begin = std::clock();

    try {
        cv::String filename = "/home/xiaoyc/repo/test/screenshot.png";
        cv::Mat srcHost = cv::imread(filename, cv::IMREAD_GRAYSCALE);
        cv::Mat resultHost;

        for (int i = 0; i < 1000; i++) {
            if (TestCUDA) {
                cv::cuda::GpuMat dst, src;
                src.upload(srcHost);

                // cv::cuda::threshold(src,dst,128.0,255.0, CV_THRESH_BINARY);
                cv::cuda::bilateralFilter(src, dst, 3, 1, 1);

                dst.download(resultHost);
            } else {
                cv::Mat dst;
                cv::bilateralFilter(srcHost, dst, 3, 1, 1);
            }
        }

        // cv::imshow("Result",resultHost);
        // cv::waitKey();

    } catch (const cv::Exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
    }

    std::clock_t end = std::clock();
    std::cout << double(end - begin) / CLOCKS_PER_SEC << std::endl;
}
