#ifndef BLURFILTERDIALOG_HPP
#define BLURFILTERDIALOG_HPP

#include <QDialog>
#include <QStringList>

#include "Degradations/BlurFilter.hpp" //for Method, Moe, Area, ...

class QLabel;

namespace Ui {
class BlurFilterDialog;
}

class BlurFilterDialog : public QDialog
{
  Q_OBJECT

public:
  explicit BlurFilterDialog(QWidget *parent = nullptr);

  ~BlurFilterDialog();

  BlurFilterDialog(const BlurFilterDialog &) = delete;
  BlurFilterDialog &operator=(const BlurFilterDialog &) = delete;

  const QImage &getPattern() const { return _patternImg; }

  int getIntensity() const { return _intensity; }

  float getCoeff() const { return _coeff; }

  int getVertical() const { return _vertical; }

  int getHorizontal() const { return _horizontal; }

  int getRadius() const { return _radius; }

  dc::Method getMethod() const { return _method; }

  dc::Mode getMode() const { return _mode; }

  dc::Area getArea() const { return _area; }

  dc::Function getFunction() const { return _function; }

  void setOriginalImage(const QImage &img);

  static bool fileExists(const QString &path, const QString &name);
  static QString getBlurImagesPath();
  static QString getBlurPatternsPath();
  static QString getBlurExamplesPath();
  static QStringList getDirectoryList(const QString &dirName);

public slots:
  void intensityChanged(int intensity);
  void coeffChanged(int coeff);
  void verticalChanged(int vertical);
  void horizontalChanged(int horizontal);
  void radiusChanged(int radius);
  void methodChanged(int method);
  void modeChanged(int mode);
  void areaChanged(int area);
  void functionChanged(int function);
  void previousClicked();
  void nextClicked();
  void updateExamples();
  void exampleChosen();
  void previousPatternClicked();
  void nextPatternClicked();
  void changePatterns();
  void updatePatterns();
  void patternChosen();
  void advancedOptionsClicked();
  void showAdvancedOptions();
  void hideAdvancedOptions();
  void moreSettingClicked();
  void showSettings();
  void hideSettings();
  void updateParameters();
  void savePattern();

protected:
  virtual void changeEvent(QEvent *e) override;

  void updateSliders();
  void setupGUIImages();
  void updateBlurredImage();

private:
  Ui::BlurFilterDialog *ui;
  int _exampleChosen; // for fit
  int _currentExample;
  int _currentPattern;
  int _intensity;
  float _coeff;
  int _horizontal;
  int _vertical;
  int _radius;

  dc::Method _method;
  dc::Mode _mode;
  dc::Area _area;
  dc::Function _function;

  QStringList _patterns;
  QStringList _examples;

  QLabel *_originalLabel;
  QLabel *_blurredLabel;

  QImage _patternImg;
  QImage _originalImg;
  QImage _blurredImg;
  QImage _originalImgPart;
  QImage _blurredImgPart;
  QImage _originalImgSmall;
  QImage _blurredImgSmall;
};

#endif // BLURFILTERDIALOG_HPP
