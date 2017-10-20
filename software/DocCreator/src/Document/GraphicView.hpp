#ifndef GRAPHICVIEW_H
#define GRAPHICVIEW_H

#include <QGraphicsView>

#include "ADocumentView.hpp"
#include "DocRenderFlags.hpp"
#include "mvc/icontroller.h"

class DocumentView;
class QPrinter;
class GraphicsPageItem;

static const int MAX_ZOOM_GRAPHICVIEW = 20;

class GraphicView
  : public QGraphicsView
  , public ADocumentView<Doc::Document>
{
  Q_OBJECT

public:
  GraphicView(Mvc::IController *controller, DocumentView *parent);

  virtual void clear() override;
  virtual void setOffset(int value) override;

  void print(QPrinter *printer);
  void saveToImage(const QString &filepath);
  QImage toQImage(DocRenderFlags flags);
  QSize getImageSize();

  //    bool eventFilter(QObject *obj, QEvent *ev);

  //    void mousePressEvent(QMouseEvent *event);
  //    void mouseReleaseEvent(QMouseEvent *event);

  virtual void wheelEvent(QWheelEvent *event) override;
  virtual void keyPressEvent(QKeyEvent *event) override;
  virtual void keyReleaseEvent(QKeyEvent *event) override;

public slots:
  void zoomIn();
  void zoomOut();

protected:
  virtual void load() override;
  virtual void draw(bool complete) override;

private:
  QImage getDocumentImage(DocRenderFlags flags);
  //QImage getDocumentBinaryGTImage();
  GraphicsPageItem *currentPageItem();
  void synchroniseWithElement();
  void addPage(Doc::Page *);
  void removePage(Doc::Page *);

private:
  DocumentView *_parent;
  Models::Font *_newFont;
  QMap<Doc::Page *, GraphicsPageItem *> _map;
  QPoint _mousePressedPosition;
  QPoint _mouseReleasedPosition;
  int _zoomScaleIndex;
  bool _isMousePressed;

  QString m_cachedBgImagePath;
  QImage m_cachedBgImage;
  int m_cachedW;
  int m_cachedH;
};

#endif /* GRAPHICVIEW_H */
