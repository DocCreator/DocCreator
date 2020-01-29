#include "GraphicView.hpp"

#include <cassert>
#include <iostream>

#include <context.h>
#include <core.h>

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QMessageBox>
#include <QPainter>
#include <QPair>
#include <QPen>
#include <QPrinter>

#include "appconstants.h"
//#include "Document/LabelingComponentDialog.hpp"
//#include "Document/ChooseLabelForComponentForm.hpp"
#include "DocumentController.hpp"
#include "DocumentView.hpp"
#include "GraphicsBlockItem.hpp"
#include "GraphicsPageItem.hpp"
#include "Utils/ImageUtils.hpp" //toGray

GraphicView::GraphicView(Mvc::IController *controller, DocumentView *parent)
  : QGraphicsView()
  , ADocumentView<Doc::Document>(controller)
  , _parent(parent)
  , _newFont(nullptr)
  , _map()
  , _mousePressedPosition(-1, -1)
  , _mouseReleasedPosition(-1, -1)
  , _zoomScaleIndex(1)
  , _isMousePressed(false)
  , m_cachedW(0)
  , m_cachedH(0)
{
  auto _scene = new QGraphicsScene(this);
  _scene->setItemIndexMethod(QGraphicsScene::NoIndex);

  setScene(_scene);

  setBackgroundBrush(QBrush(QColor(192, 192, 192)));
}

//bool GraphicView::eventFilter(QObject *obj, QEvent *ev){
//    if (obj!=this){
//        qDebug() << " in scene : " << ev->type();
//        if (ev->type()==QEvent::MouseButtonPress) qDebug() << "press";
//        if (ev->type()==QEvent::MouseButtonRelease) qDebug() << "release";
//        return true;
//    }
//    else {
//        qDebug() << " OUT scene : "<< ev->type();
//        return QWidget::eventFilter(obj, ev);
//    }

//}

void
GraphicView::clear()
{
  //_map.clear();
}

void
GraphicView::setOffset(int value)
{
  GraphicsPageItem *current = currentPageItem();
  if (current != nullptr) {
    current->setOffset(value);
  }
}

void
GraphicView::print(QPrinter *printer)
{
  QImage image = getDocumentImage(Color | WithTextBlocks | WithImageBlocks);
  QPainter painter(printer);
  QRect rect = painter.viewport();
  QSize size = image.size();
  size.scale(rect.size(), Qt::KeepAspectRatio);
  painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
  painter.setWindow(image.rect());
  painter.drawImage(0, 0, image);
}

void
GraphicView::saveToImage(const QString &filepath)
{
  const QImage image =
    getDocumentImage(Color | WithTextBlocks | WithImageBlocks);

  image.save(filepath, "PNG");
  // Save it as binary TIF
  //    QImage binary = getDocumentBinaryGTImage();
  //    QString binaryPath = filepath;binaryPath.append("_binary");
  //    binary.save(binaryPath,"TIF");
}

QImage
GraphicView::toQImage(DocRenderFlags flags)
{
  return getDocumentImage(flags);
}

QSize
GraphicView::getImageSize()
{
  GraphicsPageItem *currentPage = currentPageItem();
  if (currentPage == nullptr) {
    return QSize();
  }

  return currentPage->boundingRect().toRect().size();
}

/* Events */
//void GraphicView::mousePressEvent(QMouseEvent *event){

//    _isMousePressed = true;
//    _mousePressedPosition.setX(event->pos().x());
//    _mousePressedPosition.setY(event->pos().y());
//}
//void GraphicView::mouseReleaseEvent(QMouseEvent *event){

//    if (!_isMousePressed) {
//        _mousePressedPosition.setX(-1);
//        _mousePressedPosition.setY(-1);
//        _mouseReleasedPosition.setX(-1);
//        _mouseReleasedPosition.setX(-1);
//        return;}

//    _mouseReleasedPosition.setX(event->pos().x());
//    _mouseReleasedPosition.setY(event->pos().y());

