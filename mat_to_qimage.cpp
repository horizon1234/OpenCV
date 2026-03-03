#include "mat_to_qimage.h"

QImage matToQImage(const cv::Mat &mat)
{
    if (mat.empty())
    {
        return {};
    }

    switch (mat.type())
    {
    case CV_8UC1:
    {
        QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8);
        return image.copy();
    }
    case CV_8UC3:
    {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        QImage image(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
        return image.copy();
    }
    case CV_8UC4:
    {
        cv::Mat rgba;
        cv::cvtColor(mat, rgba, cv::COLOR_BGRA2RGBA);
        QImage image(rgba.data, rgba.cols, rgba.rows, static_cast<int>(rgba.step), QImage::Format_RGBA8888);
        return image.copy();
    }
    default:
        return {};
    }
}
