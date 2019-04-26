#include "BleedThrough.hpp"
#include <QDebug>
#include <QProgressDialog>
#include <QRgb>
#include <QThread>
#include <QVector>
#include <cassert>
#include <iostream>
#include <vector>

/*
  Code inspired from original Cuong's code.

  This is a stencil operation, it could be coded to be faster !
  cf TestBleedThrough directory, for some tests

 */

static inline int
bleedThrough_kernel(int u, int current, int ixp, int ixn, int iyp, int iyn)
{
  const float dt = 0.05f; //B: ?????  //Why isn't it a parameter of the algo ?

  //B: remove useless: / 100.0f * 100.0f
  const float c =
    (1.0f /
     (1.0f + (ixn - u) * (ixn - u))) /* (1.0 / ((1.0 + ixn*ixn)/(0.2*0.2)) )*/;
  const float c1 =
    (1.0f /
     (1.0f + (ixp - u) * (ixp - u))) /* (1.0 / ((1.0 + ixp*ixp)/(0.2*0.2)) )*/;
  const float c2 =
    (1.0f /
     (1.0f + (iyp - u) * (iyp - u))) /* (1.0 / ((1.0 + iyp*iyp)/(0.2*0.2)) )*/;
  const float c3 =
    (1.0f /
     (1.0f + (iyn - u) * (iyn - u))) /* (1.0 / ((1.0 + iyn*iyn)/(0.2*0.2)) )*/;

  const float delta = (c * (ixn - current) + c1 * (ixp - current) +
                       c2 * (iyp - current) + c3 * (iyn - current));

  const int g = static_cast<int>(dt * delta + current);
  //B: later casted in uchar. Should we use a cv::saturate_cast ?

  return g;
}

static void
convertToAtLeast8Bit(QImage &src)
{
  if (src.depth() < 8) {
#ifndef NDEBUG
    const QSize s = src.size();
#endif //NDEBUG
    src = src.convertToFormat(QImage::Format_Indexed8);
    assert(src.size() == s);
  }
  assert(src.depth() >= 8);
}

//Qt 8-bit images are indexed ! We cannot use them directly !
//Thus we use a simple vector of uchar to store 8-bit grayscale images.
//REM: In Qt 5.5 there is now QImage::Format_Grayscale8
static std::vector<uchar>
getGray(const QImage &psrc)
{
  QImage src = psrc;

  int w = src.width();
  int h = src.height();

  std::vector<uchar> res(w * h);

  convertToAtLeast8Bit(src);
  int depth = src.depth();

  if (depth == 32) {

    const uchar *b = src.constBits();
    const int b_shift = src.bytesPerLine() - 4 * w;

    if (b_shift == 0) {
      w *= h;
      h = 1;
    }

    int k = 0;
    for (int i = 0; i < h; ++i) {
      for (int j = 0; j < w; ++j) {
        res[k] = qGray(*(unsigned int *)(b));
        b += 4;
        ++k;
      }
      b += b_shift;
    }

  }
#if QT_VERSION >= 0x050500 //Qt 5.5
  else if (depth == 8 && src.format() == QImage::Format_Grayscale8) {

    const uchar *b = src.constBits();
    const int b_shift = src.bytesPerLine() - w;

    if (b_shift == 0) {
      memcpy(&res[0], b, (size_t)w * (size_t)h);
    } else {
      int k = 0;
      for (int i = 0; i < h; ++i) {
        memcpy(&res[k], b, w);
        b += w + b_shift;
        k += w;
      }
    }

  }
#endif //Qt 5.5
  else if (depth == 8 && src.format() == QImage::Format_Indexed8) {

    QVector<QRgb> colorTable = src.colorTable();
    const size_t s = colorTable.size();
    std::vector<uchar> colorTableGray(s);
    for (size_t i = 0; i < s; ++i) {
      colorTableGray[i] = static_cast<uchar>(qGray(colorTable[i]));
    }

    const uchar *b = src.constBits();
    const int b_shift = src.bytesPerLine() - w;
    if (b_shift == 0) {
      w *= h;
      h = 1;
    }
    size_t k = 0;
    for (int i = 0; i < h; ++i) {
      for (int j = 0; j < w; ++j) {
        const uint index = *b;
        assert(index < colorTableGray.size());
        assert(k < res.size());
        res[k] = colorTableGray[index];
        ++b;
        ++k;
      }
      b += b_shift;
    }

  } else {
    std::cerr << "getGray(): unhandled format (depth=" << depth << ")\n";
    std::cerr << "  format=" << static_cast<int>(src.format()) << "\n";
    std::cerr << "  Format_Mono=" << static_cast<int>(QImage::Format_Mono)
              << "\n";
    std::cerr << "  Format_Indexed8="
              << static_cast<int>(QImage::Format_Indexed8) << "\n";
    assert(false);
  }

  return res;
}

