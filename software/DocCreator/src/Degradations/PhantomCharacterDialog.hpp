#ifndef PHANTOMCHARACTERDIALOG_HPP
#define PHANTOMCHARACTERDIALOG_HPP

#include <QDialog>
#include <QString>

#include "Degradations/PhantomCharacter.hpp"

class QLabel;

namespace Ui {
class PhantomCharacterDialog;
}

class PhantomCharacterDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PhantomCharacterDialog(QWidget *parent = nullptr);

  ~PhantomCharacterDialog();

  PhantomCharacterDialog(const PhantomCharacterDialog &) = delete;
  PhantomCharacterDialog &operator=(const PhantomCharacterDialog &) = delete;

  const QImage &getResultImg() const { return _resultImg; }

  Frequency getFrequency() const { return _frequency; }

  void setOriginalImage(const QImage &img);

public slots:
  void frequencyChanged(int frequency);
  void zoomXChanged(int zoomX);
  void zoomYChanged(int zoomY);
  void updateZoom();

protected:
  virtual void changeEvent(QEvent *e) override;
  void setupGUIImages();
  void updateResultImage();

private:
  Ui::PhantomCharacterDialog *ui;
  Frequency _frequency;
  int _zoomX;
  int _zoomY;
  QString _phantomPatternsPath;

  QLabel *_originalLabel;
  QLabel *_resultLabel;

  QImage _originalImg;
  QImage _resultImg;
  QImage _originalImgSmall;
  QImage _resultImgSmall;
  QImage _resultImgPart;
};

#endif // PHANTOMCHARACTERDIALOG_HPP
