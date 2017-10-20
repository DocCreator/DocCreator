#include "Lipsum4Qt.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDomDocument>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

const QUrl Lipsum4Qt::WEBSERVICE_URL(
  QStringLiteral("http://lipsum.com/feed/xml"));

const QString Lipsum4Qt::Lipsum = QStringLiteral(
  "Lorem ipsum dolor sit amet, consectetur adipiscing elit. In semper massa "
  "sapien, id pulvinar justo elementum tincidunt. Cras vel semper enim. "
  "Integer tortor nisl, maximus eget dolor eu, pulvinar iaculis metus. Nulla "
  "malesuada convallis nisi, sed dictum mauris gravida quis. Sed a dui nisl. "
  "Nullam gravida quam quis dapibus vehicula. Proin in sapien orci. Vivamus et "
  "placerat nibh. Aliquam erat volutpat. Cum sociis natoque penatibus et "
  "magnis dis parturient montes, nascetur ridiculus mus. \n Suspendisse "
  "rhoncus nibh sed dui dapibus efficitur. Morbi tempor egestas condimentum. "
  "Nam sit amet dolor lacus. Cras a lectus pulvinar, tempor neque eleifend, "
  "pharetra felis. Proin gravida ex justo, nec placerat mauris sollicitudin "
  "in. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per "
  "inceptos himenaeos. Cras non tellus in sem eleifend rutrum vitae et ipsum. "
  "Nulla in nisi sollicitudin, hendrerit urna imperdiet, rutrum purus. Sed "
  "fermentum felis bibendum lorem porta sodales. Class aptent taciti sociosqu "
  "ad litora torquent per conubia nostra, per inceptos himenaeos. Aliquam "
  "elementum arcu in sapien sollicitudin, vitae lobortis justo "
  "venenatis.\nCurabitur sit amet est ac libero varius consequat. Sed auctor "
  "lobortis pharetra. Nam feugiat, eros at facilisis rhoncus, lectus ligula "
  "blandit est, sit amet volutpat diam massa eget massa. Nunc mi nulla, "
  "accumsan sit amet erat in, dapibus porta est. Aliquam aliquam malesuada "
  "venenatis. Donec elementum nibh vitae elit vehicula placerat. Fusce mattis, "
  "tellus vitae iaculis dignissim, quam nunc posuere neque, at porta leo odio "
  "tincidunt erat. Fusce eleifend malesuada elit at pellentesque. Nam feugiat "
  "felis quis turpis posuere, in varius erat pharetra. In aliquam fringilla "
  "lectus, id ultricies lacus. Interdum et malesuada fames ac ante ipsum "
  "primis in faucibus.\nSed porttitor nec dolor id pellentesque. Donec "
  "faucibus, ex in porta consequat, felis tortor volutpat quam, in egestas "
  "quam tortor et lacus. Maecenas rutrum porttitor vehicula. Interdum et "
  "malesuada fames ac ante ipsum primis in faucibus. Donec sed pretium odio. "
  "Nullam odio nunc, hendrerit quis pharetra nec, porttitor vitae turpis. "
  "Vestibulum ullamcorper, massa vel feugiat viverra, purus mi facilisis "
  "magna, nec ullamcorper mi turpis in lectus. Vivamus rutrum iaculis leo, at "
  "laoreet erat imperdiet a. Aliquam facilisis hendrerit massa, eu maximus "
  "ligula luctus sit amet. Proin sed neque et libero congue dapibus eget ut "
  "massa. Vestibulum condimentum tellus arcu. Suspendisse ultrices tellus vel "
  "metus auctor porttitor. Integer et ante in purus facilisis ullamcorper. Sed "
  "tristique metus ut dolor tincidunt commodo placerat eget eros. Donec et "
  "dolor augue.\nSed scelerisque porta magna dapibus consectetur. Pellentesque "
  "eu purus mi. Aenean porta sagittis porttitor. Mauris finibus enim vel purus "
  "sollicitudin rutrum. Fusce condimentum pharetra imperdiet. Nam nec leo "
  "placerat, dictum mi et, sagittis tellus. Maecenas vulputate, urna rhoncus "
  "congue elementum, orci mi fringilla neque, ut ultrices ante purus nec "
  "dolor. Vestibulum auctor interdum interdum. Curabitur vel tellus et augue "
  "venenatis pulvinar sit amet eu urna. Donec ac venenatis sem. Vivamus in "
  "elit iaculis, congue sapien ut, lacinia neque. Maecenas eget tellus eu nibh "
  "placerat posuere sed vitae nisi. Cras sed massa porta diam tempus malesuada "
  "id a nisi. Quisque tempus justo non metus mattis, eget malesuada elit "
  "interdum. Suspendisse eget eros sollicitudin, ornare neque sit amet, "
  "sodales nulla. Suspendisse potenti.\nSuspendisse rhoncus pretium congue. "
  "Mauris ultrices leo libero. Ut ac volutpat eros. Proin at porta augue, ac "
  "vehicula metus. Praesent sit amet dictum arcu. Aliquam sit amet tellus eget "
  "magna placerat sollicitudin a placerat nulla. Phasellus porttitor, elit non "
  "varius faucibus, velit orci aliquam magna, sed aliquet sapien lorem a leo. "
  "Duis posuere mi nunc, et mattis ligula pellentesque non. Praesent imperdiet "
  "risus in massa ullamcorper, nec molestie libero pretium.\nSed varius at "
  "arcu a iaculis. Proin a ex vel mi condimentum scelerisque sed a nunc. Donec "
  "lacinia, nulla condimentum interdum suscipit, libero enim lobortis felis, "
  "ac aliquet sapien velit quis odio. Fusce sit amet sem eget velit fringilla "
  "imperdiet. Curabitur vestibulum turpis accumsan dapibus hendrerit. Integer "
  "nec nisl accumsan, rutrum augue sit amet, convallis nulla. Vestibulum ante "
  "ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; "
  "Quisque vitae egestas eros, a facilisis ipsum.\nSuspendisse placerat "
  "molestie massa, a luctus diam pulvinar in. Donec eu vestibulum magna. Morbi "
  "mollis nulla purus, in eleifend magna vulputate at. Maecenas faucibus eros "
  "non nibh sollicitudin, non rutrum diam porta. Curabitur dapibus nisl elit, "
  "quis vestibulum quam molestie tristique. Donec volutpat enim ac nunc "
  "sollicitudin, ut vestibulum erat auctor. Integer finibus sapien iaculis "
  "eros scelerisque sollicitudin. Sed viverra, urna at sagittis faucibus, orci "
  "tortor venenatis nunc, sed vehicula quam risus vitae dui. Quisque sed "
  "vestibulum quam, eget pulvinar lorem. Class aptent taciti sociosqu ad "
  "litora torquent per conubia nostra, per inceptos himenaeos. Nullam euismod "
  "dictum enim, sit amet auctor metus scelerisque id. Donec ut libero ac magna "
  "blandit volutpat. Aenean sit amet lectus dui. Nulla ac auctor eros.");

