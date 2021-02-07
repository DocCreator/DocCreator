#ifndef CHARACTERWIDGET_HPP
#define CHARACTERWIDGET_HPP

//inspired from Qt's example
// https://doc.qt.io/qt-5/qtwidgets-widgets-charactermap-example.html

#include <QFont>
#include <QSize>
#include <QWidget>

#include "Range.hpp"

class QMouseEvent;
class QPaintEvent;

class CharacterWidget : public QWidget
{
  Q_OBJECT

public:
  CharacterWidget(QWidget *parent = nullptr);
  QSize sizeHint() const override;
  void setChoices(const RangeVector &choices);

public slots:
  void updateFont(const QFont &font);
  void updateSize(const QString &fontSize);
  void updateStyle(const QString &fontStyle);
  void updateFontMerging(bool enable);

  //signals:
  //void characterSelected(const QString &character);

protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  //void mousePressEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private:
  void calculateSquareSize();

private:
  QFont displayFont;
  std::vector<int> choices;
  
  int columns = 16;
  //uint lastKey = -1;
  int squareSize = 0;
};

#endif /* ! CHARACTERWIDGET_HPP */
