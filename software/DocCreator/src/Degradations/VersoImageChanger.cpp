#include "VersoImageChanger.hpp"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "Utils/ImageUtils.hpp" //getReadImageFilterList()
#include "Utils/RandomElement.hpp"

VersoImageChanger::VersoImageChanger(const VersoImageChangerParameters &params,
                                     QObject *parent)
  : QObject(parent)
{
  _params = params;
}

void
VersoImageChanger::changeVerso()
{
  qDebug() << "#@@@@@ VersoImageChanger::changeVerso() _params._versosDirPath="
           << _params._versosDirPath;

  QDir versosDir(_params._versosDirPath);
  QFileInfoList list = versosDir.entryInfoList(
    getReadImageFilterList()); //QStringList() << "*.png" << "*.PNG" << "*.jpeg" <<"*.JPEG" <<"*.jpg" <<"*.JPG" <<"*.tif" <<"*.TIF"); //B
  if (!list.empty()) {
    RandomElement elem(this);
    const int index = elem.randomInt(0, list.size() - 1);
    qDebug() << "#@@@@@ VersoImageChanger::changeVerso() emit versoChanged "
             << list.at(index).absoluteFilePath() << " " << index;

    emit versoChanged(list.at(index).absoluteFilePath());
  } else {
    qDebug() << "Warning: VersoImageChanger::changeVerso(): no image found in "
                "directory: "
             << _params._versosDirPath;
  }
}
