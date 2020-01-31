#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

class QComboBox;
class QFontComboBox;
class QLineEdit;
class QPushButton;
class QStatusBar;

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  
  MainWindow();

protected:

  void buildGUI();
  void findStyles(const QFont &font);
  void findSizes(const QFont &font);
  void updateOutputFilename(const QFont &font);
  QString getOutputFilename(const QFont &font);

protected slots:
  void changeFont(const QFont &font);
  void chooseOutputFilename();
  void updateProcess();
  void produceOutput();
  void updateOutputFilename();

protected:
  QFontComboBox *m_fontCB;
  QComboBox *m_sizeCB;
  QComboBox *m_styleCB;
  QLineEdit *m_outputFileLE;
  QPushButton *m_outputFilePB;
  QPushButton *m_processPB;
  QStatusBar *m_statusBar;

};

#endif /* ! MAINWINDOW_HPP */
