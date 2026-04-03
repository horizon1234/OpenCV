#include "advanced_lesson_widget_support.h"

#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

#include <QStringList>

#include "advanced_lesson_runtime.h"

namespace
{
QWidget *createCard(const QString &title, const QString &body, QWidget *parent)
{
    auto *frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setStyleSheet(QStringLiteral(
        "QFrame {"
        "  background: #f7f9fc;"
        "  border: 1px solid #d8e1ee;"
        "  border-radius: 10px;"
        "}"
    ));

    auto *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(6);

    auto *titleLabel = new QLabel(title, frame);
    titleLabel->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 600; color: #1f2937;"));

    auto *bodyLabel = new QLabel(body, frame);
    bodyLabel->setWordWrap(true);
    bodyLabel->setTextFormat(Qt::RichText);
    bodyLabel->setStyleSheet(QStringLiteral("color: #465467; line-height: 1.5;"));

    layout->addWidget(titleLabel);
    layout->addWidget(bodyLabel);
    return frame;
}

QString buildGuideBody(const AdvancedLessonConfig &config)
{
    QStringList items;
    items << QStringLiteral("1. 先通读本课摘要，明确本课解决的是哪类视觉问题。")
          << QStringLiteral("2. 先运行第一个核心演示，再按按钮顺序观察参数变化和中间结果。")
          << QStringLiteral("3. 每跑完一个 demo，都记录“输入变了什么、输出变了什么、为什么会这样”。")
          << QStringLiteral("4. 如果某个结果看起来异常，先检查输入质量，再检查参数量级，最后再怀疑算法实现。")
          << QStringLiteral("5. 建议把本课结果和前后课程串起来看，判断它更适合作为最终输出还是预处理环节。");

    if (!config.demos.empty())
    {
        items << QStringLiteral("6. 本课共有 %1 个互动演示，适合至少做一轮顺序实验和一轮逆向排错实验。")
                     .arg(static_cast<int>(config.demos.size()));
    }

    return QStringLiteral("<ul><li>%1</li></ul>")
        .arg(items.join(QStringLiteral("</li><li>")));
}

QString buildDemoNotesBody(const AdvancedLessonConfig &config)
{
    QStringList sections;
    const int demoCount = static_cast<int>(config.demos.size());
    for (int index = 0; index < demoCount; ++index)
    {
        const auto &demo = config.demos[static_cast<std::size_t>(index)];
        sections << QStringLiteral("<b>%1.</b> %2<br/>%3")
                        .arg(index + 1)
                        .arg(QString::fromStdString(demo.buttonText))
                        .arg(QString::fromStdString(demo.description));
    }

    if (sections.isEmpty())
    {
        sections << QStringLiteral("本课当前没有单独的 demo 描述，请直接参考状态栏提示和讲义步骤进行实验。");
    }

    return sections.join(QStringLiteral("<br/><br/>"));
}
} // namespace

QWidget *createAdvancedLessonGuideCard(int lessonId, QWidget *parent)
{
    const auto &config = advancedLessonConfig(lessonId);
    return createCard(QStringLiteral("建议学习顺序"), buildGuideBody(config), parent);
}

QWidget *createAdvancedLessonDemoNotesCard(int lessonId, QWidget *parent)
{
    const auto &config = advancedLessonConfig(lessonId);
    return createCard(QStringLiteral("本课 Demo 导读"), buildDemoNotesBody(config), parent);
}