class BleedThroughDiffusionThreadA : public QThread
{
public:
  BleedThroughDiffusionThreadA(const std::vector<uchar> &originalRectoGray,
                               const std::vector<uchar> &rectoGray,
                               const std::vector<uchar> &versoGray,
                               int width,
                               int height,
                               QPoint xy,
                               QPoint wh);

  void getRecto(std::vector<uchar> &outGray) const;

protected:
  void run() override;

protected:
  const std::vector<uchar> &_originalRectoGray;
  const std::vector<uchar> &_rectoGray;
  const std::vector<uchar> &_versoGray;
  int _width;
  int _height;
  int _x, _y, _w, _h; //block coordinates
  std::vector<uchar> _outGray;
  int _shift;
};

BleedThroughDiffusionThreadA::BleedThroughDiffusionThreadA(
  const std::vector<uchar> &originalRectoGray,
  const std::vector<uchar> &rectoGray,
  const std::vector<uchar> &versoGray,
  int width,
  int height,
  QPoint xy,
  QPoint wh)
  : _originalRectoGray(originalRectoGray)
  , _rectoGray(rectoGray)
  , _versoGray(versoGray)
  , _width(width)
  , _height(height)
  , _x(xy.x())
  , _y(xy.y())
  , _w(wh.x())
  , _h(wh.y())
  , _outGray(originalRectoGray.size())
  , _shift(width - wh.x())
{
  assert(_w <= _width);
  assert(_h <= _height);
  assert(_originalRectoGray.size() == (size_t)(_width * _height));
  assert(_rectoGray.size() == (size_t)(_width * _height));
  assert(_versoGray.size() == (size_t)(_width * _height));
  assert(_shift >= 0);
  assert(_shift < _width);
}

void
BleedThroughDiffusionThreadA::getRecto(std::vector<uchar> &outGray) const
{
  assert(outGray.size() == (_width * (size_t)_height));

  const uchar *s = &_outGray[0];
  uchar *d = &outGray[_y * (size_t)_width + _x];

  for (int x = _x; x < _w; ++x) {
    for (int y = _y; y < _h; ++y) {
      *d = *s;
      ++d;
      ++s;
    }
    d += _shift;
  }
}

void
BleedThroughDiffusionThreadA::run() //bleedThrough4b
{
  const int width = _width;
  const int height = _height;
  const int shift = _shift;

  const size_t start = _y * (size_t)width + _x;
  assert(start < _versoGray.size());
  const uchar *v = &_versoGray[0];
  const uchar *sr = &_originalRectoGray[0] + start;
  const uchar *r = &_rectoGray[0] + start;
  uchar *o = &_outGray[0];

  for (int y = _y; y < _h; ++y) {
    for (int x = _x; x < _w; ++x) {
      //const int current = qGray(_recto->pixel(x, y));
      const int current = *r;
      ++r;

      const int px = (x > 0 ? x - 1 : 0);
      const int nx = (x < width - 1 ? x + 1 : width - 1);
      const int py = (y > 0 ? y - 1 : 0);
      const int ny = (y < height - 1 ? y + 1 : height - 1);

      const int iyn = v[py * width + x];
      const int ixp = v[y * width + px];
      const int ixn = v[y * width + nx];
      const int iyp = v[ny * width + x];

      //const int u = qGray(_originalRecto->pixel(x, y));
      const int u = *sr;
      ++sr;

      const int g = bleedThrough_kernel(u, current, ixp, ixn, iyp, iyn);

      *o = g;
      ++o;
    }

    r += shift;
    sr += shift;
  }
}

