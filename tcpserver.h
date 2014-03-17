#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTCPServer>

class TCPServer : public QTCPServer
{
  Q_OBJECT
public:
  explicit TCPServer(QObject *parent = 0);

signals:

public slots:

};

#endif // TCPSERVER_H
