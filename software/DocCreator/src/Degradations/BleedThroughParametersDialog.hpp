#ifndef BLEEDTHROUGHPARAMETERSDIALOG_HPP
#define BLEEDTHROUGHPARAMETERSDIALOG_HPP

#include "BleedThroughParameters.hpp"
#include "VersoImageChangerParameters.hpp"
#include <QDialog>

class QLabel;

namespace Ui {
class BleedThroughParametersDialog;
}

class BleedThroughParametersDialog : public QDialog
{
  Q_OBJECT

public:
  explicit BleedThroughParametersDialog(QWidget *parent = nullptr);

  ~BleedThroughParametersDialog();

  BleedThroughParametersDialog(const BleedThroughParametersDialog &) = delete;
  BleedThroughParametersDialog &operator=(
    const BleedThroughParametersDialog &) = delete;

  void getParameters(BleedThroughParameters &o) const { o = _params; }

  //TODO: remove !
  void getVersoParams(VersoImageChangerParameters &o) const
  {
    o = _versoParams;
  }

  void setRectoImage(const QImage &rectoImg);

  void setVersoImage(const QImage &versoImg);

public slots:
  void versoChosen();

  void nbIterationsChanged(int value);

protected:
  virtual void changeEvent(QEvent *e) override;

  void setupGUIImages();
  void updateVersoImage();
  void updateBleedImage(int nbIter, bool fromZero = false);

private:
  Ui::BleedThroughParametersDialog *ui;
  BleedThroughParameters _params;
  //TODO: remove !
  VersoImageChangerParameters _versoParams;

  QLabel *_rectoLabel;
  QLabel *_versoLabel;
  QLabel *_bleedLabel;
  QImage _rectoImg;
  QImage _versoImg;
  QImage _rectoImgPart;
  QImage _versoImgPart;
  QImage _rectoImgSmall;
  QImage _versoImgSmall;
  QImage _bleedImgPart;
};

#endif // BLEEDTHROUGHPARAMETERSDIALOG_HPP
