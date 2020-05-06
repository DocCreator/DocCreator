#ifndef DOCUMENTPROPERTIESVIEW
#define DOCUMENTPROPERTIESVIEW

#include <QWidget>

#include "mvc/icontroller.h"
#include "mvc/iview.h"
#include "patterns/observer.h"

class DocumentController;
class QIntValidator;
class QLineEdit;
class QCheckBox;

class DocumentPropertiesView
  : public QWidget
  , public Mvc::IView
  , public Patterns::Observer
{
  Q_OBJECT

public:
  explicit DocumentPropertiesView(DocumentController *controller,
                                  QWidget *parent = nullptr);

  /* Events */
  void keyPressEvent(QKeyEvent *e) override;
  void keyReleaseEvent(QKeyEvent *e) override;

  Mvc::IController *getController() override;

  /* Observer method */
  void update() override;

public slots:
  void changePageWidth();
  void changePageHeight();
  void changeLineSpacing();
  void changeMarginTop();
  void changeMarginBottom();
  void changeMarginLeft();
  void changeMarginRight();
  void changeBaseLineVisibility();

private:
  QString getVisibilityString() const;

private:
  DocumentController *_controller;

  QLineEdit *_pageWidth;
  QLineEdit *_pageHeight;
  QLineEdit *_lineSpacingLineEdit;
  QLineEdit *_marginTopLineEdit;
  QLineEdit *_marginBottomLineEdit;
  QLineEdit *_marginLeftLineEdit;
  QLineEdit *_marginRightLineEdit;
  QCheckBox *_baseLineVisibleCheckBox;

  QIntValidator *_intValidator;
};

#endif
