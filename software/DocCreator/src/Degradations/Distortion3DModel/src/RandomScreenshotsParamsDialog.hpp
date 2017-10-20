#ifndef RANDOMSCREENSHOTSPARAMSDIALOG_HPP
#define RANDOMSCREENSHOTSPARAMSDIALOG_HPP

#include <QDialog>

class QLineEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QPushButton;

class RandomScreenshotsParamsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit RandomScreenshotsParamsDialog(QWidget *parent = nullptr);

  void setDstDirectory(const QString &dir);
  QString getDstDirectory() const;

  void setDstPrefix(const QString &prefix);
  QString getDstPrefix() const;

  void setDstExtension(const QString &text);
  QString getDstExtension() const;

  void setNumScreenshots(size_t num);
  size_t getNumScreenshots() const;

  void setMinPhy(float v);
  float getMinPhy() const;

  void setMaxPhy(float v);
  float getMaxPhy() const;

  void setMinTheta(float v);
  float getMinTheta() const;

  void setMaxTheta(float v);
  float getMaxTheta() const;

protected slots:

  void chooseDstDirectory();

protected:
  QLineEdit *m_dstDirectory;
  QLineEdit *m_dstPrefix;
  QComboBox *m_extensions;
  QSpinBox *m_numScreenshots;

  QPushButton *m_advancedButton;
  QWidget *m_extension;
  QDoubleSpinBox *m_minPhy;
  QDoubleSpinBox *m_maxPhy;
  QDoubleSpinBox *m_minTheta;
  QDoubleSpinBox *m_maxTheta;
};

#endif /* ! RANDOMSCREENSHOTSPARAMSDIALOG_HPP */
