# Copilot Instructions

## Project overview
- Minimal Qt 6 Widgets app that loads a WebP image via OpenCV and displays it in a `QLabel`.
- Entry point: [main.cpp](main.cpp).

## Build & run (CMake)
- Configure with Qt 6.8.3 installed at /home/zyh/Qt/6.8.3:
  - cmake -S . -B build -DCMAKE_PREFIX_PATH=/home/zyh/Qt/6.8.3
- Build:
  - cmake --build build
- Run from build directory or project root:
  - ./build/QtOpenCVWebpViewer [optional-image-path]

## Key dependencies
- Qt 6 Widgets (`Qt6::Widgets`) and OpenCV (>= 4.10) located via `find_package` in [CMakeLists.txt](CMakeLists.txt).
- OpenCV must be installed via apt; do not switch to source builds or vcpkg for this repo.

## Code patterns
- Image loading uses OpenCV `cv::imread(..., IMREAD_UNCHANGED)` and conversion to `QImage` in `matToQImage`.
- Supported formats: 8-bit grayscale, 8-bit BGR, and 8-bit BGRA.
- Default image path is `cat.jpg` in the working directory; can be overridden by CLI argument.

## Files
- Build config: [CMakeLists.txt](CMakeLists.txt)
- UI/app logic: [main.cpp](main.cpp)
