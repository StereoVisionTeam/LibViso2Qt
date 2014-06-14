#include "core.h"

using namespace cv;
using namespace std;

Core::Core(QObject *parent) :
  QObject(parent),
  pose(Matrix::eye(4))
{
  calibrationSamplesCounter = 0;
  goodSamplesCounter = 0;
  isCalibrationDone = false;
  minimumSamplesForCalibration = 6;
  minimumPatternSizeWidth = 2;
  minimumPatternSizeHeight = 2;
  squareSize = 0.1;
  isSetPatternSizeDone = false;
  viso = NULL;
}

bool Core::addSampleToCalibration(Mat &calibrationImage, u_int8_t &errorCode){

  if(!isSetPatternSizeDone){
    errorCode = EC_NO_PATTERN_SIZE ;
    return false;
  }
  if(calibrationImage.data == NULL){
      errorCode = EC_NO_IMAGE;
      return false;
    }
  imageSize = calibrationImage.size();
    cv::vector<Point2f> corners;
    bool result;
    if(calibrationImage.channels()!=1)
      cvtColor(calibrationImage,calibrationImage, CV_RGB2GRAY);
    result = findChessboardCorners(calibrationImage,
                                   patternSize,
                                   corners,
                                   CALIB_CB_ADAPTIVE_THRESH+CALIB_CB_NORMALIZE_IMAGE );

  //We accept only samples with full visibility of corners
  if(!result){
    errorCode = EC_BAD_CORNERS ;
    return false;
  }
  goodSamplesCounter++;

  //If we found corners then we need to do some subpix corners interpolation
  cornerSubPix(calibrationImage, corners, Size(11, 11), Size(-1, -1),
               TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

  imagePoints.push_back(corners);
  errorCode = EC_OK ;
  return true;
}
bool Core::calibrateCamera(std::string outputURL, u_int8_t &errorCode){

  if(goodSamplesCounter<minimumSamplesForCalibration){
    errorCode = EC_NOT_ENOUGHT_SAMPLES;
    return false;
  }

  objectPoints.clear();
  //Assign values to objectPoints in object coordinate space
  for(int i = 0; i <goodSamplesCounter ; i++ ){
    objectPoints.push_back(vector<Point3f>());
      for(int j = 0; j < patternSize.height; j++ )
        for(int k = 0; k < patternSize.width; k++ )
          objectPoints[i].push_back(Point3f(float(j*squareSize), float(k*squareSize), 0));
  }

  vector<Mat> rvec, tvec;

  isCalibrationDone = cv::calibrateCamera(objectPoints,imagePoints,imageSize,cameraMatrix,distCoeffs,rvec,tvec);

  if(isCalibrationDone){
    if(!saveCalibration(outputURL, errorCode))
    return false;

    errorCode = EC_OK;
    return true;
  }
  errorCode = EC_OPENCV_BAD_CALIBRATION ;
  return false;
}
bool Core::calibrateFromImages(std::string inputImagesPathURL , std::string outputCalibrationDataURL, int numberOfSamples, u_int8_t &errorCode){

  //Kalibracja z pliku
  for(int i=1; i<numberOfSamples;i++)
  {
    cv::Mat image;
    char base_name[256];
    sprintf(base_name,"%06d.JPG",i);
    string actualImageURL  = inputImagesPathURL +"/" + base_name;

    image = cv::imread(actualImageURL,CV_LOAD_IMAGE_GRAYSCALE);
    if(!image.data ){
      errorCode = EC_NO_IMAGE ;
      return false;
    }
    addSampleToCalibration(image, errorCode);
  }
  return calibrateCamera(outputCalibrationDataURL, errorCode);
}

bool Core::saveCalibration(std::string outputURL, u_int8_t &errorCode){

  cv::FileStorage outputFile(outputURL, FileStorage::WRITE);
  if(!outputFile.isOpened()){
    errorCode = EC_BAD_FILE_PATH ;
    return false;
  }
  outputFile<<"cameraMatrix"<<cameraMatrix;
  outputFile.release();
  errorCode = EC_OK;
  return true;
}

bool Core::loadCalibration(std::string inputURL, u_int8_t &errorCode){

    cv::FileStorage inputFile(inputURL, FileStorage::READ);
    if(!inputFile.isOpened()){
        errorCode = EC_BAD_FILE_PATH;
        return false;
    }
    inputFile["cameraMatrix"] >> cameraMatrix;
    isCalibrationDone = true;
    errorCode = EC_OK ;
    return true;
}

bool Core::addImgToOdometry(cv::Mat img, u_int8_t &errorCode, bool replace){

  if(viso == NULL){
    errorCode = EC_VISO_IS_NULL;
    return false;
  }

  if(!isCalibrationDone){
    errorCode = EC_NO_CALIBRATION;
    return false;
  }

  // image dimensions
  int32_t width= img.cols;
  int32_t height= img.rows;
  int32_t dims[] = {width,height,width};

  // compute visual odometry

  if (viso->process(img.datastart, dims,errorCode, replace)) {
    pose = pose* Matrix::inv(viso->getMotion());
    errorCode = EC_OK;

    return true;
  }
  std::cout<<"INLIERS"<<viso->getNumberOfInliers();
  std::cout<<"MATCHES"<<viso->getNumberOfMatches();
  return false;
}

bool Core::setPatternSize(cv::Size x, u_int8_t &errorCode){
  if(x.height>=minimumPatternSizeHeight && x.width>=minimumPatternSizeWidth){
    patternSize = x;
    isSetPatternSizeDone = true;
    errorCode = EC_OK;
    return true;
  }
  errorCode = EC_BAD_SIZE;
  return false;
}
bool Core::createVisualOdometryMonoObject(u_int8_t &errorCode){
  if(isCalibrationDone){
    VisualOdometryMono::parameters param;

    param.calib.f  = cameraMatrix.at<double>(0); // focal length in pixels
    param.calib.cu = cameraMatrix.at<double>(2); // principal point (u-coordinate) in pixels
    param.calib.cv = cameraMatrix.at<double>(5); // principal point (v-coordinate) in pixels

    if(viso != NULL)
      delete viso;
    viso=new VisualOdometryMono(param);
    resetPose();

    errorCode = EC_OK ;
    return true;
  }
  errorCode = EC_NO_CALIBRATION ;
  return false;
}
void Core::changeInternalParameters(double height,
                                    double pitch,
                                    u_int8_t &errorCode){
  if(viso == NULL){
    errorCode = EC_VISO_IS_NULL;
    return;
  }
  qDebug()<<"Height"<<height;
  qDebug()<<"Pitch"<<pitch;
  viso->param.height = height;
  viso->param.pitch = pitch;
  errorCode = EC_OK;
}
void Core::resetPose(){
  pose = Matrix::eye(4);
}
