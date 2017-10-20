#ifndef LIPSUM4QT_HPP
#define LIPSUM4QT_HPP

#include "Lipsum4Qt_global.h"
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

class QNetworkReply;
class QUrl;

class LIPSUM4QTSHARED_EXPORT Lipsum4Qt : public QObject
{
  Q_OBJECT

public:
  const static QUrl WEBSERVICE_URL;
  const static QString Lipsum;

  enum CONNECTION_MODE
  {
    SYNC,
    ASYNC
  };

  Lipsum4Qt(CONNECTION_MODE mode, QObject *object = NULL);

  QString generate();

protected slots:
  void httpFinished();
  void httpReadyRead();

private:
  QString _repliedLipsum;
  QNetworkAccessManager _qnam;
  QNetworkReply *_reply;
};

#endif // LIPSUM4QT_HPP