//    int max_x = _mousePressedPosition.x();
//    int max_y = _mousePressedPosition.y();
//    int min_x = _mouseReleasedPosition.x();
//    int min_y = _mouseReleasedPosition.y();

//    if (_mousePressedPosition.x()<_mouseReleasedPosition.x()){
//        max_x = _mouseReleasedPosition.x();
//        min_x = _mousePressedPosition.x();
//    }
//    if (_mousePressedPosition.y()<_mouseReleasedPosition.y()){
//        max_y = _mouseReleasedPosition.y();
//        min_y = _mousePressedPosition.y();
//    }
//    if (max_x<0 && max_y <0) return;
//    qDebug() << "min(x, y) = " << min_x << ", " << min_y;
//    qDebug() << "max(x, y) = " << max_x << ", " << max_y;
//    QList<DocComponentBlock*> chosenComponentBlocks;

//    DocumentController * docController = (DocumentController*)getController();
//    if (docController != nullptr)
//    {
//       QList<DocComponentBlock*> componentsBlock=
//       docController->getDocument()->currentPage()->getComponentBlocks();
//       for (DocComponentBlock* dcb : componentsBlock){

//           if (dcb->x()>= min_x && dcb->x() <= max_x && dcb->y()>= min_y &&
//           dcb->y()<=max_y)
//               chosenComponentBlocks.push_back(dcb);
//       }
//    }
//    if (chosenComponentBlocks.size()>0){
//        // labeling
//        LabelingComponentDialog* labelDialog = new
//        LabelingComponentDialog(this);
//        if (labelDialog->exec()){
//            BlockType blockType = labelDialog->getBlockType();
//            if (blockType == TextBlock){
//                if (_newFont==nullptr) _newFont = new Models::Font("Font from
//                image document");
//                ChooseLabelForComponentForm* chooseLabelForm = new
//                ChooseLabelForComponentForm( docController,_newFont,
//                chosenComponentBlocks);
//                chooseLabelForm->show();

//            }
//            else if (blockType == ImageBlock){

//                QString path =
//                Context::BackgroundContext::instance()->getPath() +
//                Context::BackgroundContext::instance()->getCurrentBackground();
//                QImage tmp_bg(path);
//                QImage
//                tmp(tmp_bg.scaled((docController->toQImage())->width(),(docController->toQImage())->height()));
//                int i = 0;

//                for (DocComponentBlock* dcb : chosenComponentBlocks){
//                    QImage imgBlock = tmp.copy(dcb->x(), dcb->y(),
//                    dcb->width(), dcb->height());
//                    QString path
//                    ="/home/kvcuong/AncientDocumentCreator/trunk/documentImageCreator/ImageBlockData/imageblock_extracted_";
//                    path.append(QString::number(i));
//                    path.append(".png");
//                    imgBlock.save(path);
//                    //
//                    DocImageBlock* imageBlock = new DocImageBlock(path);
//                    docController->getDocument()->currentPage()->add(imageBlock);
//                    i++;
//                }
//                docController->setModified();
//            }
//            else if (blockType == BackGround){
//                for (DocComponentBlock* dcb : chosenComponentBlocks)
//                    docController->getDocument()->currentPage()->remove(dcb);
//                 docController->setModified();
//            }
//            else if (blockType == Other){

//            }
//            else{

//            }
//        }
//    }
//    _isMousePressed = false;
//    _mousePressedPosition.setX(-1);
//    _mousePressedPosition.setY(-1);
//    _mouseReleasedPosition.setX(-1);
//    _mouseReleasedPosition.setX(-1);

//    chosenComponentBlocks.clear();

//    qDebug() << " nb of component blocks = " <<
//    (docController->getDocument()->currentPage()->getComponentBlocks()).size();
//}

void
GraphicView::wheelEvent(QWheelEvent *event)
{
  if (event->delta() > 0) {
    if (_zoomScaleIndex < MAX_ZOOM_GRAPHICVIEW) {
      zoomIn();
    }
  }
  else {
    if (_zoomScaleIndex > -MAX_ZOOM_GRAPHICVIEW) {
      zoomOut();
    }
  }
}