static inline QImage
makeRGBfromGray(int width, int height, const std::vector<uchar> &gray)
{
  assert(gray.size() == (size_t)width * height);
  QImage out(width, height, QImage::Format_RGB32);
  assert(out.depth() == 32);
  uchar *o = out.bits();
  const int o_shift = out.bytesPerLine() - 4 * width;
  const uchar *r = &gray[0];
  if (o_shift == 0) {
    width *= height;
    height = 1;
  }
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const int g = *r;
      *(unsigned int *)(o) = qRgb(g, g, g);
      ++r;
      o += 4;
    }
    o += o_shift;
  }
  return out;
}

static QImage
bleedThroughMT0(const QImage &originalRecto,
                const QImage &imgRecto,
                const QImage &imgVerso,
                int nbIter,
                int nbThreadsPerRow = 4)
{
  assert(imgRecto.size() == imgVerso.size());
  assert(originalRecto.size() == imgVerso.size());

  const int width = imgRecto.width();
  const int height = imgRecto.height();
  const std::vector<uchar> originalRectoGray = getGray(originalRecto);
  const std::vector<uchar> versoGray = getGray(imgVerso);
  std::vector<uchar> rectoGray = getGray(imgRecto);

  if (originalRectoGray.empty() || versoGray.empty() || rectoGray.empty())
    return QImage();

  std::vector<uchar> outGray(width * height);

  //B: divide only lines (for cache)

  const int nbThreadsPerCol = 1;
  //const int nbThreadsPerRow = 16;
  nbThreadsPerRow = (nbThreadsPerRow > 0 ? nbThreadsPerRow : 1);

  const int blockWidth = (width) / nbThreadsPerCol;
  const int blockHeight = (height) / nbThreadsPerRow;

  //std::cerr<<"blockWidth="<<blockWidth<<" blockHeight="<<blockHeight<<" nbThreadsPerRow="<<nbThreadsPerRow<<"\n";

  //B:TODO:OPTIM: this is a stencil operation !
  //We can code it with some communication/synchronization between threads (for borders) to be be much faster !
  //We would avoid to create/run/destroy the threads each time, and keep them alive during the nbIter iterations !

  QVector<BleedThroughDiffusionThreadA *> threads;
  threads.reserve(nbThreadsPerCol * nbThreadsPerRow);

  const int lNbIter = nbIter;
  for (int p = 0; p < lNbIter; ++p) {

    for (int l = 0; l < nbThreadsPerRow; ++l) {
      const int y = l * blockHeight;
      const int h = (l != nbThreadsPerRow - 1 ? blockHeight : height - y);
      for (int k = 0; k < nbThreadsPerCol; ++k) {
        const int x = k * blockWidth;
        const int w = (k != nbThreadsPerCol - 1 ? blockWidth : width - x);

        BleedThroughDiffusionThreadA *thread1 =
          new BleedThroughDiffusionThreadA(originalRectoGray,
                                           rectoGray,
                                           versoGray,
                                           width,
                                           height,
                                           QPoint(x, y),
                                           QPoint(x + w, y + h));
        thread1->start();
        threads.push_back(thread1);
      }
    }

    for (BleedThroughDiffusionThreadA *thread : threads) {
      thread->wait();
      thread->getRecto(outGray);
      delete thread;
    }
    threads.clear();

    outGray.swap(rectoGray);
  }

  QImage out = makeRGBfromGray(width, height, rectoGray);

  return out;
}

/*
static
QImage
bleedThroughMT0(const QImage &imgRecto, const QImage &imgVerso, int nbIter, int nbThreadsPerRow=4)
{
  return bleedThroughMT0(imgRecto.copy(), imgRecto, imgVerso, nbIter, nbThreadsPerRow);
}
*/

