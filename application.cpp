#include "application.h"
#include <iostream>
std::string calibrationFile = "calibrationWiktor.xml";
std::string odometryFile = "/home/haskis/Odometria_Korytarz.txt";

Application::Application(QObject *parent) :
  QObject(parent),
  myCore(this)
{
  i = 0;
  pose = new double[24];
  myTcpSocket = NULL;
  myTcpServer = new QTcpServer(this);
  connect(this->myTcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
  myTcpServer->listen(QHostAddress::Any, 2000);

  delayTimer = new QTimer;
  connect(delayTimer,SIGNAL(timeout()),this,SLOT(continuousStream()));
}
void Application::newConnection(){
  qDebug()<<"New Connection";
  if(myTcpSocket != NULL)
  myTcpSocket->deleteLater();
  myTcpSocket = myTcpServer->nextPendingConnection();
  connect(this->myTcpSocket, SIGNAL(readyRead()), this, SLOT(interpretRequest()));
}
void Application::disconnected(){
  myTcpSocket->deleteLater();
  myTcpSocket = NULL;
}
void Application::interpretRequest(){
  myArray = myTcpSocket->readAll();
  //for(int i=0;i<myArray.size();i++)
    //qDebug()<<(quint8)myArray[i];


  if(myArray.size()<5)
    return;

  if((quint8)myArray[0] != START_BYTE ||
     (quint8)myArray[(myArray.size()-1)] != STOP_BYTE)
    return;

  quint8 length= (quint8)myArray[3];
  if((quint8)myArray.size() != 5+length)
    return;

  quint8 transactionID = (quint8)myArray[1];
  //Interpret data
  quint8 functionCode = (quint8)myArray[2];

  u_int8_t errorCode = EC_OK;
  switch (functionCode){
    case SET_PATTERN_SIZE:{
      if(length != 2)
        return;
      myCore.setPatternSize(cv::Size(quint8(myArray[4]),quint8(myArray[5])),errorCode);
      QByteArray response = formatIdleResponse(transactionID,functionCode,errorCode);
      myTcpSocket->write(response);
      myTcpSocket->flush();
      break;
    }

    case OPEN_VIDEO:{
      if(length != 0)
        return;
      openVideo(0, errorCode);
      QByteArray response = formatIdleResponse(transactionID,functionCode,errorCode);
      myTcpSocket->write(response);
      myTcpSocket->flush();
      break;
    }
    case GET_FRAME:{
      if(length != 0)
        return;
      updateFrame(errorCode);

      QByteArray response = formatIdleResponse(transactionID,functionCode,errorCode);

      myTcpSocket->write(response);
      myTcpSocket->flush();

      break;
    }
    case ADD_SAMPLE_TO_CALIBRATION:{
      if(length != 0)
        return;

      myCore.addSampleToCalibration(frame,errorCode);
      QByteArray response = formatIdleResponse(transactionID,functionCode,errorCode);
      myTcpSocket->write(response);
      myTcpSocket->flush();

      break;
    }
    case CALIBRATE_CAMERA:{
      if(length != 0)
        return;
      QByteArray response;
      if(myCore.calibrateCamera(calibrationFile,errorCode)){
          response = formatHeaderResponse(transactionID,functionCode,errorCode);

          char calibrationDataLength = 9 * sizeof(double);
          response.push_back((char)calibrationDataLength);

          for(int i=0;i<calibrationDataLength;i++)
            response.push_back(*(myCore.cameraMatrix.data +i));
          response.push_back(STOP_BYTE);
        }
      else
        response = formatIdleResponse(transactionID,functionCode,errorCode);
      myTcpSocket->write(response);
      myTcpSocket->flush();
      break;
    }
    case LOAD_CALIBRATION:{
      if(length != 0)
        return;
      QByteArray response;
      if(myCore.loadCalibration(calibrationFile,errorCode)){
          response = formatHeaderResponse(transactionID, functionCode, errorCode);
          char calibrationDataLength = 9 * sizeof(double);
          response.push_back((char)calibrationDataLength);

          for(int i=0;i<calibrationDataLength;i++)
            response.push_back(*(myCore.cameraMatrix.data +i));
          response.push_back(STOP_BYTE);
        }
      else
        response = formatIdleResponse(transactionID,functionCode,errorCode);
      myTcpSocket->write(response);
      myTcpSocket->flush();
      break;
    }
    case CREATE_VISO_OBJECT:{
        if(length != 0)
          return;
        myCore.createVisualOdometryMonoObject(errorCode);
        QByteArray response = formatIdleResponse(transactionID,functionCode,errorCode);
        myTcpSocket->write(response);
        myTcpSocket->flush();
        break;
      }
    case ADD_IMG_TO_ODOMETRY:{
      if(length != 0)
        return;
//      std::ofstream file;
//      file.open(odometryFile.data());
//      for(int i=1;i<67;i++){
//      char base_name[256];
//      sprintf(base_name,"%06d.JPG",i);
//      QString base = QString::fromStdString(base_name);
//      QString am = QString::fromStdString("/home/haskis/Pictures/Projects/Nasze/26_03/korytarz/") + base;
//      qDebug()<<am;
//      frame = cv::imread(am.toStdString(),CV_LOAD_IMAGE_GRAYSCALE);

//      myCore.addImgToOdometry(frame,errorCode,false);
//      file << myCore.pose<<std::endl;
//      }
      myCore.addImgToOdometry(frame,errorCode,false);
      QByteArray response = formatHeaderResponse(transactionID,functionCode,errorCode);

      char poseLength = 12 * sizeof(double);

      response.push_back((char)poseLength);
      myCore.pose.getData(pose);

      for(int i=0;i<poseLength;i++)
        response.push_back(*(reinterpret_cast<char*>(pose)+i));
      response.push_back(STOP_BYTE);

      myTcpSocket->write(response);
      myTcpSocket->flush();

      break;
    }
    case CONTINUOUS_STREAM:{
      if(length != 1)
        return;
      continuousStreamState = bool(myArray[4]);
      delayTimer->start();
      break;
    }
    case CHANGE_PARAMETERS:{
      if(length != 2*sizeof(double))
        return;
      double *pointer;
      pointer = reinterpret_cast<double *>(myArray.data()+4);
      double height = *pointer;
      double pitch = *(++pointer);
      myCore.changeInternalParameters(height,pitch, errorCode);
      QByteArray response = formatIdleResponse(transactionID,functionCode,errorCode);
      myTcpSocket->write(response);
      myTcpSocket->flush();
      break;
    }
    case RESET_POSE:{
      if(length != 0)
        return;
      myCore.resetPose();
      QByteArray response = formatIdleResponse(transactionID,functionCode,errorCode);
      myTcpSocket->write(response);
      myTcpSocket->flush();
      break;
    }
    default:{
      errorCode = NO_FUNCTION;
      QByteArray response = formatIdleResponse(transactionID,functionCode,errorCode);
      myTcpSocket->write(response);
      myTcpSocket->flush();
      break;
    }
  }
  qDebug()<<"After case";
}


bool Application::updateFrame(u_int8_t &errorCode){
  if(!myVideoCapture.isOpened()){
    errorCode = EC_NO_VIDEO ;
    return false;
    }
  myVideoCapture >> frame;
  myVideoCapture >> frame;
  myVideoCapture >> frame;
  myVideoCapture >> frame;
  myVideoCapture >> frame;

//  i++;
//  char base_name[256];
//  std::string path = "/home/haskis/Pictures/Projects/Nasze/26_03/kalibracja/";
//  sprintf(base_name,"%06d.JPG",i);
//  std::string actualImageURL  = path  + base_name;
//  std::cerr<<actualImageURL;
//  frame = cv::imread(actualImageURL,CV_LOAD_IMAGE_GRAYSCALE);

  qDebug()<<"Frame updated";
  return true;
}
bool Application::openVideo(int id, u_int8_t &errorCode){
  if(!myVideoCapture.open(id)){
    errorCode = EC_NO_VIDEO;
    return false;
  }
  else errorCode = EC_OK;
    return true;
}
QByteArray Application::formatIdleResponse(u_int8_t transactionID, u_int8_t functionCode, u_int8_t errorCode){
  QByteArray response;
  response.push_back(START_BYTE);
  response.push_back(transactionID);

  if(errorCode == EC_OK)
    response.push_back(functionCode);
  else
    response.push_back(functionCode + 0x80);

  response.push_back(errorCode);
  response.push_back((char)0);
  response.push_back(STOP_BYTE);

  return response;
}
QByteArray Application::formatHeaderResponse(u_int8_t transactionID, u_int8_t functionCode, u_int8_t errorCode){
  QByteArray response;

  response.push_back(START_BYTE);
  response.push_back(transactionID);

  if(errorCode == EC_OK)
    response.push_back(functionCode);
  else
    response.push_back(functionCode + 0x80);

  response.push_back(errorCode);

  return response;
}

void Application::continuousStream(){

  delayTimer->stop();
  quint8 errorCode = EC_OK;
  quint8 functionCode = CONTINUOUS_STREAM;
  if(!continuousStreamState)
    return;
  if(!myVideoCapture.isOpened()){
    errorCode = EC_NO_VIDEO;
    return ;
  }

  myVideoCapture >> frame;
  myVideoCapture >> frame;
  myVideoCapture >> frame;
  myVideoCapture >> frame;
  myVideoCapture >> frame;

  myCore.addImgToOdometry(frame,errorCode,false);

  QByteArray response = formatHeaderResponse(transactionIDStream,functionCode,errorCode);

  char poseLength = 12 * sizeof(double);

  response.push_back((char)poseLength);
  myCore.pose.getData(pose);

  for(int i=0;i<poseLength;i++)
    response.push_back(*(reinterpret_cast<char*>(pose)+i));
  response.push_back(STOP_BYTE);

  myTcpSocket->write(response);
  myTcpSocket->flush();

  delayTimer->start(20);
}
