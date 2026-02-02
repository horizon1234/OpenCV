#pragma once

#include <QImage>
#include <opencv2/opencv.hpp>

QImage matToQImage(const cv::Mat &mat);
