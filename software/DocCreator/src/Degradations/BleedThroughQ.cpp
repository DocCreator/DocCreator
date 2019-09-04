#include "BleedThroughQ.hpp"
#include <QDebug>
#include <QProgressDialog>
#include "Utils/convertor.h"


namespace dc {

  namespace BleedThrough {

    QImage bleedThrough(const QImage &imgRecto, const QImage &imgVerso, int nbIters, int x, int y, int nbThreads)
    {
      cv::Mat imgRectoMat = Convertor::getCvMat(imgRecto);
      cv::Mat imgVersoMat = Convertor::getCvMat(imgVerso);

      cv::Mat out = dc::BleedThrough::bleedThrough(imgRectoMat.clone(), imgRectoMat, imgVersoMat, nbIters, x, y, nbThreads);

      return Convertor::getQImage(out);
    }
    
    QImage bleedThrough(const QImage &originalRecto, const QImage &imgRecto, const QImage &imgVerso, int nbIters, int x, int y, int nbThreads)
    {
      cv::Mat originalRectoMat = Convertor::getCvMat(originalRecto);
      cv::Mat imgRectoMat = Convertor::getCvMat(imgRecto);
      cv::Mat imgVersoMat = Convertor::getCvMat(imgVerso);

      cv::Mat out = dc::BleedThrough::bleedThrough(originalRectoMat, imgRectoMat, imgVersoMat, nbIters, x, y, nbThreads);

      return Convertor::getQImage(out);
    }

  } //namespace BleedThrough



  void
  BleedThroughQ::setVerso(const QString &path)
  {
    //qDebug() << "BleedThrough::setVerso " << path;
    _verso = QImage(path);
    ////_verso = _verso.convertToFormat(QImage::Format_RGB32);
    _verso = _verso.mirrored(true, false);
    //qDebug() << path << " " << _verso.isNull() << "\n";
    ////apply();
  }

  QImage
  BleedThroughQ::apply()
  {
    if (_verso.isNull()) {
      qDebug() << "BleedThrough: verso is null";
      return QImage();
    }

    QImage originalRecto = getBackGround();

    return apply(originalRecto);
  }

  QImage
  BleedThroughQ::apply(const QString &rectoPath)
  {
    if (_verso.isNull()) {
      qDebug() << "BleedThrough: verso is null";
      return QImage();
    }

    QImage originalRecto = QImage(rectoPath);

    return apply(originalRecto);
  }

  QImage
  BleedThroughQ::apply(QImage originalRecto)
  {
    assert(! _verso.isNull());

    //qDebug() << metaObject()->className() << ":::"<< "APPLYING";


    const int width = originalRecto.width();
    const int height = originalRecto.height();

    _verso = _verso.scaled(width, height);

    assert(originalRecto.width() == _verso.width());
    assert(originalRecto.height() == _verso.height());

    QImage imgRecto = originalRecto.copy();
    QImage imgVerso = _verso;


    QProgressDialog progress(QStringLiteral("Bleed Through Effect"),
			     QStringLiteral("Cancel"),
			     0,
			     _nbIter);
    progress.setWindowModality(Qt::WindowModal);

    
    cv::Mat originalRectoMat = Convertor::getCvMat(originalRecto);
    cv::Mat imgRectoMat = Convertor::getCvMat(imgRecto);
    cv::Mat imgVersoMat = Convertor::getCvMat(imgVerso);
    cv::Mat outMat; // = originalRectoMat.clone();
    
    const int lNbIter = _nbIter;
    for (int p = 0; p < lNbIter; ++p) {

      progress.setValue(p);

      if (progress.wasCanceled()) {
	break;
      }

      outMat = dc::BleedThrough::bleedThrough(originalRectoMat, imgRectoMat, imgVersoMat, 1, 0, 0, -1);

      cv::swap(imgRectoMat, outMat);
      
      //B: some CODE DUPLICATION with bleedThroughMT0()
      //B: As we want to indicate progression, we are slower...

    }
    progress.setValue(lNbIter);

    QImage out = Convertor::getQImage(imgRectoMat);

    emit imageReady(out);

    return out;
  }      

  void
  BleedThroughQ::setVersoAndApply(const QString &versoPath)
  {
    setVerso(versoPath);
    apply();
  }

} //namespace dc