static void
copyImagePart(QImage &dst,
              int x0_dst,
              int y0_dst,
              const QImage &src,
              int x0_src,
              int y0_src,
              int w,
              int h)
{
  assert(dst.depth() >= 8);
  assert(src.depth() >= 8);

  assert(x0_dst >= 0 && x0_dst + w <= dst.width());
  assert(y0_dst >= 0 && y0_dst + h <= dst.height());
  assert(x0_src >= 0 && x0_src + w <= src.width());
  assert(y0_src >= 0 && y0_src + h <= src.height());

  const int src_depth = src.depth();
  const int dst_depth = dst.depth();

  if (dst_depth == 32 && src_depth == 32) {

    int y_src = y0_src;
    int y_dst = y0_dst;
    for (int y = 0; y < h; ++y) {
      const QRgb *s = (const QRgb *)src.constScanLine(y_src) + x0_src;
      QRgb *d = (QRgb *)dst.scanLine(y_dst) + x0_dst;
      for (int x = 0; x < w; ++x) {
        d[x] = s[x];
      }
      ++y_src;
      ++y_dst;
    }

  } else {
    assert(src.format() == QImage::Format_Indexed8);
    assert(dst.format() == QImage::Format_Indexed8);

    std::vector<uint> src2dst(src.colorCount(), 0);
    for (size_t i = 0; i < src2dst.size(); ++i)
      src2dst[i] = i;

    if (dst.colorCount() < 256) {

      bool changed = false;
      QVector<QRgb> src_colorTable = src.colorTable();

      assert(src2dst.size() <= (size_t)src_colorTable.size());

      QVector<QRgb> dst_colorTable = dst.colorTable();
      for (int i = 0; i < src_colorTable.count(); ++i) {
        const QRgb c = src_colorTable[i];
        if (dst_colorTable.count() >= 256)
          break;
        bool found = false;
        uint ind = dst_colorTable.count();
        for (int j = 0; j < dst_colorTable.count(); ++j) {
          if (dst_colorTable[j] == c) {
            found = true;
            ind = j;
            break;
          }
        }
        if (found) {
          src2dst[i] = ind;
        } else {
          dst_colorTable.push_back(c);
          src2dst[i] = dst_colorTable.size() - 1;
          changed = true;
        }
        assert(src2dst[i] < (size_t)dst_colorTable.size());
      }
      if (changed)
        dst.setColorTable(dst_colorTable);
    }

    int y_src = y0_src;
    int y_dst = y0_dst;
    for (int y = 0; y < h; ++y) {

      assert(y_src >= 0 && y_src < src.height());
      assert(y_dst >= 0 && y_dst < dst.height());

      const uchar *s = (const uchar *)src.constScanLine(y_src) + x0_src;
      uchar *d = (uchar *)dst.scanLine(y_dst) + x0_dst;
      for (int x = 0; x < w; ++x) {

        const uint src_index = s[x];
        assert(src_index <= src2dst.size());
        const uint dst_index = src2dst[src_index];
        assert(dst_index <= (size_t)dst.colorCount());
        d[x] = dst_index;
      }
      ++y_src;
      ++y_dst;
    }
  }
}

/*
  Get image corresponding of the overlapping of @a img at position (@a x, @a y) on an original image of size @a width x @a height,
  at position (0, 0).

  (@a x, @a y) is the position of image relative to the original image at position (0, 0).
  Thus @a x and/or @a y may be negative.
 */
static QImage
getOverlappingPart(int width, int height, const QImage &img, int x, int y)
{
  if (img.width() == width && img.height() == height && x == 0 && y == 0) {
    return img;
  }

  QImage dst(width, height, img.format());
  dst.fill(0);

  int x0_src = 0;
  int x0_dst = 0;
  int y0_src = 0;
  int y0_dst = 0;
  int w = img.width();
  int h = img.height();
  if (y < 0) {
    y0_src = -y;
    y0_dst = 0;
    h = std::min(img.height() - y0_src, height);
  } else {
    y0_src = 0;
    y0_dst = y;
    h = std::min(img.height(), height - y0_dst);
  }

  if (x < 0) {
    x0_src = -x;
    x0_dst = 0;
    w = std::min(img.width() - x0_src, width);
  } else {
    x0_src = 0;
    x0_dst = x;
    w = std::min(img.width(), width - x0_dst);
  }

  assert(w <= img.width() && w <= width);
  assert(h <= img.height() && h <= height);

  copyImagePart(dst, x0_dst, y0_dst, img, x0_src, y0_src, w, h);

  assert(dst.width() == width && dst.height() == height);

  return dst;
}

