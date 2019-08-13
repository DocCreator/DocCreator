#include "connectedcomponentextraction.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <stack>

#include <QDebug>
#include <QImage>
#include <QString>

#include <opencv2/imgproc/imgproc.hpp>

#include <Utils/convertor.h>
#include <models/doc/doccomponent.h>

//static const uchar BACKGROUND = 255;


int
ConnectedComponentExtraction::getLabelOfPixel(
  const std::vector<LabelledPixel *> &listLabelledPixels,
  cv::Point p)
{
  int label = -1;
  const size_t s = listLabelledPixels.size();
  for (size_t i = 0; i < s; ++i) {
    LabelledPixel *pCurrent = listLabelledPixels[i];
    if (p.x == pCurrent->pixel.x && p.y == pCurrent->pixel.y) {
      label = pCurrent->label;
      break;
    }
  }
  return label;
}

void
ConnectedComponentExtraction::updatePixelLabel(
  std::vector<LabelledPixel *> listLabelledPixels,
  cv::Point p,
  int newLabel)
{
  const size_t s = listLabelledPixels.size();
  for (size_t i = 0; i < s; ++i) {
    LabelledPixel *pCurrent = listLabelledPixels[i];
    if (p.x == pCurrent->pixel.x && p.y == pCurrent->pixel.y) {
      pCurrent->label = newLabel;
    }
  }
}

void
ConnectedComponentExtraction::insertToStack(int x,
                                            int y,
                                            const cv::Mat &src,
                                            QList<QString> &stack)
{
  int scope = 2;
  for (int yk = -scope; yk <= scope; ++yk) {
    for (int xk = -scope; xk <= scope; ++xk) {
      if ((x + xk) < src.cols && (x + xk) >= 0 && (y + yk) < src.rows &&
          (y + yk) >= 0) {
        if (src.at<uchar>((y + yk), (x + xk)) == 0) {
          QString xyk(
            QString::number(x + xk) + ',' +
            QString::number(y + yk)); //B: why pass by QString ?????????
          if (!stack.contains(xyk))
            stack.push_back(xyk);
        }
      }
    }
  }
}

#if 0

//B: Cuong's version is awfully slow... 

CCs ConnectedComponentExtraction::oilSpreadFunction(const cv::Mat &src)
{
  assert(src.type() == CV_8UC1);

  //B: this is region growing !!!

  CCs ccs;
  QList<QString> listCheckedForegroundPixels;
  
  for (int y =0; y<src.rows; ++y) {
    for (int x = 0; x<src.cols; ++x) {
      if (src.at<uchar>(y, x) == 0) {
	QString xy(QString::number(x) + ',' + QString::number(y));  //B: why pass by QString ?????????
	
	if (!listCheckedForegroundPixels.contains(xy)) {

	  QList<QString> stack;
	  CC cc;
	  //B:TODO:OPTIM: reserve ???
	  
	  //
	  insertToStack(x, y, src, stack);
	  
	  // spread
	  while (!stack.isEmpty()) {
	    
	    QString txy = stack.last();
	    if (!listCheckedForegroundPixels.contains(txy)) {
	      QStringList x_y = txy.split(',');
	      
	      int tx = x_y.at(0).toInt();
	      int ty = x_y.at(1).toInt();
	      
	      insertToStack(tx, ty, src, stack);
	      
	      cc.push_back(cv::Point(tx, ty));
	      listCheckedForegroundPixels.push_back(txy);
	    }
	    stack.removeLast();
	  }
	  //
	  
	  if (cc.size() > 1) {
	    ccs.push_back(cc);
	    //                        qDebug() << " cc ID " << id_CC << " size = " << CC.size();
	  }
	}
      }
    }
  }
  listCheckedForegroundPixels.clear();
  
  return ccs;
}

#else

//B: Boris's version
// Faster, but gives a slightly different result.

//B:TODO: 'SCOPE' & 'MIN_SIZE' should be parameters of the function !!!

CCs
ConnectedComponentExtraction::oilSpreadFunction(const cv::Mat &src)
{
  assert(src.type() == CV_8UC1);

  //B: this is region growing !!!

  CCs ccs;
  ccs.reserve(src.rows); //arbitrary size

  cv::Mat visited = cv::Mat::zeros(src.rows, src.cols, src.type());

  for (int y = 0; y < src.rows; ++y) {
    const unsigned char *s = src.ptr<unsigned char>(y);
    unsigned char *v = visited.ptr<unsigned char>(y);
    for (int x = 0; x < src.cols; ++x) {
      if (s[x] == 0 && v[x] == 0) {

        v[x] = 255;

        CC cc;

        const int SCOPE = 2;

        std::stack<cv::Point, std::vector<cv::Point>> stack;
        stack.push(cv::Point(x, y));

        while (!stack.empty()) {

          cv::Point p = stack.top();
          stack.pop();

          cc.push_back(p);

          const int xmin = std::max(p.x - SCOPE, 0);
          const int xmax = std::min(p.x + SCOPE, src.cols - 1);
          const int ymin = std::max(p.y - SCOPE, 0);
          const int ymax = std::min(p.y + SCOPE, src.rows - 1);

          for (int yk = ymin; yk <= ymax; ++yk) {
            const uchar *sr = src.ptr<uchar>(yk);
            uchar *vi = visited.ptr<uchar>(yk);
            for (int xk = xmin; xk <= xmax; ++xk) {
              if (sr[xk] == 0 && vi[xk] == 0) {
                stack.push(cv::Point(xk, yk));
                vi[xk] = 255;
              }
            }
          }
        }

        const int MIN_SIZE = 2;
        if (cc.size() >= MIN_SIZE) {
          ccs.push_back(cc);
        }
      }
    }
  }

  return ccs;
}

