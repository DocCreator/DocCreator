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

  HoleType getType() const { return _type; }

  int getSide() const { return _side; }

  void setOriginalImage(const QImage &img);
  void findBound(HoleType type,
                 int &minH,
                 int &maxH,
                 int &minV,
                 int &maxV,
                 int side,
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
  void sideChanged(int side);
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
  virtual void changeEvent(QEvent *e) override;
  void setupGUIImages();
  void updateResultImage();

  //void updateSliders();

private:
  Ui::HoleDegradationDialog *ui;
  int _pattern;
  int _horizontal;
  int _vertical;
  int _size;
  HoleType _type;
  int _side;
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
