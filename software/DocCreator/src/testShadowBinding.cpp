#include "Degradations/ShadowBinding.hpp"
#include "Utils/convertor.h"
#include <QApplication>
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
  int border = 0;
  int distance = 50;
  float intensity = 0.5;
  float angle = 30;

  if (argc > 1) {
    border = atoi(argv[1]);
  }
  if (argc > 2) {
    distance = atoi(argv[2]);
  }
  if (argc > 3) {
    intensity = atof(argv[3]);
  }
  if (argc > 4) {
    angle = atof(argv[4]);
  }
  if (argc > 5) {
    inImagePath = argv[5];
  }

  QImage imIn = QImage(inImagePath);

  QLabel *label = new QLabel();

  QImage imOut;
  imOut = shadowBinding(imIn, (ShadowBorder)border, distance, intensity, angle);

  if (argc > 5) {
    imOut.save(argv[5]);
  }

  label->setPixmap(QPixmap::fromImage(imOut));
  auto layout = new QVBoxLayout;
  layout->addWidget(label);
  window->setLayout(layout);

  window->show();

  return app.exec();
}
