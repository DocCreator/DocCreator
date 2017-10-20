#ifndef GRAYCHARACTERDEGRADATIONDIALOG_H
#define GRAYCHARACTERDEGRADATIONDIALOG_H

#include <QDialog>
class DocumentController;

const int TOTAL_PERCENT = 100;

namespace Ui {
class GrayCharacterDegradationDialog;
}

class GrayCharacterDegradationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit GrayCharacterDegradationDialog(DocumentController *docController,
                                          QWidget *parent = 0);
  ~GrayCharacterDegradationDialog();
  void degrade();

public slots:
  void chooseSaveDirectory();
  void changeIndependentPercent();
  void changeOverlappingPercent();
  void changeDisconnectionPercent();

private:
  Ui::GrayCharacterDegradationDialog *ui;

  DocumentController *_docController;
};

#endif // GRAYCHARACTERDEGRADATIONDIALOG_H
