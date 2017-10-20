#ifndef CHAREDITVIEW_H
#define CHAREDITVIEW_H

#include "mvc/icontroller.h"
#include "mvc/iview.h"
#include <QGraphicsView>

class CharEditScene;

static const int MAX_ZOOM = 100;

class CharEditView
  : public QGraphicsView
  , public Mvc::IView
{
  Q_OBJECT

public:
  CharEditView(CharEditScene *scene, Mvc::IView *parent);

  //Take over the interaction
  virtual void wheelEvent(QWheelEvent *event) override;
  virtual void keyPressEvent(QKeyEvent *event) override;
  virtual void keyReleaseEvent(QKeyEvent *event) override;
  virtual Mvc::IController *getController() override;

public slots:
  void zoomIn();
  void zoomOut();

private:
  Mvc::IView *_parent;

  CharEditScene *_scene;

  int zoomScaleIndex;
};

#endif // CHAREDITVIEW_H
