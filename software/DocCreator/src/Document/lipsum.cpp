#include "lipsum.h"
#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QNetworkRequest>
#include <QThread>
#include <QUrl>

Lipsum::Lipsum(QObject *parent)
  : QObject(parent)
  , _replyS()
  , _qnam()
  , _reply(nullptr)

{}

void
Lipsum::getFile(const QUrl &url)
{
  qDebug() << "Get File" << url.host();

  _reply = _qnam.get(QNetworkRequest(url));
  connect(_reply, SIGNAL(finished()), this, SLOT(httpFinished()));

  connect(_reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
  connect(_reply,
          SIGNAL(error(QNetworkReply::NetworkError)),
          this,
          SLOT(error(QNetworkReply::NetworkError)));
  /*       connect(_reply, SIGNAL(downloadProgress(qint64, qint64)),
          this, SLOT(updateDataReadProgress(qint64, qint64)));*/

  QEventLoop loop(this);
  connect(_reply, SIGNAL(finished()), &loop, SLOT(quit()));
  loop.exec();
}

void
Lipsum::error(QNetworkReply::NetworkError e)
{
  qDebug() << "error : " << e;
}

void
Lipsum::httpFinished()
{
  _reply->deleteLater();
}

void
Lipsum::httpReadyRead()
{
  qDebug() << _reply->readAll();
}

QString
Lipsum::generate()
{
  return _replyS;
}
