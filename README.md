# Qt OpenCV 学习项目

一个最小的 Qt 6 Widgets 学习项目，用 OpenCV 读取/生成图片并展示。

## 依赖
- Qt 6.8.3（安装在 /home/zyh/Qt/6.8.3）
- OpenCV（通过 apt 安装）

## 构建
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/home/zyh/Qt/6.8.3/gcc_64
cmake --build build
```

## 运行
```bash
./build/QtOpenCVWebpViewer
```

## 目录结构
- main.cpp：入口
- main_window.*：主窗口（首页+导航）
- imwrite_lesson_widget.*：imwrite 子项目
- imread_lesson_widget.*：imread 子项目
- mat_to_qimage.*：OpenCV 到 QImage 转换
- generate_wolf_ppt.py："狼来了"故事 PPT 生成脚本

## 生成"狼来了"PPT

使用 Python 脚本自动生成一个约 20 页的幼儿园绘本风格 PPT，配有 Pillow 绘制的插图。

### 直接下载

已生成的 PPT 文件可以直接从仓库下载：[狼来了.pptx](狼来了.pptx)

### 自行生成

```bash
pip install python-pptx Pillow
python generate_wolf_ppt.py
```

运行后会在当前目录生成 `狼来了.pptx`。
