#ifndef DOCUMENTTOXMLEXPORTER_HPP
#define DOCUMENTTOXMLEXPORTER_HPP

#include <QObject>

class DocumentToXMLExporter : public QObject
{
  Q_OBJECT

public:
  static int _nb;

  explicit DocumentToXMLExporter(const QString &path, QObject *parent = 0)
    : QObject(parent)
    , _path(path)
    , _numberWidth(0)
  {}

  /*
    Width used to save number in filenames.
    If set to 0, no leading zeros will be added.

    For example for width=5, 193 will be written as 00193.
    For width=0, 193 will be written as 193.
   */
  void setNumberWidth(int width);

signals:

public slots:

  void toXML();

protected:
  QString _path;
  int _numberWidth;
};

#endif // DOCUMENTTOXMLEXPORTER_HPP