Lipsum4Qt::Lipsum4Qt(CONNECTION_MODE mode, QObject *object)
  : QObject(object)
  , _repliedLipsum()
  , _qnam()
  , _reply(nullptr)
{
  _reply = _qnam.get(QNetworkRequest(WEBSERVICE_URL));
  if (_reply->error() != QNetworkReply::NoError) {
    qDebug() << "Lipsum4Qt::Lipsum4Qt error during request "
             << static_cast<int>(_reply->error());
  }
  connect(_reply, SIGNAL(finished()), this, SLOT(httpFinished()));

  connect(_reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));

  if (mode == SYNC) {
    QEventLoop loop(this);
    connect(_reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
  }
}

void
Lipsum4Qt::httpFinished()
{
  _reply->deleteLater();
}

void
Lipsum4Qt::httpReadyRead()
{
  QDomDocument doc;
  QByteArray content = _reply->readAll();
  doc.setContent(content);

  QDomElement root = doc.documentElement();
  QDomElement child = root.firstChild().toElement();

  bool found = false;
  while (!child.isNull() && !found) {
    if (child.tagName() == QLatin1String("lipsum")) {
      _repliedLipsum.append(child.text());
      found = true;
    }

    child = child.nextSibling().toElement();
  }
}

QString
Lipsum4Qt::generate()
{
  return _repliedLipsum;
}