#endif //0 //Boris version


QList<Doc::DocComponent *>
ConnectedComponentExtraction::getListComponents(const QImage &src,
                                                Doc::Document *document)
{
  QList<Doc::DocComponent *> listComponents;
  //int connectivity  = 8;
  int exWindow = 1; //B: why do we need this ???

  cv::Mat input =
    Convertor::getCvMat(src.copy(0, 0, src.width(), src.height()));
  
  cv::Mat img_binarisation = Convertor::binarizeOTSU(input);

  CCs ccs = oilSpreadFunction(img_binarisation);

  //    cv::imwrite("checker.png",checker);
  /**/

  // ise::ConnectedComponentTools::extractAllConnectedComponents(img_binarisation,
  // ccs, connectivity);
  /**
    int CC_counter = 0;
    std::vector<LabelledPixel*> listLabelledPixels;
    for (int y = 1; y < src.height() - 1 ; y++)
        for (int x = 1; x < src.width() - 1 ; x++){
            if (img_binarisation.at<uchar>(y, x) == 0){
                int labelOfThisPixel = -1;

                int labelAbovePixel = -1;
                if (img_binarisation.at<uchar>(y - 1, x) == 0){// label of the above pixel
                    labelAbovePixel = getLabelOfPixel(listLabelledPixels, cv::Point(x, y - 1));
                }
                int labelLeftPixel = -1;
                if (img_binarisation.at<uchar>(y, x - 1) == 0){// label of the left pixel
                    labelLeftPixel = getLabelOfPixel(listLabelledPixels, cv::Point(x - 1, y));
                }

                int label_min = -1;

                if(labelLeftPixel !=-1) label_min = labelLeftPixel;

                if(label_min!=-1){
                    if(labelAbovePixel !=-1 && label_min>labelAbovePixel) label_min = labelAbovePixel;
                } else { if(labelAbovePixel !=-1) label_min = labelAbovePixel;}

                if(label_min !=-1) labelOfThisPixel = label_min;
                else{
                    CC_counter++;
                    labelOfThisPixel = CC_counter;
                }
                //
                LabelledPixel* p = new LabelledPixel();
                p->pixel.x = x;
                p->pixel.y = y;
                p->label = labelOfThisPixel;
                listLabelledPixels.push_back(p);
            }
        }

    for (int y = src.height() - 2; y >= 1; y--)
        for (int x = src.width() - 2; x >= 1; x--){
            if (img_binarisation.at<uchar>(y, x) == 0){
                int labelOfThisPixel = getLabelOfPixel(listLabelledPixels, cv::Point(x, y));

                int labelOfRightPixel = getLabelOfPixel(listLabelledPixels, cv::Point(x + 1, y));
                int labelOfBottomPixel = getLabelOfPixel(listLabelledPixels, cv::Point(x, y + 1));

                if (labelOfRightPixel != -1 && labelOfThisPixel > labelOfRightPixel) labelOfThisPixel = labelOfRightPixel;
                if (labelOfBottomPixel != -1 && labelOfThisPixel > labelOfBottomPixel) labelOfThisPixel = labelOfBottomPixel;
                updatePixelLabel(listLabelledPixels, cv::Point(x, y), labelOfThisPixel);
            }
        }

    // build CCs List
    cv::Mat checker = cv::Mat::zeros(input.rows, input.cols,CV_8U);
    for(int x=0; x<checker.cols;x++)
        for(int y=0;y<checker.rows;y++){
            checker.at<uchar>(y, x) = 255;
        }

    for (int i = 0; i <= CC_counter; i++){
        ConnectedComponent lst;
        for (int k = 0; k < listLabelledPixels.size(); k++){
            LabelledPixel* p = listLabelledPixels.at(k);
            if (p->label == i) {
                lst.push_back(cv::Point(p->pixel.x, p->pixel.y));
            }
        }
        if(lst.size()>0) ccs.push_back(lst);
    }

    for(int i = 0;i<ccs.size();i++){
        ConnectedComponent cc = ccs.at(i);
        if(cc.size()>10)
            for(int j=0;j<cc.size();j++){
                checker.at<uchar>(cc.at(j).y,cc.at(j).x) = 0;
            }
        else {
            for(int j=0;j<cc.size();j++){
                checker.at<uchar>(cc.at(j).y,cc.at(j).x) = 170;
            }
        }
    }
    cv::imwrite("checker.png",checker);

    for (int k = 0; k < listLabelledPixels.size(); k++){
        LabelledPixel* p = listLabelledPixels.at(k);
        delete p;
    }
    listLabelledPixels.clear();

    */

  qDebug() << "nb composant = " << ccs.size();

  const size_t sz = ccs.size();
  listComponents.reserve(sz);
  for (size_t i = 0; i < sz; ++i) {

    CC cc = ccs[i];
    int max_X = INT_MIN;
    int max_Y = INT_MIN;
    int min_X = INT_MAX;
    int min_Y = INT_MAX;
    const size_t s = cc.size();
    for (size_t j = 0; j < s; ++j) {

      cv::Point p = cc[j];
      max_X = std::max(max_X, p.x);
      max_Y = std::max(max_Y, p.y);
      min_X = std::min(min_X, p.x);
      min_Y = std::min(min_Y, p.y);
    }
    //int surface = (max_X - min_X)*(max_Y - min_Y);

    max_X += exWindow;
    min_X -= exWindow;
    max_Y += exWindow;
    min_Y -= exWindow;
    //        if(surface <10 || (min_X<0) || min_Y<0 ||
    //        surface>(src.width()*src.height()/4)) continue;
    //if(surface <5 || (min_X<0) || min_Y<0 || surface>(src.width()*src.height()/4)) continue;
    //        qDebug() << i << " min " << min_X << " " << min_Y << " max " <<
    //        max_X << " " << max_Y << " sf = " << surface;

    //const QMap<int, std::vector<int> > scanlines = getScanlines(cc, min_X, max_X, min_Y, max_Y);
    const QMap<int, std::vector<int>> scanlines = getScanlinesB(cc);

    //qDebug() << scanlines.size();

    auto component = new Doc::DocComponent(scanlines, i, document);
    component->setX(min_X);
    component->setY(min_Y);
    component->setHeight(max_Y - min_Y); //B:wrong ? +1 ?
    component->setWidth(max_X - min_X);  //B:wrong ? +1 ?

    listComponents.append(component);
    //break;
  }

  qDebug() << "nb accepted composant = " << listComponents.size();
  return listComponents;
}

