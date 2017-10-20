#ifndef OCRSETTINGSDIALOG_HPP
#define OCRSETTINGSDIALOG_HPP

#include <QDialog>

class QLineEdit;
class QComboBox;
class QPushButton;

class OCRSettingsDialog : public QDialog
{
  Q_OBJECT

public:
  /*
    Specified directory @a tessdataParentDir will be used only of TESSDATA_PREFIX environment variable is not set.
   */
  OCRSettingsDialog(QWidget *parent = nullptr,
                    const QString &tessdataParentDir = QString());

  QString tessdataParentDir() const;
  QString language() const;

protected slots:

  void openDir();
  void getEnv();

  void updateLanguages();

protected:
  void buildGUI();

protected:
  QLineEdit *m_dirLE;
  QComboBox *m_langCB;
  QPushButton *m_okButton;
  QString m_tessdataParentDir2;
};

#endif /* !  OCRSETTINGSDIALOG_HPP */