void
GraphicView::keyPressEvent(QKeyEvent *event)
{
  if (_parent != nullptr) {
    _parent->keyPressEvent(event);
  }
}

void
GraphicView::keyReleaseEvent(QKeyEvent *event)
{
  if (_parent != nullptr) {
    _parent->keyReleaseEvent(event);
  }
}

/* Slots */
void
GraphicView::zoomIn()
{
  scale(1.2, 1.2);
  _zoomScaleIndex++;
}

void
GraphicView::zoomOut()
{
  scale(1 / 1.2, 1 / 1.2);
  _zoomScaleIndex--;
}

/* protected methods */
void
GraphicView::load()
{
  clear();

  Doc::Document *document = getElement();
  if (document == nullptr) {
    return;
  }

  for (Doc::Page *p : document->getPages()) {
    addPage(p);
  }
}

void
GraphicView::draw(bool complete)
{
  Doc::Document *document = getElement();
  if (document == nullptr) {
    return;
  }

  synchroniseWithElement();

  if (complete) {
    //QMessageBox::information(nullptr, "COMPLETE", "redraw");
    //QList<Doc::Page*> list = _map.keys();
    //for (Doc::Page* p : list) {
    for (auto it = _map.begin(); it != _map.end(); ++it) {
      Doc::Page *p = it.key();
      GraphicsPageItem *pageItem = _map.value(p);
      pageItem->drawElement(p, complete);
    }
  }
  else {
    GraphicsPageItem *currentPage = currentPageItem();
    if (currentPage != nullptr) {
      currentPage->drawElement(document->currentPage(), complete);
    }
    //QMessageBox::information(nullptr, "PARTIAL", "redraw");
  }
}

/* private methods */
/*
QImage GraphicView::getDocumentBinaryGTImage()
{
  GraphicsPageItem* currentPage = currentPageItem();
  if (currentPage == nullptr)
    return QImage();

  // Hide blocks rects
  for (GraphicsBlockItem* blockItem : currentPage->getGraphicsBlockItems())
    blockItem->hide(true);
  
  // Create the image and render it...
  const int w = currentPage->pixmap().width();
  const int h = currentPage->pixmap().height();

  //        DocumentController *controller = (DocumentController*)getController();
  //        if (controller!=nullptr)
  //        {
  //                w = controller->getPageWidth();
  //                h = controller->getPageHeight();
  //                qDebug() << " Controller w, h = " << w << ", " << h;
  //        }

  QImage binaryGT(w, h, QImage::Format_RGB16);
  binaryGT.fill(qRgb(255, 255, 255));

  DocumentController *controller = static_cast<DocumentController*>(getController());//B:dynamic_cast?
  Doc::Page* currentDocPage = controller->getDocument()->currentPage();
  Context::FontContext *fontContext = Context::FontContext::instance();

  for (Doc::DocTextBlock* tb : currentDocPage->getTextBlocks()) {
    int tb_x = tb->x();
    int tb_y = tb->y();
    for (Doc::DocParagraph * p : tb->getParagraphs()) {
      for (Doc::DocString * s : p->getStrings()) {
        Doc::DocStyle * style = s->getStyle();
        Models::Font* currentFont = fontContext->getFont(style->getFontName());
        for (Doc::DocCharacter * c : s->getCharacters()) {
          const int box_char_x = c->x();
          const int box_char_y = c->y();
          if (c->getDisplay() !="\n" && c->getDisplay()!=" ") { //B:TODO: have a function isSpace() ???
            Models::Character* character = currentFont->getCharacter(c->getDisplay());
            const Models::CharacterData* charData = character->getCharacterData(c->getId());
            const QImage charImage = charData->getImage();
            //B:TODO:OPTIM: do no use setPixel()/pixel() & remove checks from inside loops
            const int h = charImage.height();
            const int w = charImage.width();
            for (int y=0; y<h; ++y) {
              for (int x=0; x<w; ++x) {
                const int gray = qGray(charImage.pixel(x, y));//B:TODO:OPTIM: instead of qGray we could use the sum of 3 channels ?
                const int imgX = (tb_x + box_char_x + x);
                const int imgY = (tb_y + box_char_y + y);
                if (gray!=0 && imgX<binaryGT.width() && imgY<binaryGT.height() && imgX>=0 && imgY >=0)//B:TODO:OPTIM: do not check y in inner loop
                  binaryGT.setPixel(imgX, imgY, qRgb(0, 0, 0));
              }
            }
          }
        }
      }
    }
  }
    
  // Show blocks rects
  for (GraphicsBlockItem* blockItem : currentPage->getGraphicsBlockItems())
    blockItem->hide(false);
    
  return binaryGT;
}
*/

