#ifndef SHADOWBINDINGDIALOG_HPP
#define SHADOWBINDINGDIALOG_HPP

#include "Degradations/ShadowBinding.hpp" //ShadowBorder
#include <QDialog>

class QLabel;

namespace Ui {
class ShadowBindingDialog;
}

class ShadowBindingDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ShadowBindingDialog(QWidget *parent = nullptr);

  ~ShadowBindingDialog();

  ShadowBindingDialog(const ShadowBindingDialog &) = delete;
  ShadowBindingDialog &operator=(const ShadowBindingDialog &) = delete;

  /**
   * return a distance/radius in pixels.
   */
  int getDistance() const { return _distance; }

  /**
   * return an intensity in [0; 1].
   */
  float getIntensity() const { return _intensity; }

  /**
   * return an angle in [0; 90]
   */
  float getAngle() const { return _angle; }

  const dc::ShadowBorder &getBorder() const { return _border; }

  void setOriginalImage(const QImage &img);

public slots:
  void distanceChanged(int distance);
  void intensityChanged(int intensity);
  void angleChanged(int angle);
  void borderChanged();

protected:
  virtual void changeEvent(QEvent *e) override;

  void setupGUIImages();
  void updateResultImage();

  int getDistanceAux(int distance) const;
  float getIntensityAux(int intensity) const;

private:
  Ui::ShadowBindingDialog *ui;
  int _distance;
  float _intensity;
  float _angle;
  dc::ShadowBorder _border;

  QLabel *_originalLabel;
  QLabel *_resultLabel;

  QImage _originalImg;
  QImage _resultImg;
  QImage _originalImgSmall;
  QImage _resultImgSmall;
};

#endif // SHADOWBINDINGDIALOG_HPP
