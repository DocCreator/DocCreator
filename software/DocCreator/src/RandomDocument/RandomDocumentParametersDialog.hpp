#ifndef RANDOMDOCUMENTPARAMETERSDIALOG_HPP
#define RANDOMDOCUMENTPARAMETERSDIALOG_HPP

#include "RandomDocumentParameters.hpp"
#include <QDialog>

namespace Ui {
class RandomDocumentParametersDialog;
}

class RandomDocumentParametersDialog : public QDialog
{
  Q_OBJECT

public:
  explicit RandomDocumentParametersDialog(QWidget *parent = 0);
  ~RandomDocumentParametersDialog();

  const RandomDocumentParameters &getParameters() const { return _params; }

public slots:
  void chooseOutputFolderSlot();
  void chooseFontsSlot();
  void chooseTextsSlot();

protected slots:
  void updateLineSpacingMin();
  void updateLineSpacingMax();
  void updateNbBlocksPerColMin();
  void updateNbBlocksPerColMax();
  void updateNbBlocksPerRowMin();
  void updateNbBlocksPerRowMax();

protected:
  void changeEvent(QEvent *e) override;

private:
  Ui::RandomDocumentParametersDialog *_ui;
  RandomDocumentParameters _params;
};

#endif // RANDOMDOCUMENTPARAMETERSDIALOG_HPP