//#define TIMING 1
#ifdef TIMING
#include <chrono>
#endif //TIMING

/*
  Copy image @a charImage at position (@a cx0, @a cy0) in @a outImg.

*/
static void
copyImage(QImage &outImg, const QImage &charImage, int cx0, int cy0)
{
  assert(outImg.depth() == 32);

  const int w = outImg.width();
  const int h = outImg.height();

  const int cW = charImage.width();
  const int cH = charImage.height();

  const int y0 = std::max(0, cy0);
  const int y1 = std::min(cy0 + cH, h);
  const int x0 = std::max(0, cx0);
  const int x1 = std::min(cx0 + cW, w);

  if (charImage.depth() == 32) {

    for (int y = y0; y < y1; ++y) {
      const int cy = y - cy0;

      const QRgb *s =
        reinterpret_cast<const QRgb *>(charImage.constScanLine(cy));
      QRgb *d = reinterpret_cast<QRgb *>(outImg.scanLine(y));

      for (int x = x0; x < x1; ++x) {
        const int cx = x - cx0;
        assert(cx >= 0 && cy >= 0 && cx < cW && cy < cH);
        assert(x >= 0 && y >= 0 && x < w && y < h);

        const QRgb cPix = s[cx];
        const int a = qAlpha(cPix);
        if (a != 0)
          d[x] = cPix;
      }
    }

  }
  else {

    for (int y = y0; y < y1; ++y) {
      const int cy = y - cy0;
      for (int x = x0; x < x1; ++x) {
        const int cx = x - cx0;
        assert(cx >= 0 && cy >= 0 && cx < cW && cy < cH);
        assert(x >= 0 && y >= 0 && x < w && y < h);
        const QRgb cPix = charImage.pixel(cx, cy);
        const int a = qAlpha(cPix);
        if (a != 0) {
          outImg.setPixel(
            x,
            y,
            cPix); //B: we do not take destination pixel into account ... ?
	}
      }
    }
  }
}

