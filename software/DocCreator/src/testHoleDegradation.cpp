#include "Degradations/HoleDegradationQ.hpp"
#include "Utils/convertor.h"
#include <QApplication>
#include <QColor>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <opencv2/imgproc/imgproc.hpp>

#include "Utils/ImageUtils.hpp"

int
main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QWidget *window = new QWidget();

  const char *inImagePath = "/home/doctorant/LABRI/sDEfauts/CESR_287_0010.jpg";
  int pattern = 1;
  int xOrigin = 0;
  int yOrigin = 0;
  int type = 0;
  int side = 0;
  QImage pageBelow = QImage("");

  if (argc > 1) {
    pattern = atoi(argv[1]);
  }
  if (argc > 2) {
    xOrigin = atoi(argv[2]);
  }
  if (argc > 3) {
    yOrigin = atoi(argv[3]);
  }
  if (argc > 4) {
    type = atoi(argv[4]);
  }
  if (argc > 5) {
    side = atoi(argv[5]);
  }
  if (argc > 6) {
    inImagePath = argv[6];
  }

  QImage imIn = QImage(inImagePath);
  QLabel *label = new QLabel();
  QImage imOut;

  QString path = "/home/doctorant/Trous/pattern";

  if (type == 1)
    path += "Border";
  else if (type == 2)
    path += "Corner";

  path += QString::number(pattern) + ".png";

  QImage patternImg = QImage(path);
  imOut = dc::HoleDegradation::holeDegradation(imIn,
					       patternImg,
					       xOrigin,
					       yOrigin,
					       0,
					       (dc::HoleDegradation::HoleType)type,
					       side,
					       QColor(0, 0, 0, 0),
					       pageBelow,
					       5,
					       1000);

  if (argc > 7) {
    imOut.save(argv[7]);
  }

  label->setPixmap(QPixmap::fromImage(imOut));
  auto layout = new QVBoxLayout;
  layout->addWidget(label);
  window->setLayout(layout);

  window->show();

  return app.exec();
}
