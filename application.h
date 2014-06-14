#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDataStream>
#include <core.h>
#include <functioncodes.h>
#include <errorcodes.h>
#include <string>
#include <QTimer>
class Application : public QObject
{
  Q_OBJECT
public:
  explicit Application(QObject *parent = 0);

signals:

private:
  QTcpSocket *myTcpSocket;
  QTcpServer *myTcpServer;

  QByteArray myArray;
  Core myCore;

  cv::VideoCapture myVideoCapture;
  cv::Mat frame;
  double *pose;
  int i;
  QTimer *delayTimer;

  bool openVideo(int id, u_int8_t &errorCode);
  bool updateFrame(u_int8_t &errorCode);

private slots:
  void disconnected();
  void newConnection();
  void interpretRequest();
  void continuousStream();
  QByteArray formatIdleResponse(u_int8_t transactionID, u_int8_t functionCode, u_int8_t errorCode);
  QByteArray formatHeaderResponse(u_int8_t transactionID, u_int8_t functionCode, u_int8_t errorCode);
private:
  bool continuousStreamState;
  bool transactionIDStream;
};

#endif // APPLICATION_H