QImage
GraphicView::getDocumentImage(DocRenderFlags flags)
{
  //std::cerr << "GraphicView::getDocumentImage()\n";

  GraphicsPageItem *currentPage = currentPageItem();
  if (currentPage == nullptr) {
    return QImage();
  }

  // Hide blocks rects
  for (GraphicsBlockItem *blockItem : currentPage->getGraphicsBlockItems()) {
    blockItem->hide(true);
  }

  // Create the image and render it...
  //const int w = currentPage->pixmap().width();
  //const int h = currentPage->pixmap().height();
  const QRect boundingRect = currentPage->boundingRect().toRect();
  const int w = boundingRect.width();
  const int h = boundingRect.height();

  //std::cerr << "Page w, h = " << w << ", " << h<<"\n";

  //        DocumentController *controller =
  //        (DocumentController*)getController();
  //        if (controller!=nullptr)
  //        {
  //                w = controller->getPageWidth();
  //                h = controller->getPageHeight();
  //                qDebug() << " Controller w, h = " << w << ", " << h;
  //        }
  /**NEW code for DIBCO*/

  const QString path =
    Context::BackgroundContext::instance()->getPath() +
    Context::BackgroundContext::instance()->getCurrentBackground();

  //QString m_cachedBgImagePath;
  //QImage m_cachedBgImage;

  if (path != m_cachedBgImagePath ||
      (w != m_cachedBgImage.width() || h != m_cachedBgImage.height())) {

    //qDebug() << "GraphicView::getDocumentImage() load image: "<<path;
    //std::cerr << "GraphicView::getDocumentImage() load image: "
    //          << path.toStdString() << "\n";

#ifdef TIMING
    auto t0 = std::chrono::steady_clock::now();
#endif //TIMING

    QImage bg(
      path); //B:TODO:OPTIM:ugly: how many times do we save/load the background image(s) from file ????!!!!!
    assert(!bg.isNull());

#ifdef TIMING
    auto t1 = std::chrono::steady_clock::now();
    auto time1 = t1 - t0;
    std::cerr << "time load(): "
              << std::chrono::duration<double, std::milli>(time1).count()
              << "ms\n";
    auto t2 = std::chrono::steady_clock::now();
#endif //TIMING

    bg = bg.scaled(
      w, h); //B:TODO:UGLY: we should scale content, not background image...

#ifdef TIMING
    auto t3 = std::chrono::steady_clock::now();
    auto time2 = t3 - t2;
    std::cerr << "time scale(): "
              << std::chrono::duration<double, std::milli>(time2).count()
              << "ms\n";
    //auto t4 = std::chrono::steady_clock::now();
#endif //TIMING

    bg = bg.convertToFormat(QImage::Format_ARGB32);

    assert(bg.width() == w && bg.height() == h);

    m_cachedBgImagePath = path;
    m_cachedBgImage = bg;
  }

  //QImage outImg = bg;
  QImage outImg = m_cachedBgImage;

  // if (! flags.testFlag(Color))
  // outImg = toGray(bg);

#ifdef TIMING
  //auto t5 = std::chrono::steady_clock::now();
  //auto time3 = t5-t4;
  //std::cerr<<"time toGray(): "<<std::chrono::duration<double, std::milli>(time3).count()<<"ms\n";
  //size_t DBG_numChars = 0;
  auto t6 = std::chrono::steady_clock::now();
#endif //TIMING

  Context::FontContext *fontContext = Context::FontContext::instance();
  DocumentController *controller =
    static_cast<DocumentController *>(getController()); //B:dynamic_cast?
  Doc::Page *currentDocPage = controller->getDocument()->currentPage();

  if (flags.testFlag(WithTextBlocks)) {

    for (Doc::DocTextBlock *tb : currentDocPage->getTextBlocks()) {
      const int tb_x = tb->x();
      const int tb_y = tb->y();

      for (Doc::DocParagraph *p : tb->getParagraphs()) {

        for (Doc::DocString *s : p->getStrings()) {

          Doc::DocStyle *style = s->getStyle();
          assert(style);
          Models::Font *currentFont =
            fontContext->getFont(style->getFontName());
          assert(currentFont);
          for (Doc::DocCharacter *c : s->getCharacters()) {
            //const int box_char_x = c->x();
            //const int box_char_y = c->y();
            //qDebug() << " tb x y " << tb_x << " " << " " << tb_y << " ch x y " << c->x() << " " << c->y();
            const QString cDisplay = c->getDisplay();
            if (
              cDisplay != QLatin1String("\n") &&
              cDisplay !=
                QLatin1String(
                  " ")) { //B:TODO:CODE: we should have a method isSpace() or equivalent.

              const Models::Character *character =
                currentFont->getCharacter(cDisplay);
              const Models::CharacterData *charData =
                character->getCharacterData(c->getId());
              const QImage charImage = charData->getImage();

#ifndef NEBUG
              if (charImage.isNull())
                std::cerr << "charImage.isNull() !!!!! character=" << character
                          << " charData=" << charData << " \n";
#endif //NDEBUG

              if (
                !charImage
                   .isNull()) { //B:TODO:OPTIM: possible ? should be an assert ???  //possible if wrong font ?

#ifdef TIMING
                ++DBG_numChars;
#endif //TIMING

                const int cx0 = tb_x + c->x();
                const int cy0 = tb_y + c->y();

                copyImage(outImg, charImage, cx0, cy0);
              }
            }
          }
        }
      }
    }
  }

  if (flags.testFlag(WithImageBlocks)) {

    for (Doc::DocImageBlock *ib : currentDocPage->getImageBlocks()) {
      const QString imagePath = ib->filePath();

      QImage img(imagePath);
      img = img.scaled(ib->width(), ib->height());

      copyImage(outImg, img, ib->x(), ib->y());
    }
  }

#ifdef TIMING
  auto t7 = std::chrono::steady_clock::now();
  auto time4 = t7 - t6;
  std::cerr << "time draw text & image blocks: "
            << std::chrono::duration<double, std::milli>(time4).count()
            << "ms (" << DBG_numChars << " characters)\n";
#endif //TIMING

  /**/
  /**OLD code*
     QImage outImg(w, h, QImage::Format_ARGB32);
     QPainter painter(&outImg);
     painter.setRenderHint(QPainter::Antialiasing);
     scene()->render(&painter);
     painter.end();
  */
  // Show blocks rects
  for (GraphicsBlockItem *blockItem : currentPage->getGraphicsBlockItems())
    blockItem->hide(false);

  return outImg;
}