/*
  //B: very slow !!!
  Why do we traverse bounding box instead of just CC ???

  see getScanlinesB().

 */
QMap<int, std::vector<int>>
ConnectedComponentExtraction::getScanlines(const CC &cc,
                                           int min_X,
                                           int max_X,
                                           int min_Y,
                                           int max_Y)
{
  QMap<int, std::vector<int>> scanlines;
  cv::Point p;

  std::vector<int> lines;
  for (int y = min_Y; y <= max_Y; ++y) {
    lines.clear();
    p.y = y;

    for (int x = min_X; x <= max_X; ++x) {
      p.x = x;

      if (std::find(cc.begin(), cc.end(), p) == cc.end())
        continue;

      lines.push_back(x);
      while (std::find(cc.begin(), cc.end(), p) != cc.end()) {
        ++x;
        p.x = x;
      }
      --x;
      lines.push_back(x);
    }
    scanlines.insert(y, lines);
  }
  return scanlines;
}

namespace {

struct PointSorter
{
  inline bool operator()(cv::Point p1, cv::Point p2) const
  {
    return p1.y < p2.y || (p1.y == p2.y && p1.x < p2.x);
  }
};

} //end anonymous namespace

//Boris' version
//We do not traverse BBox searching points. We only traverse CC.
//We do not keep empty lines.
QMap<int, std::vector<int>>
ConnectedComponentExtraction::getScanlinesB(const CC &cc)
{
  QMap<int, std::vector<int>> scanlines;

  const size_t sz = cc.size();

  if (sz > 0) {

    CC scc = cc;
    std::sort(scc.begin(), scc.end(), PointSorter());

    assert(scc.size() == sz);

    int prev_y = scc[0].y - 1; //inferior to all ys of cc.
    int start_x = 0;
    int end_x = 0;
    std::vector<int> lines;
    for (size_t i = 0; i < sz; ++i) {
      const int x = scc[i].x;
      const int y = scc[i].y;
      if (y > prev_y) {
        if (!lines.empty()) {
          lines.push_back(end_x);
          scanlines.insert(prev_y, lines);
          lines.clear();
        }
        prev_y = y;
        start_x = x;
        end_x = x;
        lines.push_back(start_x);
      } else {
        if (x == end_x + 1) {
          end_x = x;
        } else {
          assert(x > end_x + 1);
          lines.push_back(end_x);
          start_x = x;
          lines.push_back(start_x);
          end_x = x;
        }
      }
    }
    if (!lines.empty()) {
      lines.push_back(end_x);
      scanlines.insert(prev_y, lines);
    }
  }

  return scanlines;
}
