#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>

class Application : public QObject
{
  Q_OBJECT
public:
  explicit Application(QObject *parent = 0);

signals:

public slots:

};

#endif // APPLICATION_H
