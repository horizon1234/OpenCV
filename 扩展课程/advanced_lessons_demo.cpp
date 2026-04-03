#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>

#include "advanced_lesson_runtime.h"

namespace
{
void printUsage(const char *program)
{
    std::cout << "Usage: " << program << " <lesson-id> [demo-id]\n";
    std::cout << "lesson-id: 17 ~ 60\n";
    std::cout << "demo-id: 0 or 1 (default: 0)\n";
}
} // namespace

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    const int lessonId = std::stoi(argv[1]);
    const int demoId = (argc >= 3) ? std::stoi(argv[2]) : 0;

    std::string statusMessage;
    const bool ok = runAdvancedLessonDemo(lessonId, demoId, statusMessage);
    std::cout << statusMessage << std::endl;
    if (!ok)
    {
        return 1;
    }

    cv::waitKey(0);

    closeAdvancedLessonWindows();
    return 0;
}
