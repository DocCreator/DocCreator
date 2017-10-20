#ifndef GRAYCHARACTERDEGRADATIONPARAMETER_HPP
#define GRAYCHARACTERDEGRADATIONPARAMETER_HPP

#include <QDialog>
class DocumentController;

namespace Ui {
class GrayCharacterDegradationParameter;
}

class GrayCharacterDegradationParameter : public QDialog
{
  Q_OBJECT

public:
  explicit GrayCharacterDegradationParameter(DocumentController *docController,
                                             QWidget *parent = 0);
  ~GrayCharacterDegradationParameter();
  void degrade();
public slots:
  void chooseSaveDirectory();

private:
  Ui::GrayCharacterDegradationParameter *ui;
  DocumentController *_docController;
};

#endif // GRAYCHARACTERDEGRADATIONPARAMETER_HPP
