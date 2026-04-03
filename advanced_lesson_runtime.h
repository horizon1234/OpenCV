#pragma once

#include <string>
#include <vector>

struct AdvancedLessonDemoSpec
{
    int demoId;
    std::string buttonText;
    std::string description;
};

struct AdvancedLessonConfig
{
    int lessonId;
    std::string title;
    std::string summary;
    std::vector<AdvancedLessonDemoSpec> demos;
};

const AdvancedLessonConfig &advancedLessonConfig(int lessonId);
bool runAdvancedLessonDemo(int lessonId, int demoId, std::string &statusMessage);
void closeAdvancedLessonWindows();