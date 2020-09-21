#ifndef COLOR_BUTTON_HPP
#define COLOR_BUTTON_HPP

#include <QPushButton>

class ColorButton : public QPushButton
{
  Q_OBJECT

public:

  /**
   * @brief Constructor.
   *
   * Button is built with default style color.
   */
  explicit ColorButton(QWidget *parent = NULL);

  /**
   * @brief Constructor.
   *
   * Button is built with given color.
   */
  ColorButton(QRgb color, QWidget *parent = NULL);

  /**
   * @brief Set color of button.
   */
  void setColor(QRgb newColor);

  /**
   * @brief Get color of button.
   */
  QRgb getColor() const;


signals:

  void colorChanged(QRgb newColor);

protected slots:

  void getColorFromDialog();
  
  void setColorSS(QRgb newColor);

protected:
  QRgb m_color;
};

#endif /* ! COLOR_BUTTON_HPP */
