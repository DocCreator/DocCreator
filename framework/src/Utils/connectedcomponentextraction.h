#ifndef CONNECTEDCOMPONENTEXTRACTION_H
#define CONNECTEDCOMPONENTEXTRACTION_H

#include <vector>

#include <opencv2/core/core.hpp>

#include <QList>
#include <QMap>

#include <framework_global.h>

class QImage;
class QString;
namespace Doc {
class DocComponent;
}
namespace Doc {
class Document;
}

using CC = std::vector<cv::Point>;
using CCs = std::vector<CC>;

struct LabelledPixel
{
  cv::Point pixel;
  int label = 0;
};

/**
   Extraction of the connected components of an image, as components of a
   document.
*/
class FRAMEWORK_EXPORT ConnectedComponentExtraction
{
public:
  /**
     Extract the connected components of an image.

     The image is binarized with Otsu's method. Two dark pixels are
     connected when they are at most 2 apart, in x and in y; a component of
     a single pixel is dropped.

     @param src  image.
     @param document  document the components belong to.
     @return a new component per connected component, in the order a scan
     by rows meets them, with its pixels as scanlines. Its bounding box
     starts one pixel before the component and ends on its last row and
     column. The caller owns the components.
  */
  static QList<Doc::DocComponent *> getListComponents(const QImage &src,
                                                      Doc::Document *document);

private:
  static void insertToStack(int x,
                            int y,
                            const cv::Mat &src,
                            QList<QString> &stack);
  static CCs oilSpreadFunction(const cv::Mat &src);
  static int getLabelOfPixel(
    const std::vector<LabelledPixel *> &listLabelledPixels,
    cv::Point p);
  static QMap<int, std::vector<int> > getScanlines(const CC &cc,
                                                  int min_X,
                                                  int max_X,
                                                  int min_Y,
                                                  int max_Y);
  static QMap<int, std::vector<int> > getScanlinesB(const CC &cc);
  static void updatePixelLabel(std::vector<LabelledPixel *> listLabelledPixels,
                               cv::Point p,
                               int newLabel);

};

#endif // CONNECTEDCOMPONENTEXTRACTION_H
