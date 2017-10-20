#include "RandomDocumentParameters.hpp"

RandomDocumentParameters::RandomDocumentParameters(QObject *parent)
  : QObject(parent)
  , lineSpacingType(FontHeightAdaptedLineSpacing)
  , lineSpacingMin(30)
  , lineSpacingMax(50)
  , bottomMarginMin(50)
  , bottomMarginMax(50)
  , leftMarginMin(50)
  , leftMarginMax(50)
  , rightMarginMin(50)
  , rightMarginMax(50)
  , topMarginMin(50)
  , topMarginMax(50)
  , nbBlocksPerColMin(3)
  , nbBlocksPerColMax(3)
  , nbBlocksPerRowMin(3)
  , nbBlocksPerRowMax(3)
  ,
  //nbDocs(20),
  nbPages(1)
  , percentOfEmptyBlocks(5)
  , imageWidth(1006)
  , imageHeight(1554)
  , imageSizeUniform(true)
{}