GraphicsPageItem *
GraphicView::currentPageItem()
{
  Doc::Document *d = getElement();
  if (d == nullptr) {
    return nullptr;
  }

  return _map.value(d->currentPage());
}

void
GraphicView::synchroniseWithElement()
{
  Doc::Document *document = getElement();
  if (document == nullptr) {
    return;
  }
  /* removing items that doesn't exist on the document */
  QList<Doc::Page *> pages = document->getPages();
  QList<Doc::Page *> listToRemove;
  listToRemove.reserve(_map.size());
  //for (Doc::Page* page : _map.keys()) {
  for (auto it = _map.begin(); it != _map.end(); ++it) {
    Doc::Page *page = it.key();
    if (!pages.contains(page))
      listToRemove.append(page);
  }
  for (Doc::Page *page : listToRemove)
    removePage(page);
  //B:Why in two steps ? First store pages to remove and then remove them ???
  //  because removePage change _map and would invalidate iterators ?!

  /* adding items that exists on the document and not on graphic */
  pages =
    _map
      .keys(); //B:TODO:OPTIM: is the "copy" necessary here ? could we do _map.contains(block) in the loop ??? [similar code in graphicspageitem.cpp]
  for (Doc::Page *page : document->getPages()) {
    if (!pages.contains(page))
      addPage(page);
  }
}

void
GraphicView::addPage(Doc::Page *p)
{
  auto graphicPage = new GraphicsPageItem(getController());

  if (p->getBackgroundFileName().isEmpty()) {
    const QString bgFileName =
      Context::BackgroundContext::instance()->getCurrentBackground();
    p->setBackgroundFileName(bgFileName);
  } else {
    Context::BackgroundContext::instance()->setCurrentBackground(
      p->getBackgroundFileName());
  }
  const QString bgPath = Context::BackgroundContext::instance()->getPath() +
                         p->getBackgroundFileName();
  const int w =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigPageSizeX)
      .toInt();
  const int h =
    Core::ConfigurationManager::get(AppConfigMainGroup, AppConfigPageSizeY)
      .toInt();

  graphicPage->setBackground(bgPath, w, h);
  Context::BackgroundContext::instance()->setChanged(false);

  scene()->addItem(graphicPage);
  graphicPage->setZValue(-100);

  _map.insert(p, graphicPage);
}

void
GraphicView::removePage(Doc::Page *p)
{
  GraphicsPageItem *gp = _map.value(p);
  scene()->removeItem(gp);
  gp->setParentItem(nullptr);
  delete gp;
  _map.remove(p);
}
