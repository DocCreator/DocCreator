#ifndef LABELINGCOMPONENTDIALOG_HPP
#define LABELINGCOMPONENTDIALOG_HPP

#include <QDialog>

namespace Ui {
class LabelingComponentDialog;
}

enum BlockType
{
  TextBlock,
  ImageBlock,
  BackGround,
  Other
};

class LabelingComponentDialog : public QDialog
{
  Q_OBJECT

public:
  explicit LabelingComponentDialog(QWidget *parent = 0);
  ~LabelingComponentDialog();

  BlockType getBlockType() { return _blockType; }
public slots:
  void setChecked();

private:
  Ui::LabelingComponentDialog *ui;
  BlockType _blockType;
};

#endif // LABELINGCOMPONENTDIALOG_HPP
