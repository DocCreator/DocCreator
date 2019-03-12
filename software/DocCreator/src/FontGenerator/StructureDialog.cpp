#include "StructureDialog.hpp"
#include "ui_StructureDialog.h"

#include "Document/DocumentController.hpp"
#include "StructureDetection.hpp"
#include "Utils/FontUtils.hpp" //computeBestLineSpacing
#include "context/fontcontext.h"
#include "models/doc/docparagraph.h"
#include "models/doc/docstyle.h"
#include "models/doc/document.h"
#include "models/font.h"
#include <Lipsum4Qt.hpp>
#include <Utils/convertor.h>
#include <opencv2/imgproc/imgproc.hpp> //cvtColor

static const int IMG_WIDTH = 500;
static const int IMG_HEIGHT = 680;

StructureDialog::StructureDialog(DocumentController *ctrl, QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::StructureDialog)
  , _ctrl(ctrl)
{
  ui->setupUi(this);
}

StructureDialog::~StructureDialog()
{
  delete ui;
}

QImage
StructureDialog::getResultImage()
{
  return Convertor::getQImage(_background);
}

void
StructureDialog::setOriginalImage(const QImage &img)
{
  _originalImg = Convertor::getCvMat(img);

  ui->originalImg->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->originalImg->setText(tr("original image"));
  ui->originalImg->setPixmap(QPixmap::fromImage(img.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation)));
}

void
StructureDialog::setBinaryImage(const QImage &img)
{
  cv::cvtColor(Convertor::getCvMat(img), _binaryImg, cv::COLOR_BGR2GRAY);
}

void
StructureDialog::setBackground(const QImage &img)
{
  _background = Convertor::getCvMat(img);
}

void
StructureDialog::init(const QImage &ori, const QImage &bin, const QImage &bkg)
{
  setOriginalImage(ori);
  setBinaryImage(bin);
  setBackground(bkg);

  _characterHeight = StructureDetection::getCharacterHeight(_binaryImg);

  _distanceMap = StructureDetection::getDistanceMap(_binaryImg);

  process();
}

void
StructureDialog::process()
{
  _blocks = getBlocks();

  updateView();
}

void
StructureDialog::updateView()
{
  _background.copyTo(_structure);

  // Draw rectangles on the thumbnail
  const size_t sz = _blocks.size();
  for (size_t i = 0; i < sz; ++i) {
    cv::rectangle(_structure, _blocks.at(i), cv::Scalar(220, 150, 80), 4);
  }

  QImage structural = Convertor::getQImage(_structure);

  ui->structure->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->structure->setText(tr("original image"));
  ui->structure->setPixmap(QPixmap::fromImage(structural.scaled(
    IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation)));
}

void
StructureDialog::loremIpsum()
{
  // Generate lorem ipsum text inside the given textboxes
  std::vector<cv::Rect> blocks = getBlocks();

  Models::Font *font = Context::FontContext::instance()->getCurrentFont();
  assert(font);

  auto style = new Doc::DocStyle(font->getName(), font->getName());
  Doc::Document *doc = _ctrl->getDocument();
  assert(doc);
  doc->addStyle(style);
  //B: REM: it is to set the style before adding the paragraphs
  // Otherwise paragraphs do not have a style and it crashes when we try to
  // export to PNG.

  const int lineSpacing = computeBestLineSpacing(*font);

  // We generate a new lorem ipsum object for each text block
  for (const cv::Rect r : blocks) {

    _ctrl->addTextBlock(r.x, r.y, r.width, r.height);

    auto para = new Doc::DocParagraph(doc);
    doc->add(para);
    Doc::DocParagraph *currentParagraph =
      doc
        ->currentParagraph(); //B:TODO: why call currentParagraph() when we alreay have para ?
    currentParagraph->setLineSpacing(lineSpacing);

    //TODO:
    //In DocumentController::addCharacters(), the first line is placed at currentTextBlock->marginTop()+getParagraphLineSpacing()
    //However, it should be at  currentTextBlock->marginTop()+computeBestAboveBaselineHeight() !
    // Thus we have to add this bestAboveBaselineHeight to DocParagraph (or TextBlock ?) to be able to access it in DocumentController.

    _ctrl->resetCurrentTextBlockCursor();

    if (ui->lorem->isChecked()) {
      Lipsum4Qt lipsumGenerator(Lipsum4Qt::SYNC);
      QString text = lipsumGenerator.Lipsum;
      _ctrl->addString(text);
    } else {
      //B: what do we do if not checked ???
    }

    _ctrl->setModified();
    _ctrl->update();
  }
}

std::vector<cv::Rect>
StructureDialog::getBlocks()
{
  // Returns the extracted text blocks

  int width = _originalImg.rows;
  int height = _originalImg.cols;

  cv::Mat model = cv::Mat::ones(width, height, CV_8U);
  model *= 255;

  const int thickness =
#if CV_MAJOR_VERSION*100+CV_MINOR_VERSION*10+CV_SUBMINOR_VERSION <= 345			 
			 CV_FILLED
#else
			 cv::FILLED
#endif
    ;
  
  switch (_model) {
    case 0:
      _distanceMap.copyTo(model);
      break;
    case 1: // single txt block
      cv::rectangle(
        model,
        cv::Point((int)((float)height / 7), (int)((float)width / 7)),
        cv::Point((int)((float)height * 6 / 7), (int)((float)width * 6 / 7)),
        cv::Scalar(0),
        thickness);
      break;
    case 2: // Two columns
      cv::rectangle(
        model,
        cv::Point((int)((float)height / 7), (int)((float)width / 7)),
        cv::Point((int)((float)height * (3.3) / 7),
                  (int)((float)width * 6 / 7)),
        cv::Scalar(0),
        thickness);
      cv::rectangle(
        model,
        cv::Point((int)((float)height * (3.7) / 7), (int)((float)width / 7)),
        cv::Point((int)((float)height * 6 / 7), (int)((float)width * 6 / 7)),
        cv::Scalar(0),
        thickness);
      break;
    case 3: // Two rows
      cv::rectangle(
        model,
        cv::Point((int)((float)height / 7), (int)((float)width / 7)),
        cv::Point((int)((float)height * 6 / 7), (int)((float)width * 3.3 / 7)),
        cv::Scalar(0),
        thickness);
      cv::rectangle(
        model,
        cv::Point((int)((float)height / 7), (int)((float)width * 3.7 / 7)),
        cv::Point((int)((float)height * 6 / 7), (int)((float)width * 6 / 7)),
        cv::Scalar(0),
        thickness);
      break;

    default:
      _distanceMap.copyTo(model);
      break;
  }
  return StructureDetection::getBlocks(model,
                                       (int)_characterHeight * _dilation);
}

void
StructureDialog::on_horizontalSlider_valueChanged(int value)
{
  _dilation = (float)value / 10;
  process();
}

void
StructureDialog::on_comboBox_currentIndexChanged(int index)
{
  _model = index;
  process();
}
