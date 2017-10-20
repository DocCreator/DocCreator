#include "ImageExporter.hpp"

#include <QImageWriter>

int ImageExporter::nbImageWriten = 0;

ImageExporter::ImageExporter(const QString &filename, QObject *parent)
  : QObject(parent)
  , _filename(filename)
  , _path(QStringLiteral("."))

{}

void
ImageExporter::writeImage(const QImage *image)
{
  nbImageWriten++;
  QString filename(_path);
  filename.append("/");
  filename.append(_filename);
  filename.append("_").append(QString::number(nbImageWriten)).append("_");
  filename.append(".png");
  QImageWriter writer(filename, "png");
  writer.write(*image);
}