QImage
bleedThrough_aux(const QImage &originalRecto,
                 const QImage &imgRecto,
                 const QImage &imgVersoP,
                 int nbIter,
                 int x,
                 int y,
                 int nbThreads)
{
  assert(originalRecto.depth() >= 8);
  assert(imgRecto.depth() >= 8);
  assert(imgVersoP.depth() >= 8);

  QImage imgVerso =
    getOverlappingPart(imgRecto.width(), imgRecto.height(), imgVersoP, x, y);

  assert(imgVerso.size() == imgRecto.size());

  //B:TODO:OPTIM
  //- decide if we want to call bleedThroughMT0 or another implementation
  //- decide the number of threads based on actual hardware & OS
  if (nbThreads < 0) {
    nbThreads = 4;
    //if (originalRecto.width()*originalRecto.height() < 80000)
    //nbThreads = 1;
  }

  return bleedThroughMT0(originalRecto, imgRecto, imgVerso, nbIter, nbThreads);
}

QImage
bleedThrough(const QImage &originalRecto,
             const QImage &imgRecto,
             const QImage &imgVerso,
             int nbIter,
             int x,
             int y,
             int nbThreads)
{
  assert(originalRecto.size() == imgRecto.size());

  QImage originalRectoG = originalRecto;
  QImage imgRectoG = imgRecto;
  QImage imgVersoG = imgVerso;
  convertToAtLeast8Bit(originalRectoG);
  convertToAtLeast8Bit(imgRectoG);
  convertToAtLeast8Bit(imgVersoG);

  return bleedThrough_aux(
    originalRectoG, imgRectoG, imgVersoG, nbIter, x, y, nbThreads);
}

QImage
bleedThrough(const QImage &imgRecto,
             const QImage &imgVerso,
             int nbIter,
             int x,
             int y,
             int nbThreads)
{
  QImage imgRectoG = imgRecto;
  QImage imgVersoG = imgVerso;
  convertToAtLeast8Bit(imgRectoG);
  convertToAtLeast8Bit(imgVersoG);

  return bleedThrough_aux(
    imgRectoG.copy(), imgRectoG, imgVersoG, nbIter, x, y, nbThreads);
}

#include <QSharedPointer>
#include <context/CurrentRandomDocumentContext.h>

