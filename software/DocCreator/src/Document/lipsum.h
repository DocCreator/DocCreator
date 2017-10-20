#ifndef LIPSUM_H
#define LIPSUM_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class QUrl;

class Lipsum : public QObject
{
  Q_OBJECT

public:
  explicit Lipsum(QObject *parent = 0);

  QString generate();
  
  void getFile(const QUrl &url);



public slots :

  void httpFinished();
  void httpReadyRead();
  void error(QNetworkReply::NetworkError e);
  
private :
  QString _replyS;
  QNetworkAccessManager _qnam;
  QNetworkReply *_reply;

};

#endif // LIPSUM_H
