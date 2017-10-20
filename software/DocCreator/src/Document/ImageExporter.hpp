#ifndef IMAGEEXPORTER_HPP
#define IMAGEEXPORTER_HPP

#include <QObject>
class QImage;

class ImageExporter : public QObject
{
  Q_OBJECT
public:
  explicit ImageExporter(const QString &filename, QObject *parent = 0);
  explicit ImageExporter(const QString &filename,
                         const QString &path,
                         QObject *parent = 0)
    : QObject(parent)
    , _filename(filename)
    , _path(path)
  {}

  static int nbImageWriten;

signals:

public slots:
  void writeImage(const QImage *image);

private:
  QString _filename;
  QString _path;
};

#endif // IMAGEEXPORTER_HPP
