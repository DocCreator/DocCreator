#include "CharacterWidget.hpp"

#include <QFontDatabase>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>


CharacterWidget::CharacterWidget(QWidget *parent)
  : QWidget(parent)
{
  calculateSquareSize();
  setMouseTracking(true);
}

void CharacterWidget::updateFont(const QFont &font)
{
  displayFont.setFamily(font.family());
  calculateSquareSize();
  adjustSize();
  update();
}

void CharacterWidget::updateSize(const QString &fontSize)
{
  displayFont.setPointSize(fontSize.toInt());
  calculateSquareSize();
  adjustSize();
  update();
}

void CharacterWidget::updateStyle(const QString &fontStyle)
{
  QFontDatabase fontDatabase;
  const QFont::StyleStrategy oldStrategy = displayFont.styleStrategy();
  displayFont = fontDatabase.font(displayFont.family(), fontStyle, displayFont.pointSize());
  displayFont.setStyleStrategy(oldStrategy);
  calculateSquareSize();
  adjustSize();
  update();
}

void CharacterWidget::updateFontMerging(bool enable)
{
  if (enable)
    displayFont.setStyleStrategy(QFont::PreferDefault);
  else
    displayFont.setStyleStrategy(QFont::NoFontMerging);
  adjustSize();
  update();
}

void CharacterWidget::calculateSquareSize()
{
  squareSize = qMax(16, 4 + QFontMetrics(displayFont, this).height());
}


QSize CharacterWidget::sizeHint() const
{
  int num = choices.size();
  if (num == 0)
    num = columns*columns;

  return QSize(columns*squareSize, ((num + columns-1) / columns) * squareSize);
}

void CharacterWidget::setChoices(const RangeVector &choicesRV)
{
  choices.resize(0);
  choices.reserve(65536);
  for (const Range &r : choicesRV) {
    for (int i=r.start; i<=r.end; ++i) {
      choices.push_back(i);
    }
  }
  resize(sizeHint()); //to update the scrollArea
  update();
}


void CharacterWidget::mouseMoveEvent(QMouseEvent *event)
{
  QPoint widgetPosition = mapFromGlobal(event->globalPos());
  const uint key = (widgetPosition.y() / squareSize) * columns + widgetPosition.x() / squareSize;
  if (key < choices.size()) {

    QString text = QString::fromLatin1("<p>Character: <span style=\"font-size: 24pt; font-family: %1\">").arg(displayFont.family())
      + QChar(choices[key])
      //+ QString::fromLatin1("</span><p>Value: 0x")
      + QString::fromLatin1("</span><p>Unicode: U+")
      + QString::number(choices[key], 16);
    QToolTip::showText(event->globalPos(), text, this);
  }
}

/*
void CharacterWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
    lastKey = (event->y() / squareSize) * columns + event->x() / squareSize;
    if (lastKey < choices.size()) {
      if (QChar(choices[lastKey]).category() != QChar::Other_NotAssigned)
	emit characterSelected(QString(QChar(choices[lastKey])));
    }
    update();
  }
  else
    QWidget::mousePressEvent(event);
}
*/

void CharacterWidget::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.fillRect(event->rect(), QBrush(Qt::white));
  painter.setFont(displayFont);

  const QRect redrawRect = event->rect();
  const int beginRow = redrawRect.top() / squareSize;
  const int endRow = redrawRect.bottom() / squareSize;
  const int beginColumn = redrawRect.left() / squareSize;
  const int endColumn = redrawRect.right() / squareSize;

  painter.setPen(QPen(Qt::gray));
  for (int row = beginRow; row <= endRow; ++row) {
    for (int column = beginColumn; column <= endColumn; ++column) {
      painter.drawRect(column * squareSize, row * squareSize, squareSize, squareSize);
    }
  }

  QFontMetrics fontMetrics(displayFont);
  painter.setPen(QPen(Qt::black));
  for (int row = beginRow; row <= endRow; ++row) {
    for (int column = beginColumn; column <= endColumn; ++column) {
      const uint key = row * columns + column;
      painter.setClipRect(column * squareSize, row * squareSize, squareSize, squareSize);

      /*
      if (key == lastKey)
	painter.fillRect(column * squareSize + 1, row * squareSize + 1,
			 squareSize, squareSize, QBrush(Qt::red));
      */

      if (key < choices.size()) {
	const QChar c(choices[key]);

	painter.drawText(column * squareSize + (squareSize / 2) -
			 //fontMetrics.horizontalAdvance(QChar(key)) / 2,  //Qt 5.11 only
			 fontMetrics.width(c) / 2,
			 row * squareSize + 4 + fontMetrics.ascent(),
			 QString(c));
      }
    }
  }
}
    
