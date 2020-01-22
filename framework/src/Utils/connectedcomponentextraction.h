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
  int label;
};

class FRAMEWORK_EXPORT ConnectedComponentExtraction
{
public:
  static QList<Doc::DocComponent *> getListComponents(const QImage &src,
                                                      Doc::Document *document);

  /**
       @brief extract all connected componenents from image @a input and fill @a ccs.

       @a connectivity must be 4 or 8.
     */

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
