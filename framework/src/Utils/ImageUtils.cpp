#include "ImageUtils.hpp"

#include <QImageReader>
#include <QImageWriter>
#include <QList>
#include <QObject>
#include <QVector>

#include <cassert>

QImage
toGray(const QImage &img)
{
  int w = img.width();
  int h = img.height();

  QImage outImg(w, h, QImage::Format_RGB32);

  const int depth = img.depth();

  //const QImage::Format format = img.format();
  if (depth == 32) {

    if (img.bytesPerLine() == w * 4 && outImg.bytesPerLine() == w * 4) {
      //data is continuous
      w *= h;
      h = 1;
    }

    for (int i = 0; i < h; ++i) {
      const QRgb *s = (const QRgb *)img.constScanLine(i);
      QRgb *d = (QRgb *)outImg.scanLine(i);

      for (int j = 0; j < w; ++j) {
        const int gray = qGray(*s);
        *d = qRgb(gray, gray, gray);
        ++d;
        ++s;
      }
    }

  }
#if QT_VERSION >= 0x050500 //Qt 5.5
  else if (depth == 8 && img.format() == QImage::Format_Grayscale8) {

    for (int i = 0; i < h; ++i) {
      const uchar *s = (const uchar *)img.constScanLine(i);
      QRgb *d = (QRgb *)outImg.scanLine(i);

      for (int j = 0; j < w; ++j) {
        const int gray = *s;
        *d = qRgb(gray, gray, gray);
        ++d;
        ++s;
      }
    }

  }
#endif //Qt 5.5
  else if (depth == 8 && img.format() == QImage::Format_Indexed8) {

    QVector<QRgb> colorTable = img.colorTable();
    const int colorTableSize = colorTable.size();
    for (int i = 0; i < colorTableSize; ++i) {
      const int gray = qGray(colorTable[i]);
      colorTable[i] = qRgb(gray, gray, gray);
    }

    if (img.bytesPerLine() == w && outImg.bytesPerLine() == w * 4) {
      //data is continuous
      w *= h;
      h = 1;
    }

    for (int i = 0; i < h; ++i) {
      const unsigned char *s = (const unsigned char *)img.constScanLine(i);
      QRgb *d = (QRgb *)outImg.scanLine(i);

      for (int j = 0; j < w; ++j) {
        assert(*s < colorTable.size());
        *d = colorTable.at(*s);
        ++d;
        ++s;
      }
    }
  } else {
    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x) {
        const int gray = qGray(img.pixel(x, y));
        outImg.setPixel(x, y, qRgb(gray, gray, gray));
      }
    }
  }

  return outImg;
}

QString
buildImageFilter(const QList<QByteArray> &list)
{

  QByteArray ba0;

  const int sz = list.size();

  if (sz > 0) {

    size_t tsz = 0;
    for (int i = 0; i < sz; ++i)
      tsz += list[i].size();

    size_t sz0 = tsz + (sz - 1) + sz * 2;
    ba0.reserve(sz0);

    for (int i = 0; i < sz - 1; ++i) {
      ba0.append("*.");
      ba0.append(list[i]);
      ba0.append(' ');
    }
    ba0.append("*.");
    ba0.append(list[sz - 1]);
  }

  QString filter(QObject::tr("Image (%1)").arg(QString(ba0)));

  return filter;
}

QString
getReadImageFilter()
{
  QList<QByteArray> list = QImageReader::supportedImageFormats();

  return buildImageFilter(list);
}

QString
getWriteImageFilter()
{
  QList<QByteArray> list = QImageWriter::supportedImageFormats();

  return buildImageFilter(list);
}

QStringList
buildImageFilterList(const QList<QByteArray> &list)
{
  QStringList l;
  const int sz = list.size();
  l.reserve(sz);
  for (int i = 0; i < sz; ++i) {
    const QString s = "*." + list[i];
    l << s;
  }
  return l;
}

QStringList
getReadImageFilterList()
{
  QList<QByteArray> list = QImageReader::supportedImageFormats();

  return buildImageFilterList(list);
}

QStringList
getWriteImageFilterList()
{
  QList<QByteArray> list = QImageWriter::supportedImageFormats();

  return buildImageFilterList(list);
}
