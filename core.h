#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <fstream>

#include "libviso2/viso_mono.h"
#include "libviso2/viso_stereo.h"
#include <errorcodes.h>

class Core : public QObject
{
  Q_OBJECT
public:
  explicit Core(QObject *parent = 0);

  Matrix pose;

//Calibration
  bool addSampleToCalibration(cv::Mat &calibrationImage, u_int8_t &errorCode);
  bool calibrateCamera(std::string outputURL, u_int8_t &errorCode);
  bool setPatternSize(cv::Size x, u_int8_t &errorCode);
  void setsquareSize(double x){ squareSize= x;}
  bool saveCalibration(std::string outputURL, u_int8_t &errorCode);
  bool loadCalibration(std::string inputURL, u_int8_t &errorCode);

  bool calibrateFromImages(std::string inputURL,
                           std::string outputCalibrationDataURL,
                           int numberOfSamples,
                           u_int8_t &errorCode);

//Odometry
  bool addImgToOdometry(cv::Mat img, u_int8_t &errorCode, bool replace=false);
  bool createVisualOdometryMonoObject(u_int8_t &errorCode);

//Parameters
  void changeInternalParameters(double height,           // camera height above ground (meters)
                                double pitch, u_int8_t &errorCode,            // camera pitch (rad, negative=pointing down)
                                int32_t ransac_iters = 2000,     // number of RANSAC iterations
                                double inlier_threshold = 0.00001, // fundamental matrix inlier threshold
                                double motion_threshold = 100.0); // directly return false on small motions

  void resetPose();

  cv::Mat cameraMatrix, distCoeffs;
private:

  std::ofstream fileWithOdometry;
  int calibrationSamplesCounter;
  int goodSamplesCounter;
  bool isCalibrationDone;

  int minimumSamplesForCalibration; //default =8
  int minimumPatternSizeWidth; //default =2
  int minimumPatternSizeHeight; //default =2
  double squareSize; //size of squares in calibration chessboard;
  cv::vector<cv::vector<cv::Point3f> > objectPoints;
  cv::Size patternSize; //Number of inner corners per a chessboard row and column for (8x8) board it's (7x7)
  bool isSetPatternSizeDone;
  //cv::Mat cameraMatrix, distCoeffs;
  cv::vector<cv::vector<cv::Point2f> > imagePoints;
//Odometry
  VisualOdometryMono *viso;

  cv::Size imageSize;
};

#endif // CORE_H
