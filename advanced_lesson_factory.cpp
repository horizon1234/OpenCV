#include "advanced_lesson_factory.h"
#include "lesson_17/neighborhood_filter_lesson_widget.h"
#include "lesson_18/smoothing_filter_lesson_widget.h"
#include "lesson_19/denoise_filter_lesson_widget.h"
#include "lesson_20/sharpen_enhance_lesson_widget.h"
#include "lesson_21/gradient_detection_lesson_widget.h"
#include "lesson_22/canny_lesson_widget.h"
#include "lesson_23/find_contours_lesson_widget.h"
#include "lesson_24/contour_features_lesson_widget.h"
#include "lesson_25/convex_hull_lesson_widget.h"
#include "lesson_26/polygon_approx_lesson_widget.h"
#include "lesson_27/moments_centroid_lesson_widget.h"
#include "lesson_28/hough_lines_lesson_widget.h"
#include "lesson_29/hough_circles_lesson_widget.h"
#include "lesson_30/resize_interpolation_lesson_widget.h"
#include "lesson_31/geometric_transform_lesson_widget.h"
#include "lesson_32/perspective_transform_lesson_widget.h"
#include "lesson_33/pyramid_lesson_widget.h"
#include "lesson_34/template_matching_lesson_widget.h"
#include "lesson_35/histogram_compare_lesson_widget.h"
#include "lesson_36/back_projection_lesson_widget.h"
#include "lesson_37/dft_lesson_widget.h"
#include "lesson_38/frequency_filter_lesson_widget.h"
#include "lesson_39/restoration_lesson_widget.h"
#include "lesson_40/inpaint_lesson_widget.h"
#include "lesson_41/nl_means_lesson_widget.h"
#include "lesson_42/distance_transform_lesson_widget.h"
#include "lesson_43/watershed_lesson_widget.h"
#include "lesson_44/grabcut_lesson_widget.h"
#include "lesson_45/blending_lesson_widget.h"
#include "lesson_46/colormap_lesson_widget.h"
#include "lesson_47/corner_detection_lesson_widget.h"
#include "lesson_48/feature_detection_lesson_widget.h"
#include "lesson_49/feature_matching_lesson_widget.h"
#include "lesson_50/video_io_lesson_widget.h"
#include "lesson_51/background_subtraction_lesson_widget.h"
#include "lesson_52/frame_difference_lesson_widget.h"
#include "lesson_53/optical_flow_lesson_widget.h"
#include "lesson_54/camshift_meanshift_lesson_widget.h"
#include "lesson_55/homography_stitching_lesson_widget.h"
#include "lesson_56/camera_calibration_lesson_widget.h"
#include "lesson_57/undistort_lesson_widget.h"
#include "lesson_58/stereo_vision_lesson_widget.h"
#include "lesson_59/dnn_inference_lesson_widget.h"
#include "lesson_60/optimization_lesson_widget.h"

QWidget *createAdvancedLessonWidget(int lessonId, QWidget *parent)
{
    switch (lessonId)
    {
    case 17:
        return new NeighborhoodFilterLessonWidget(parent);
    case 18:
        return new SmoothingFilterLessonWidget(parent);
    case 19:
        return new DenoiseFilterLessonWidget(parent);
    case 20:
        return new SharpenEnhanceLessonWidget(parent);
    case 21:
        return new GradientDetectionLessonWidget(parent);
    case 22:
        return new CannyLessonWidget(parent);
    case 23:
        return new FindContoursLessonWidget(parent);
    case 24:
        return new ContourFeaturesLessonWidget(parent);
    case 25:
        return new ConvexHullLessonWidget(parent);
    case 26:
        return new PolygonApproxLessonWidget(parent);
    case 27:
        return new MomentsCentroidLessonWidget(parent);
    case 28:
        return new HoughLinesLessonWidget(parent);
    case 29:
        return new HoughCirclesLessonWidget(parent);
    case 30:
        return new ResizeInterpolationLessonWidget(parent);
    case 31:
        return new GeometricTransformLessonWidget(parent);
    case 32:
        return new PerspectiveTransformLessonWidget(parent);
    case 33:
        return new PyramidLessonWidget(parent);
    case 34:
        return new TemplateMatchingLessonWidget(parent);
    case 35:
        return new HistogramCompareLessonWidget(parent);
    case 36:
        return new BackProjectionLessonWidget(parent);
    case 37:
        return new DftLessonWidget(parent);
    case 38:
        return new FrequencyFilterLessonWidget(parent);
    case 39:
        return new RestorationLessonWidget(parent);
    case 40:
        return new InpaintLessonWidget(parent);
    case 41:
        return new NlMeansLessonWidget(parent);
    case 42:
        return new DistanceTransformLessonWidget(parent);
    case 43:
        return new WatershedLessonWidget(parent);
    case 44:
        return new GrabcutLessonWidget(parent);
    case 45:
        return new BlendingLessonWidget(parent);
    case 46:
        return new ColormapLessonWidget(parent);
    case 47:
        return new CornerDetectionLessonWidget(parent);
    case 48:
        return new FeatureDetectionLessonWidget(parent);
    case 49:
        return new FeatureMatchingLessonWidget(parent);
    case 50:
        return new VideoIoLessonWidget(parent);
    case 51:
        return new BackgroundSubtractionLessonWidget(parent);
    case 52:
        return new FrameDifferenceLessonWidget(parent);
    case 53:
        return new OpticalFlowLessonWidget(parent);
    case 54:
        return new CamshiftMeanshiftLessonWidget(parent);
    case 55:
        return new HomographyStitchingLessonWidget(parent);
    case 56:
        return new CameraCalibrationLessonWidget(parent);
    case 57:
        return new UndistortLessonWidget(parent);
    case 58:
        return new StereoVisionLessonWidget(parent);
    case 59:
        return new DnnInferenceLessonWidget(parent);
    case 60:
        return new OptimizationLessonWidget(parent);
    default:
        return nullptr;
    }
}
