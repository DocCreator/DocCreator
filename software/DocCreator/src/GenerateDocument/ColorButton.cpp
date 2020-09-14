#include "ColorButton.hpp"

#include <sstream>
#include <QColorDialog>


ColorButton::ColorButton(QWidget *parent)
  : QPushButton(parent),
    m_color(0)
{
  connect(this, SIGNAL(clicked()), this, SLOT(getColorFromDialog()));
}

ColorButton::ColorButton(QRgb color, QWidget *parent)
  : QPushButton(parent),
    m_color(color)
{
  connect(this, SIGNAL(clicked()), this, SLOT(getColorFromDialog()));

  setColorSS(color);
}

void
ColorButton::setColor(QRgb newColor)
{
  if (newColor != getColor()) {
    m_color = newColor;
    setColorSS(newColor);
    emit colorChanged(newColor);
  }
}

QRgb
ColorButton::getColor() const
{
  return m_color;
}

void
ColorButton::getColorFromDialog()
{
  const QColor newColor = QColorDialog::getColor(getColor());
  if (newColor.isValid()) {
    const QRgb c = newColor.rgb();
    if (c != m_color) {
      m_color = c;
      setColorSS(c);
      emit colorChanged(c);
    }
  }
}

void
ColorButton::setColorSS(QRgb color)
{
  /*
  QPalette palette = colorButton->palette();
  palette.setColor(QPalette::Button, color);
  colorButton->setPalette(palette);

  //This does not work on Windows XP or everywhere a style is set !
  */

  std::stringstream ss;
  ss<<"background-color: rgb("<<qRed(color)<<","<<qGreen(color)<<","<<qBlue(color)<<"); border-color: beige; border-style: outset; border-width: 2px; border-radius: 6px; min-width: 6px; padding: 6px; ";
  const QString cssPB = QLatin1String(ss.str().c_str());

  setStyleSheet(cssPB);
  update();
}
