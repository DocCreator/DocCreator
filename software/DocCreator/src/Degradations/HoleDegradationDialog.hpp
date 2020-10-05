#ifndef HOLEDEGRADATIONDIALOG_HPP
#define HOLEDEGRADATIONDIALOG_HPP

#include <QDialog>
#include <QImage>
#include <QString>
#include <QStringList>

#include "Degradations/HoleDegradation.hpp" //for HoleType, Hole, ...

class QLabel;

namespace Ui {
class HoleDegradationDialog;
}

class HoleDegradationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit HoleDegradationDialog(QWidget *parent = nullptr);

  ~HoleDegradationDialog();

  HoleDegradationDialog(const HoleDegradationDialog &) = delete;
  HoleDegradationDialog &operator=(const HoleDegradationDialog &) = delete;

  const QImage &getResultImage() const { return _resultImg; }

  const QImage &getPattern() const { return _patternImg; }

  const QImage &getPageBelow() const { return _pageBelow; }

  const QColor &getColor() const { return _color; }

  int getHorizontal() const { return _horizontal; }

  int getVertical() const { return _vertical; }

  int getWidth() const { return _width; }

  float getIntensity() const { return _intensity; }

  int getSize() const { return _size; }

  dc::HoleDegradation::HoleType getType() const { return _type; }

  //int getSide() const { return _side; }

  void setOriginalImage(const QImage &img);
  void findBound(dc::HoleDegradation::HoleType type,
                 int &minH,
                 int &maxH,
                 int &minV,
                 int &maxV,
                 dc::HoleDegradation::HoleSide side,
                 const QImage &patternImg);
  static QString getHolePatternsPath();
  static QString getCenterHolePatternsPath();
  static QString getBorderHolePatternsPath();
  static QString getCornerHolePatternsPath();
  static QStringList getDirectoryList(const QString &dirName);

public slots:
  void horizontalChanged(int horizontal);
  void verticalChanged(int vertical);
  void sizeChanged(int size);
  void widthChanged(int width);
  void intensityChanged(int intensity);
  void typeChanged(int type);
  void choosePageBelow();
  void belowChanged();
  void colorChanged(QColor color);
  void borderButtonClicked();
  void cornerButtonClicked();
  void chooseShadow();
  void setTransparent();
  void chooseColor();
  void previousClicked();
  void nextClicked();
  void updatePatterns();
  void advancedOptionsClicked();
  void showAdvancedOptions();
  void hideAdvancedOptions();
  void updateSliders();
  void setHole();
  void generateHoles();
  void removeHole();
  void updateZoom();

protected:
  void sideChanged(dc::HoleDegradation::HoleSide side);

  void changeEvent(QEvent *e) override;
  void setupGUIImages();
  void updateResultImage();

  //void updateSliders();

private:
  
  class Hole
  {
    
  public :
    
    Hole(int posX, int posY, int size, const QColor &color, const QImage &pattern) :
      _posX(posX), _posY(posY), _width(pattern.width()+ size), _height(pattern.height()+size),
      _size(size), _color(color), _pattern(pattern)
    { }
    
    int getX() const
    {
      return _posX;
    }
    
    int getY() const
    {
      return _posY;
    }
    
    int getWidth() const
    {
      return _width;
    }
    
    int getHeight() const
    {
      return _height;
    }
    
    int getSize() const
    {
      return _size;
    }
    
    const QColor &getColor() const
    {
      return _color;
    }
    
    const QImage &getPattern() const
    {
      return _pattern;
    }
  
  private :
    int _posX;
    int _posY;
    int _width;
    int _height;
    int _size;
    QColor _color;
    QImage _pattern;
  };

private:
  bool containsHole(const QList<Hole> &holes,
		    const QImage &pattern,
		    int horizontal,
		    int vertical,
		    int size);


private:
  Ui::HoleDegradationDialog *ui;
  int _pattern;
  int _horizontal;
  int _vertical;
  int _size;
  dc::HoleDegradation::HoleType _type;
  dc::HoleDegradation::HoleSide _side;
  int _width;
  float _intensity;
  QColor _color;

  QList<Hole> _holes;
  QStringList _patterns;

  QLabel *_originalLabel;
  QLabel *_resultLabel;

  QImage _patternImg;
  QImage _pageBelow;
  QImage _degradedImg;

  QImage _originalImg;
  QImage _resultImg;
  QImage _originalImgSmall;
  QImage _resultImgSmall;
};

#endif // HOLEDEGRADATIONDIALOG_HPP
