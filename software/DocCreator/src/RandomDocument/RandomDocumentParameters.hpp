#ifndef RANDOMDOCUMENTPARAMETERS_HPP
#define RANDOMDOCUMENTPARAMETERS_HPP

#define NOMINMAX //for Visual
#include <QObject>
#include <QStringList>

class RandomDocumentParameters : public QObject
{
  Q_OBJECT
public:
  Q_PROPERTY(QString Name READ getName WRITE setName);

  explicit RandomDocumentParameters(QObject *parent = 0);

  QString getName() const { return _name; }

  void setName(const QString &name) { _name = name; }

  typedef enum {
    RandomLineSpacing,
    FontHeightAdaptedLineSpacing
  } LineSpacingType;

  void setLineSpacingType(LineSpacingType t) { lineSpacingType = t; }

public slots:

  void setLineSpacing(int pLS)
  {
    lineSpacingMin = pLS;
    lineSpacingMax = pLS;
  }
  void setLineSpacingMinMax(int pMin, int pMax)
  {
    lineSpacingMin = std::min(pMin, pMax);
    lineSpacingMax = pMax;
  }

  void setBottomMargin(int pBottomMargin)
  {
    bottomMarginMin = pBottomMargin;
    bottomMarginMax = pBottomMargin;
  }
  void setBottomMarginMinMax(int pMin, int pMax)
  {
    bottomMarginMin = std::min(pMin, pMax);
    bottomMarginMax = pMax;
  }

  void setLeftMargin(int pLeftMargin)
  {
    leftMarginMin = pLeftMargin;
    leftMarginMax = pLeftMargin;
  }
  void setLeftMarginMinMax(int pMin, int pMax)
  {
    leftMarginMin = std::min(pMin, pMax);
    leftMarginMax = pMax;
  }

  void setRightMargin(int pRightMargin)
  {
    rightMarginMin = pRightMargin;
    rightMarginMax = pRightMargin;
  }
  void setRightMarginMinMax(int pMin, int pMax)
  {
    rightMarginMin = std::min(pMin, pMax);
    rightMarginMax = pMax;
  }

  void setTopMargin(int pTopMargin)
  {
    topMarginMin = pTopMargin;
    topMarginMax = pTopMargin;
  }
  void setTopMarginMinMax(int pMin, int pMax)
  {
    topMarginMin = std::min(pMin, pMax);
    topMarginMax = pMax;
  }

  void setNbBlocksPerCol(int pNbBlocksPerCol)
  {
    nbBlocksPerColMin = pNbBlocksPerCol;
    nbBlocksPerColMax = pNbBlocksPerCol;
  }
  void setNbBlocksPerColMinMax(int pMin, int pMax)
  {
    nbBlocksPerColMin = std::min(pMin, pMax);
    nbBlocksPerColMax = pMax;
  }

  void setNbBlocksPerRow(int pNbBlocksPerRow)
  {
    nbBlocksPerRowMin = pNbBlocksPerRow;
    nbBlocksPerRowMax = pNbBlocksPerRow;
  }
  void setNbBlocksPerRowMinMax(int pMin, int pMax)
  {
    nbBlocksPerRowMin = std::min(pMin, pMax);
    nbBlocksPerRowMax = pMax;
  }

  void setNbPages(int pNbPages) { nbPages = pNbPages; }

  void setPercentOfEmptyBlocks(int pPercent)
  {
    percentOfEmptyBlocks = pPercent;
  }

  //If set to true, all generated images are scaled to specified imageWidth x imageHeight
  //Otherwise, generated images will have the same size as the background image (and thus imageWidth and imageHeight are not used).
  void setImageSizeUniform(bool uniform) { imageSizeUniform = uniform; }

  void setImageWidth(int width) { imageWidth = width; }

  void setImageHeight(int height) { imageHeight = height; }

  void copyTo(RandomDocumentParameters &out)
  {
    out.outputFolderPath = outputFolderPath;

    out.backgroundList = backgroundList;
    out.fontList = fontList;
    out.textList = textList;

    out.lineSpacingType = lineSpacingType;
    out.lineSpacingMin = lineSpacingMin;
    out.lineSpacingMax = lineSpacingMax;

    out.bottomMarginMin = bottomMarginMin;
    out.bottomMarginMax = bottomMarginMax;
    out.leftMarginMin = leftMarginMin;
    out.leftMarginMax = leftMarginMax;
    out.rightMarginMin = rightMarginMin;
    out.rightMarginMax = rightMarginMax;
    out.topMarginMin = topMarginMin;
    out.topMarginMax = topMarginMax;

    out.nbBlocksPerColMin = nbBlocksPerColMin;
    out.nbBlocksPerColMax = nbBlocksPerColMax;
    out.nbBlocksPerRowMin = nbBlocksPerRowMin;
    out.nbBlocksPerRowMax = nbBlocksPerRowMax;

    out.nbPages = nbPages;
    out.percentOfEmptyBlocks = percentOfEmptyBlocks;

    out.imageWidth = imageWidth;
    out.imageHeight = imageHeight;
    out.imageSizeUniform = imageSizeUniform;
  }

public:
  QString outputFolderPath;

  QStringList backgroundList;
  QStringList fontList;
  QStringList textList;

  LineSpacingType lineSpacingType;
  int lineSpacingMin;
  int lineSpacingMax;

  int bottomMarginMin;
  int bottomMarginMax;
  int leftMarginMin;
  int leftMarginMax;
  int rightMarginMin;
  int rightMarginMax;
  int topMarginMin;
  int topMarginMax;

  int nbBlocksPerColMin;
  int nbBlocksPerColMax;
  int nbBlocksPerRowMin;
  int nbBlocksPerRowMax;

  //int nbDocs;
  int nbPages;
  int percentOfEmptyBlocks;

  int imageWidth;
  int imageHeight;
  bool
    imageSizeUniform; //tells if generated images must all be scaled to the same size

private:
  QString _name;
  //B: why this field is private and not the others ???
};

#endif // RANDOMDOCUMENTPARAMETERS_HPP