QImage
BleedThrough::apply()
{
  qDebug() << "*********** BleedThrough::apply() _nbIter=" << _nbIter << "\n";

  if (_verso.isNull()) {
    qDebug() << "verso is null";
    return QImage();
  }

  qDebug() << metaObject()->className() << ":::"
           << "APPLYING";

  QImage originalRecto = getBackGround();

  const int width = originalRecto.width();
  const int height = originalRecto.height();

  _verso = _verso.scaled(width, height);

  assert(originalRecto.width() == _verso.width());
  assert(originalRecto.height() == _verso.height());

  QImage imgRecto = originalRecto.copy();
  QImage imgVerso = _verso;

#ifndef NDEBUG
  //DEBUG
  {
    const char *filename = "/tmp/tmp_recto.png";
    std::cerr << "DEBUG BleedThrough: write recto" << filename << "\n";
    imgRecto.save(filename);
  }
  {
    const char *filename = "/tmp/tmp_verso.png";
    std::cerr << "DEBUG BleedThrough: write verso" << filename << "\n";
    imgVerso.save(filename);
    std::cerr << "DEBUG _nbIter=" << _nbIter << "\n";
  }
#endif //NDEBUG

  QProgressDialog progress(QStringLiteral("Bleed Through Effect"),
                           QStringLiteral("Cancel"),
                           0,
                           _nbIter);
  progress.setWindowModality(Qt::WindowModal);

  //B: CODE DUPLICATION with bleedThroughMT0()
  // add QProgressDialog update.

  const std::vector<uchar> originalRectoGray = getGray(originalRecto);
  const std::vector<uchar> versoGray = getGray(imgVerso);
  std::vector<uchar> rectoGray = getGray(imgRecto);

  if (originalRectoGray.empty() || versoGray.empty() || rectoGray.empty())
    return QImage();

  std::vector<uchar> outGray(width * height);

  //B: divide only lines (for cache)

  const int nbThreadsPerCol = 1;
  const int nbThreadsPerRow =
    4; //B:TODO:OPTIM: choose the number of threads according to hardware

  const int blockWidth = (width) / nbThreadsPerCol;
  const int blockHeight = (height) / nbThreadsPerRow;

  //B:TODO:OPTIM: this is a stencil operation !
  //We can code it with some communication/synchronization between threads (for borders) to be be much faster !
  //We would avoid to create/run/destroy the threads each time, and keep them alive during the nbIter iterations !

  QVector<BleedThroughDiffusionThreadA *> threads;
  threads.reserve(nbThreadsPerCol * nbThreadsPerRow);

  const int lNbIter = _nbIter;
  for (int p = 0; p < lNbIter; ++p) {

    progress.setValue(p);

    if (progress.wasCanceled()) {
      break;
    }

    for (int l = 0; l < nbThreadsPerRow; ++l) {
      const int y = l * blockHeight;
      const int h = (l != nbThreadsPerRow - 1 ? blockHeight : height - y);
      for (int k = 0; k < nbThreadsPerCol; ++k) {
        const int x = k * blockWidth;
        const int w = (k != nbThreadsPerCol - 1 ? blockWidth : width - x);

        BleedThroughDiffusionThreadA *thread1 =
          new BleedThroughDiffusionThreadA(originalRectoGray,
                                           rectoGray,
                                           versoGray,
                                           width,
                                           height,
                                           QPoint(x, y),
                                           QPoint(x + w, y + h));
        thread1->start();
        threads.push_back(thread1);
      }
    }

    for (BleedThroughDiffusionThreadA *thread : threads) {
      thread->wait();
      thread->getRecto(outGray);
      delete thread;
    }
    threads.clear();

    outGray.swap(rectoGray);

#if 0
      //B: If we emit during progression
      // we will save intermediary images !!!

      Context::CurrentRandomDocumentContext::instance()->addProperty("trans_iter", QString::number(p));//B: ???
      if (p % 16 == 0) { //B: update frequency should be dependent on image size
	QImage recto = makeRGBfromGray(width, height, rectoGray);
	emit imageReady(recto);
      }
#endif
  }

  progress.setValue(lNbIter);

  QImage out = makeRGBfromGray(width, height, rectoGray);

#ifndef NDEBUG
  qDebug() << "BleedThrough::apply() emit imageReady(out)\n";

/*
    {
      const char *filename = "/tmp/tmp.png";
      std::cerr<<"DEBUG BleedThrough: write "<<filename<<"\n";
      out.save(filename);
    }
    */
#endif //NDEBUG

  emit imageReady(out);

  return out;
}

