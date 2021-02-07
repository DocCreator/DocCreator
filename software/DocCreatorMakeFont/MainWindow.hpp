#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include "Range.hpp"

class QCheckBox;
class QComboBox;
class QFontComboBox;
class QLineEdit;
class QPushButton;
class QStatusBar;

class CharacterWidget;

class MainWindow : public QWidget
{
  Q_OBJECT

public:
  
  MainWindow();

protected:

  void populateChoices();
  void buildGUI();
  void findStyles(const QFont &font);
  void findSizes(const QFont &font);
  void updateOutputFilename(const QFont &font);
  QString getOutputFilename(const QFont &font);

protected slots:
  void changeFont(const QFont &font);
  void chooseOutputFilename();
  void updateProcess();
  void saveCharacters();
  void updateOutputFilename();
  void updateChoice(int);

protected:
  CharacterWidget *m_characterWidget;
  QFontComboBox *m_fontCB;
  QComboBox *m_sizeCB;
  QComboBox *m_styleCB;
  QCheckBox *m_fontMergingCB;
  QLineEdit *m_outputFileLE;
  QPushButton *m_outputFilePB;
  QComboBox *m_choicesCB;
  QPushButton *m_processPB;
  QStatusBar *m_statusBar;

  std::vector<RangeVector> m_choices;

};

#endif /* ! MAINWINDOW_HPP */