QImage
BleedThrough::apply(const QString &rectoPath)
{
  qDebug() << "*********** BleedThrough::apply() _nbIter=" << _nbIter << "\n";

  if (_verso.isNull()) {
    qDebug() << "verso is null";
    return QImage();
  }

  qDebug() << metaObject()->className() << ":::"
           << "APPLYING";

  QImage originalRecto = QImage(rectoPath);

  const int width = originalRecto.width();
  const int height = originalRecto.height();

  _verso = _verso.scaled(width, height);

  assert(originalRecto.width() == _verso.width());
  assert(originalRecto.height() == _verso.height());

  QImage imgRecto = originalRecto.copy();
  QImage imgVerso = _verso;

#ifndef NDEBUG
  //DEBUG
  {
    const char *filename = "/tmp/tmp_recto.png";
    std::cerr << "DEBUG BleedThrough: write recto" << filename << "\n";
    imgRecto.save(filename);
  }
  {
    const char *filename = "/tmp/tmp_verso.png";
    std::cerr << "DEBUG BleedThrough: write verso" << filename << "\n";
    imgVerso.save(filename);
    std::cerr << "DEBUG _nbIter=" << _nbIter << "\n";
  }
#endif //NDEBUG

  QProgressDialog progress(QStringLiteral("Bleed Through Effect"),
                           QStringLiteral("Cancel"),
                           0,
                           _nbIter);
  progress.setWindowModality(Qt::WindowModal);

  //B: CODE DUPLICATION with bleedThroughMT0()
  // add QProgressDialog update.

  const std::vector<uchar> originalRectoGray = getGray(originalRecto);
  const std::vector<uchar> versoGray = getGray(imgVerso);
  std::vector<uchar> rectoGray = getGray(imgRecto);

  if (originalRectoGray.empty() || versoGray.empty() || rectoGray.empty())
    return QImage();

  std::vector<uchar> outGray(width * height);

  //B: divide only lines (for cache)

  const int nbThreadsPerCol = 1;
  const int nbThreadsPerRow =
    4; //B:TODO:OPTIM: choose the number of threads according to hardware

  const int blockWidth = (width) / nbThreadsPerCol;
  const int blockHeight = (height) / nbThreadsPerRow;

  //B:TODO:OPTIM: this is a stencil operation !
  //We can code it with some communication/synchronization between threads (for borders) to be be much faster !
  //We would avoit to create/run/destroy the threads each time, and keep them alive during the nbIter iterations !

  QVector<BleedThroughDiffusionThreadA *> threads;
  threads.reserve(nbThreadsPerCol * nbThreadsPerRow);

  const int lNbIter = _nbIter;
  for (int p = 0; p < lNbIter; ++p) {

    progress.setValue(p);

    if (progress.wasCanceled()) {
      break;
    }

    for (int l = 0; l < nbThreadsPerRow; ++l) {
      const int y = l * blockHeight;
      const int h = (l != nbThreadsPerRow - 1 ? blockHeight : height - y);
      for (int k = 0; k < nbThreadsPerCol; ++k) {
        const int x = k * blockWidth;
        const int w = (k != nbThreadsPerCol - 1 ? blockWidth : width - x);

        BleedThroughDiffusionThreadA *thread1 =
          new BleedThroughDiffusionThreadA(originalRectoGray,
                                           rectoGray,
                                           versoGray,
                                           width,
                                           height,
                                           QPoint(x, y),
                                           QPoint(x + w, y + h));
        thread1->start();
        threads.push_back(thread1);
      }
    }

    for (BleedThroughDiffusionThreadA *thread : threads) {
      thread->wait();
      thread->getRecto(outGray);
      delete thread;
    }
    threads.clear();

    outGray.swap(rectoGray);

#if 0
      //B: If we emit during progression
      // we will save intermediary images !!!

      Context::CurrentRandomDocumentContext::instance()->addProperty("trans_iter", QString::number(p));//B: ???
      if (p % 16 == 0) { //B: update frequency should be dependent on image size
	QImage recto = makeRGBfromGray(width, height, rectoGray);
	emit imageReady(recto);
      }
#endif
  }

  progress.setValue(lNbIter);

  QImage out = makeRGBfromGray(width, height, rectoGray);

#ifndef NDEBUG
  qDebug() << "BleedThrough::apply() emit imageReady(out)\n";

/*
    {
      const char *filename = "/tmp/tmp.png";
      std::cerr<<"DEBUG BleedThrough: write "<<filename<<"\n";
      out.save(filename);
    }
    */
#endif //NDEBUG

  emit imageReady(out);

  return out;
}

void
BleedThrough::setVerso(const QString &path)
{
  qDebug() << "BleedThrough::setVerso " << path;
  _verso = QImage(path);
  //_verso = _verso.convertToFormat(QImage::Format_RGB32);
  _verso = _verso.mirrored(true, false);
  qDebug() << path << " " << _verso.isNull() << "\n";
  //apply();
}

void
BleedThrough::setVersoAndApply(const QString &versoPath)
{
  setVerso(versoPath);
  apply();
}
