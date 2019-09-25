#define NOMINMAX //for Visual

#include "OCRDialog.hpp"
#include "ui_OCRDialog.h"

#include <algorithm> //min
#include <cstdlib>
#include <fstream>
#include <iostream>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMouseEvent>
#include <QTableView>
#include <QTextStream>

#include <Utils/convertor.h>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include "opencv2/imgproc/imgproc.hpp"

#include "Baseline.hpp"
#include "Binarization.hpp"

#include "core/configurationmanager.h"
#include "iomanager/fontfilemanager.h"
#include "models/character.h"
#include "models/characterdata.h"
#include "models/font.h"
#include "appconstants.h"

//#include "tesseract_config.h" //TESSERACT_TESSDATA_PARENT_DIR;

//#define WRITE_BASELINES true

#if WRITE_BASELINES
#include <opencv2/highgui/highgui.hpp>
#endif //WRITE_BASELINES

static const int IMG_WIDTH = 500;
static const int IMG_HEIGHT = 680;
static const cv::Scalar SELECTED_BLUE = cv::Scalar(255, 0, 0);

static const int NUM_COLUMNS = 3;

static const QString DEFAULT_FONT_NAME = QStringLiteral("ocrFont");

OCRDialog::OCRDialog(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::OCRDialog)
  , m_table(nullptr)
  , m_currentIndex(0)
{
  ui->setupUi(this);
  ui->originalLabel->installEventFilter(this);
}

OCRDialog::~OCRDialog()
{
  delete ui;
}

void
OCRDialog::setParameters(const QString &tessdataParentDir,
                         const QString &language)
{
  m_tessdataParentDir = tessdataParentDir;
  m_language = language;
}

bool
OCRDialog::eventFilter(QObject *watched, QEvent *event)
{
  // Add an event to detect the click on the image to update the preview area
  QLabel *label = qobject_cast<QLabel *>(watched);
  if (label != nullptr && event->type() == QEvent::MouseButtonPress) {
    const QMouseEvent *k = (QMouseEvent *)event;

    const double xmouse =
      static_cast<int>((static_cast<double>(k->pos().x()) / ui->originalLabel->pixmap()->width()) *
            m_originalImg.width());
    const double ymouse =
      static_cast<int>((static_cast<double>(k->pos().y()) / ui->originalLabel->pixmap()->height()) *
            m_originalImg.height());

    for (int i = 0; i < (int)m_font.size(); ++i) {
      const FontLetter &f = m_font[i];

      // We check if the click is inside a known bounding box
      if (f.rect.contains(cv::Point(xmouse, ymouse))) {
        m_currentIndex = i;
        m_currentLetter = f;
        updateView();
        return QObject::eventFilter(watched, event);
      }
    }
  }
  return QObject::eventFilter(watched, event);
}

void
OCRDialog::init(const QImage &ori, const QImage &bin)
{
  setOriginalImage(ori);
  setBinarizedImage(bin);
  process();
  updateAlphabet();
}

void
OCRDialog::setOriginalImage(const QImage &img)
{
  // Initializa the image and launch the process
  m_originalImg = img;
}

void
OCRDialog::setBinarizedImage(const QImage &img)
{
  m_binarizedImg = img;
}

static double MASK_THRESHOLD = 20;

cv::Mat
OCRDialog::getLetterViewFromMask(const cv::Mat &original,
                                 const cv::Mat &mask) const
{
  // We extract the image
  cv::Mat mask_tmp;
  cv::threshold(mask, mask_tmp, MASK_THRESHOLD, 1, cv::THRESH_BINARY_INV);

  cv::Mat letter;
  original.copyTo(letter);
  cv::add(letter, 100, letter);
  //cv::threshold(letter, letter, 237, 237, cv::THRESH_TRUNC);

  original.copyTo(letter, mask_tmp);
  return letter;
}

cv::Mat
OCRDialog::getImageFromMask(const cv::Mat &original,
                            const cv::Mat &mask,
                            int background_value) const
{
  // We extract the image
  cv::Mat mask_tmp;
  cv::threshold(mask, mask_tmp, MASK_THRESHOLD, 1, cv::THRESH_BINARY_INV);

  cv::Mat letter(mask_tmp.rows, mask_tmp.cols, original.type());
  letter.setTo(
    cv::Scalar(background_value, background_value, background_value));

  original.copyTo(letter, mask_tmp);

  return letter;
}

QImage
OCRDialog::getQImageFromMask(const cv::Mat &original,
                             const cv::Mat &mask,
                             QRgb backgroundColor) const
{
  QImage qimg(original.cols, original.rows, QImage::Format_ARGB32);

  int rows = original.rows;
  int cols = original.cols;
  if (original.isContinuous() && mask.isContinuous() &&
      static_cast<size_t>(qimg.bytesPerLine()) == sizeof(int) * cols) {
    cols *= rows;
    rows = 1;
  }

  assert(original.type() == CV_8UC3);
  assert(mask.type() == CV_8UC1);

  for (int i = 0; i < rows; ++i) {
    const uchar *m = mask.ptr<uchar>(i);
    const cv::Vec3b *o = original.ptr<cv::Vec3b>(i);
    QRgb *d = reinterpret_cast<QRgb *>(qimg.scanLine(i));

    for (int j = 0; j < cols; ++j) {
      if (m[j] <= MASK_THRESHOLD) {
        const cv::Vec3b &c = o[j];
        d[j] = qRgba(c[2], c[1], c[0], 255); //BGR
      } else {
        d[j] = backgroundColor;
      }
    }
  }

  return qimg;
}

#include <iostream> //DEBUG

#include <clocale>

void
OCRDialog::process()
{
  cv::Mat src(Convertor::getCvMat(m_binarizedImg)),
    original(Convertor::getCvMat(m_originalImg)), tmp;
  cvtColor(src, tmp, cv::COLOR_BGR2GRAY);

  std::vector<cv::Vec4i> baselines_tmp = m_baselines;
  Baseline::computeBaselines(tmp, baselines_tmp);

  /* Tesseract processing */

  tesseract::TessBaseAPI tess;
  tess.SetVariable("tessedit_char_whitelist",
                   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

  const QByteArray tessdataParentDirBA = m_tessdataParentDir.toLatin1();
  const char *tessdataParentDir = tessdataParentDirBA.constData();
  //B:Warning: tessdataParentDir must end with "/"
  const QByteArray languageBA = m_language.toLatin1();
  const char *language = languageBA.constData();

  std::cerr << "ocr tessdataParentDir=" << tessdataParentDir << "\n";
  std::cerr << "ocr language=" << language << "\n";

#ifndef TESSERACT_VERSION

  const std::string lc = setlocale(LC_NUMERIC, NULL);
  const std::string newLocale = "C";
  if (lc != newLocale) {
    setlocale(LC_NUMERIC, newLocale.c_str());
    const std::string newLocaleSet = setlocale(LC_NUMERIC, NULL);
    std::cerr << "Warning: changing LC_NUMERIC from " << lc << " to "
              << newLocaleSet << " for Tesseract\n";
    /*
      On Ubuntu 14.04.1, with tesseract 3.03.02-3,
      if we do not change the LC_NUMERIC to C and let it to fr_FR.UTF-8 for example,
      tess.Init() fails and prints the following error:

      Error: Illegal min or max specification!
      "Fatal error encountered!" == NULL:Error:Assert failed:in file globaloc.cpp, line 75

      On Ubuntu 16.04, tesseract 3.04, with same locale, there is no such error.

      It seems that tesseract 3.03.02-3 does not define TESSERACT_VERSION
    */
  }

#endif //TESSERACT_VERSION

  const int tessInitFailed =
    tess.Init(tessdataParentDir, language, tesseract::OEM_DEFAULT);

  if (tessInitFailed == 0) {

    tess.SetPageSegMode(tesseract::PSM_SPARSE_TEXT);

    tess.SetImage(static_cast<uchar *>(tmp.data), tmp.cols, tmp.rows, 1, tmp.cols);
    tess.GetUTF8Text();

    tesseract::ResultIterator *ri = tess.GetIterator();

    cv::Mat base, base2;
    original.copyTo(base);

#if WRITE_BASELINES
    {
      for (int i = 0; i < (int)baselines_tmp.size(); ++i) {
        cv::Vec4i l = baselines_tmp[i];
        double angle = atan2(l[1] - l[3], l[0] - l[2]);
        if (fabs(angle) > 3)
          cv::line(base,
                   cv::Point(l[0], l[1]),
                   cv::Point(l[2], l[3]),
                   cv::Scalar(50, 255, 50));
      }

      cv::imwrite("myBaselinesLol.png", base);
      base = cv::Mat::zeros(src.rows, src.cols, src.type());
      original.copyTo(base);
    }
#endif //WRITE_BASELINES

    // For each detected Symbol
    if (ri != nullptr) {

      do {

        // We get the corresponding guessed character
        const char *symbol = ri->GetUTF8Text(tesseract::RIL_SYMBOL);

        if (symbol != nullptr && strcmp("\"", symbol) != 0) {

#if WRITE_BASELINES
          if (ri->IsAtBeginningOf(tesseract::RIL_TEXTLINE)) {
            int x1, y1, x2, y2;
            ri->Baseline(tesseract::RIL_TEXTLINE, &x1, &y1, &x2, &y2);
            cv::line(base,
                     cv::Point(x1, y1),
                     cv::Point(x2, y2),
                     cv::Scalar(255, 0, 0));
          }
#endif //WRITE_BASELINES

          // Bounding box computation
          int x1, y1, x2, y2;
          ri->BoundingBox(tesseract::RIL_SYMBOL, &x1, &y1, &x2, &y2);
          cv::Rect r = cv::Rect(cv::Point(x1, y1), cv::Point(x2, y2));

          // Tesseract baseline extraction
          ri->Baseline(tesseract::RIL_SYMBOL, &x1, &y1, &x2, &y2);

          // Tesseract confidence determination
          float conf = ri->Confidence(tesseract::RIL_SYMBOL);

          // We extract the image
          //cv::Mat mask = src(r);
          cv::Mat mask;
          original(r).copyTo(mask);

          // Then the object is created
          FontLetter fl;
          fl.binarization_step = Binarization::binarize(mask, mask);
          fl.mask = mask;
          fl.label = symbol;

          const int b2 = Baseline::getBaseline(r, baselines_tmp);
          fl.baseline2 = b2 - r.y;
          fl.baseline = (int)((y1 + y2) / 2 - r.y);

          fl.GT_baseline = static_cast<int>((fl.baseline2 + fl.baseline) / 2);
          fl.confidence = conf;
          fl.rect = r;
          fl.checked = false;

          // And pushed to the list
          m_font.push_back(fl);
        }
        delete[] symbol;

      } while ((ri->Next(tesseract::RIL_SYMBOL)));

      if (!m_font.empty())
        m_currentLetter = m_font[0];
    }

#if WRITE_BASELINES
    cv::imwrite("tessBaselines.png", base);
#endif //WRITE_BASELINES

    m_currentIndex = 0;

    updateView();
  }
}

void
OCRDialog::updateTableLetters()
{

  ui->tableLetters->clear();
  ui->tableLetters->setRowCount(0);
  ui->tableLetters->setColumnCount(1);

  QStringList horzHeaders;
  horzHeaders << QStringLiteral("Similar symbols");
  ui->tableLetters->setHorizontalHeaderLabels(horzHeaders);

  // We take the list of letters with the same label (sorted by confidence)
  m_similarList = getSimilarLetters(m_currentLetter);

  // And display them in the table
  for (const int ind : m_similarList) {
    assert(ind >= 0 && ind < (int)m_font.size());
    const FontLetter &f = m_font[ind];
    QTableWidgetItem *thumb = new QTableWidgetItem();
    thumb->setData(
      Qt::DecorationRole,
      QPixmap::fromImage(Convertor::getQImage(f.mask).scaled(20, 20)));
    thumb->setTextAlignment(Qt::AlignCenter);

    ui->tableLetters->insertRow(ui->tableLetters->rowCount());
    ui->tableLetters->setItem(ui->tableLetters->rowCount() - 1, 0, thumb);
    ui->tableLetters->item(ui->tableLetters->rowCount() - 1, 0)
      ->setBackground(getConfidenceColor(f.confidence));
  }
}

/*
  Return index of fontLetter @a f in alphabet.
  If not found, m_alphabet.size() is returned.
 */
int
OCRDialog::indexOfFontLetterInAlphabet(const FontLetter &f)
{
  int index = m_alphabet.size();
  for (int j = 0; j < (int)m_alphabet.size(); ++j) {
    const int ind = m_alphabet[j].first;
    assert(ind >= 0 && ind < (int)m_font.size());
    const FontLetter &fl = m_font[ind];
    if (f.label == fl.label) {
      index = j;
      break;
    }
  }
  return index;
}

void
OCRDialog::updateAlphabet()
{
  ui->tableAlphabet->clear();
  ui->tableAlphabet->setRowCount(0);
  ui->tableAlphabet->setColumnCount(NUM_COLUMNS);
  ui->tableAlphabet->verticalHeader()->setVisible(false);
  m_alphabet.clear();

  // We fill the alphabet
  for (int i = 0; i < (int)m_font.size(); ++i) {
    FontLetter &f = m_font[i];

    const int indexA = indexOfFontLetterInAlphabet(f);
    if (indexA < (int)m_alphabet.size()) {
      //label already in alphabet, we just increase its frequency
      m_alphabet[indexA].second++;
    }
    else {
      //label is not in the alphabet, it is added with a frequency of one.
      m_alphabet.emplace_back(std::pair<int, int>(i, 1));
    }
  }

  // We sort the table according to their frequency in the text
  std::sort(m_alphabet.begin(),
	    m_alphabet.end(),
	    [](const std::pair<int, int> c1, const std::pair<int, int> c2) {
	      return c1.second > c2.second;
	    });

  // We fill the QTable
  for (int j = 0; j < (int)m_alphabet.size(); ++j) {
    QTableWidgetItem *thumb = new QTableWidgetItem();
    thumb->setText(
      QString::fromStdString(m_font[m_alphabet[j].first].label + " (" +
                             std::to_string(m_alphabet[j].second) + ")"));

    //TODO:OPTIM: set number of rows at once (setRowCount() ?)
    if (j % NUM_COLUMNS == 0)
      ui->tableAlphabet->insertRow(ui->tableAlphabet->rowCount());

    ui->tableAlphabet->setItem(ui->tableAlphabet->rowCount() - 1, j % NUM_COLUMNS, thumb);

    if (j % 2 == 0)
      ui->tableAlphabet->item(ui->tableAlphabet->rowCount() - 1, j % NUM_COLUMNS)
        ->setBackground(QColor(235, 235, 239));
  }

  ui->tableAlphabet->horizontalHeader()->adjustSize();
}

void
OCRDialog::updateView()
{
  cv::Mat thumb = Convertor::getCvMat(m_originalImg);

  // Draw the baseline on the thumbnail
  cv::Mat image =
    getImageFromMask(thumb(m_currentLetter.rect), m_currentLetter.mask);

  // (+ borders to display it clearer)
  cv::copyMakeBorder(
    image, image, 3, 3, 3, 3, cv::BORDER_CONSTANT, cv::Scalar(237, 237, 237));

  if (m_currentLetter.baseline > image.rows)
    cv::copyMakeBorder(image,
                       image,
                       0,
                       m_currentLetter.baseline - image.rows + 3,
                       0,
                       0,
                       cv::BORDER_CONSTANT,
                       cv::Scalar(237, 237, 237));

  cv::line(image,
           cv::Point(0, m_currentLetter.baseline + 3),
           cv::Point(image.cols - 1, m_currentLetter.baseline + 3),
           cv::Scalar(50, 220, 50),
           1);

  QImage img = Convertor::getQImage(image);

  // Set the images
  ui->thumbnailImage->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->thumbnailImage->setText(tr("thumbnail"));
  ui->thumbnailImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  ui->thumbnailImage->setPixmap(QPixmap::fromImage(
    img.scaled(200, 200, Qt::KeepAspectRatio, Qt::FastTransformation)));

  // Set the original image
  cv::Mat ori_img = thumb(m_currentLetter.rect);
  cv::copyMakeBorder(ori_img,
                     ori_img,
                     3,
                     3,
                     3,
                     3,
                     cv::BORDER_CONSTANT,
                     cv::Scalar(237, 237, 237));

  if (m_currentLetter.baseline > image.rows)
    cv::copyMakeBorder(ori_img,
                       ori_img,
                       0,
                       m_currentLetter.baseline - ori_img.rows + 3,
                       0,
                       0,
                       cv::BORDER_CONSTANT,
                       cv::Scalar(237, 237, 237));

  QImage ori_img2 = Convertor::getQImage(ori_img);
  ui->thumbnailOriginalImage->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->thumbnailOriginalImage->setText(tr("original thumbnail"));
  ui->thumbnailOriginalImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  ui->thumbnailOriginalImage->setPixmap(QPixmap::fromImage(
    ori_img2.scaled(200, 200, Qt::KeepAspectRatio, Qt::FastTransformation)));

  // Set the label
  ui->letterLabel->setText(QString::fromStdString(m_currentLetter.label));

  // Set the baseline value
  ui->baselineSpinBox->setValue(m_currentLetter.baseline);

  // Set the binarization threshold value
  ui->binarizationSpinBox->setValue(m_currentLetter.binarization_step);

  // Draw a rectangle on the image over the selected component
  cv::rectangle(thumb, m_currentLetter.rect, cv::Scalar(0, 0, 255), 3);

  ui->smoothed->setChecked(m_currentLetter.checked);

  updateTableLetters();


  //Set currentLetter as current cell in tableAlphabet
  if (! m_alphabet.empty()) {
    const int indexA = indexOfFontLetterInAlphabet(m_currentLetter);
    assert((size_t)indexA < m_alphabet.size());
    const int row = indexA/NUM_COLUMNS;
    const int col = indexA-row*NUM_COLUMNS;
    ui->tableAlphabet->setCurrentCell(row, col);
  }

  // Highlight the symbols in the image with the same label
  for (const int ind : m_similarList) {
    assert(ind >= 0 && ind < (int)m_font.size());
    const FontLetter &f = m_font[ind];
    const cv::Mat roi = thumb(f.rect);
    cv::Mat color(roi.size(), CV_8UC3, cv::Scalar(250, 150, 0));
    const double alpha = 0.2;

    cv::addWeighted(color, alpha, roi, 1 - alpha, 0, roi);
  }

  // Set the image
  ui->originalLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->originalLabel->setText(tr("Original Image"));
  ui->originalLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  ui->originalLabel->setPixmap(
    QPixmap::fromImage(Convertor::getQImage(thumb).scaled(
      IMG_WIDTH, IMG_HEIGHT, Qt::KeepAspectRatio, Qt::FastTransformation)));
}

QColor
OCRDialog::getConfidenceColor(float conf) const
{
  // Returns a color from a confidence value (between 0-100)
  QColor color(230, 230, 230);
  if (conf < 60)
    color = QColor(255, 215, 215);
  else if (conf < 80)
    color = QColor(250, 250, 250);
  else if (conf >= 85 && conf < 100)
    color = QColor(215, 255, 215);
  else if (conf >= 100)
    color = QColor(100, 255, 100);

  return color;
}

/*
   Returns a sorted vector of the symbols with the same label than fl
   Returns also the related index of each letter
   The letters are sorted by decreasing confidence of the guessed label
*/
std::vector<int>
OCRDialog::getSimilarLetters(const FontLetter &fl) const
{
  // Get symbols with same label
  std::vector<std::pair<int, int>> list;

  const int sz = m_font.size();
  for (int i = 0; i < sz; ++i) {
    const FontLetter f = m_font[i];
    if (f.label == fl.label) {
      list.emplace_back(std::pair<int, int>(i, f.confidence));
    }
  }

  // Sort them by decreasing confidence
  std::sort(list.begin(),
	    list.end(),
	    [](std::pair<int, int> c1, std::pair<int, int> c2) {
	      return c1.second > c2.second;
	    });

  // We select only the n-firsts (where n is given by the user)
  const size_t size = std::min((int)list.size(), m_maxNumberOfSymbols);
  std::vector<int> croplist(size);
  for (size_t i = 0; i < size; ++i)
    croplist[i] = list[i].first;
  return croplist;
}

/*
std::vector<OCRDialog::FontLetter> OCRDialog::getFinalFont() const
{
  int spacing_w =0, spacing_h = 0;
  int sum = 0;
  std::vector<FontLetter> finalFont;
  for (int i=0; i<(int)m_alphabet.size(); ++i) {

    const int ind = m_alphabet[i].first;
    const FontLetter &fl = m_font[ind];

    const std::vector<int> sim = getSimilarLetters(fl);
    for (int ind2 : sim) {

      const FontLetter &fls = m_font[ind2];
      finalFont.push_back(fls);
      spacing_w += fls.rect.width;
      spacing_h += fls.rect.height;
      ++sum;
    }
  }

  //Add space letter

  spacing_w = static_cast<int>(spacing_w / (float)sum + 0.5f);
  spacing_h = static_cast<int>(spacing_h / (float)sum + 0.5f);

  FontLetter space;
  space.binarization_step = 0;
  space.mask = cv::Mat::ones(spacing_h, spacing_w, CV_8U);
  space.label = " ";
  space.baseline2 = spacing_h;
  space.baseline = spacing_h;

  space.GT_baseline = spacing_h;
  space.confidence = 100;
  space.rect = cv::Rect(0, 0, spacing_w, spacing_h);
  space.checked = false;

  finalFont.insert(finalFont.begin(), space);

  return finalFont;
}
*/

void
OCRDialog::on_tableLetters_clicked(const QModelIndex &index)
{
  // Change the current letter
  if (index.isValid()) {
    m_currentIndex = m_similarList[index.row()];
    m_currentLetter = m_font[m_currentIndex];
  }

  updateView();
}

void
OCRDialog::on_baselineSpinBox_valueChanged(int arg1)
{
  // update baseline value
  m_currentLetter.baseline = arg1;
  updateView();
}

void
OCRDialog::on_apply_clicked()
{
  if (m_font.empty())
    return;

  assert(m_currentIndex >= 0 && m_currentIndex < (int)m_font.size());

  // Modify the label and set the confidence to 100%
  m_currentLetter.label = ui->letterLabel->text().toUtf8().constData();
  m_currentLetter.confidence = 100;
  m_font[m_currentIndex] = m_currentLetter;

  m_validatedFont.push_back(m_currentLetter);

  // Switch to the next symbol
  const int fontSize = m_font.size();
  if (++m_currentIndex >= fontSize)
    m_currentIndex = 0;
  if (fontSize == 0)
    return;

  m_currentLetter = m_font[m_currentIndex];

  updateView();
  updateAlphabet();
}

void
OCRDialog::on_letterLabel_textChanged()
{
  // Update the label
  m_currentLetter.label = ui->letterLabel->text().toUtf8().constData();
}

void
OCRDialog::on_deleteButton_clicked()
{
  if (m_font.empty())
    return;

  // Remove the symbol from the font
  assert(m_currentIndex >= 0 && m_currentIndex < (int)m_font.size());
  ui->tableLetters->removeRow(m_currentIndex);
  m_font.erase(m_font.begin() + m_currentIndex);

  const int fontSize = m_font.size();
  if (m_currentIndex >= fontSize)
    m_currentIndex = 0;
  if (fontSize == 0)
    return;

  m_currentLetter = m_font[m_currentIndex];

  updateView();
  updateAlphabet();
}

void
OCRDialog::on_tableAlphabet_cellClicked(int row, int column)
{
  // Get current label
  assert(row * NUM_COLUMNS + column < (int)m_alphabet.size());
  std::string label = m_font[m_alphabet[row * NUM_COLUMNS + column].first].label;

  int tmp_conf = -1;

  // Select the one with the highest confidence
  for (int i = 0; i < (int)m_font.size(); ++i) {
    const FontLetter &f = m_font[i];
    if (f.label == label && f.confidence > tmp_conf) {
      m_currentIndex = i;
      m_currentLetter = m_font[m_currentIndex];
      tmp_conf = f.confidence;
    }
  }
  updateView();
}

void
OCRDialog::on_maxSymbol_valueChanged(int arg1)
{
  m_maxNumberOfSymbols = arg1;
  updateView();
}

void
OCRDialog::rebinarizeCurrentLetter()
{
  // Re-binarize the image of the current symbol
  cv::Mat thumb = Convertor::getCvMat(m_originalImg);
  thumb = thumb(m_currentLetter.rect);
  cv::cvtColor(thumb, thumb, cv::COLOR_BGR2GRAY);

  cv::Mat img_bin(thumb.rows, thumb.cols, CV_8U);

  cv::threshold(
		thumb, img_bin, m_currentLetter.binarization_step, 255, cv::THRESH_BINARY);

  // Holes filling + noise reduction
  if (m_currentLetter.checked)
    cv::medianBlur(img_bin, img_bin, 5);
  m_currentLetter.mask = img_bin;
  updateView();
}

void
OCRDialog::on_smoothed_toggled(bool checked)
{
  m_currentLetter.checked = checked;
  rebinarizeCurrentLetter();
}

/*
void OCRDialog::on_pushButton_3_clicked()
{
  float error1 =0;
  float error2 =0;
  for (const FontLetter &f : m_validatedFont) {
    error1 += std::abs(f.baseline - f.GT_baseline);
    error2 += std::abs(f.baseline2 - f.GT_baseline);
  }

  error1 /= m_validatedFont.size();
  error2 /= m_validatedFont.size();

  //B:useless ???
}
*/

void
OCRDialog::on_binarizationSpinBox_valueChanged(int arg1)
{
  m_currentLetter.binarization_step = arg1;
  rebinarizeCurrentLetter();
}

void
OCRDialog::on_saveFont_clicked()
{
  saveFont();
}

QString
OCRDialog::saveFont()
{
  const QString fontPath = QDir(Core::ConfigurationManager::get(
                                  AppConfigMainGroup, AppConfigFontFolderKey)
                                  .toString())
                             .absolutePath();
  //REM: we open the dialog in the application Font Directory
  // to be able to save the created font with other application fonts
  // and thus be able to use it when batch-generating.

  const QString filters(QStringLiteral("font files (*.of)"));
  const QString filename = QFileDialog::getSaveFileName(
    nullptr, QStringLiteral("Save Font"), fontPath, filters);
  if (! filename.isEmpty())
    writeFont(filename);

  return filename;
}

Models::Font *
OCRDialog::getFont() const
{
  auto font = new Models::Font(DEFAULT_FONT_NAME);

  const cv::Mat thumb = Convertor::getCvMat(m_originalImg);

  int sum_spacing_w = 0;
  int sum_spacing_h = 0;
  int nb = 0;

  const int nbLetters = m_alphabet.size();
  for (int i = 0; i < nbLetters; ++i) {
    const int ind = m_alphabet[i].first;
    const FontLetter &fl = m_font[ind];

    const qreal upLine = 0;
    const qreal baseLine = (100.f * fl.baseline / (float)fl.rect.height);
    const qreal leftLine = 0;
    const qreal rightLine = 100;

    auto c = new Models::Character(
      QString::fromStdString(fl.label), upLine, baseLine, leftLine, rightLine);
    int picture_id = 0;
    const std::vector<int> sim = getSimilarLetters(fl);
    for (int ind2 : sim) {
      const FontLetter &fls = m_font[ind2];

      QImage image = getQImageFromMask(thumb(fls.rect), fls.mask);

      auto *cd = new Models::CharacterData(image, picture_id);
      c->add(cd);

      ++picture_id;

      sum_spacing_w += fls.rect.width;
      sum_spacing_h += fls.rect.height;
      ++nb;
    }

    const bool addOk = font->addCharacter(c);
    if (!addOk) {
      std::cerr << "Warning: character for letter " << fl.label
                << " was not added to font. Already present ?\n";
    }
  }

  if (nb > 0) {
    //add space character to font

    const float mean_spacing_w = static_cast<int>(sum_spacing_w / nb + 0.5f);
    const float mean_spacing_h = static_cast<int>(sum_spacing_h / nb + 0.5f);

    const qreal upLine = 0;
    const qreal baseLine = 100;
    const qreal leftLine = 0;
    const qreal rightLine = 100;
    auto c = new Models::Character(
      QStringLiteral(" "), upLine, baseLine, leftLine, rightLine);

    QImage image(mean_spacing_w, mean_spacing_h, QImage::Format_ARGB32);
    image.fill(qRgba(255, 255, 255, 0)); //all transparent
    const int picture_id = 0;
    auto *cd = new Models::CharacterData(image, picture_id);
    c->add(cd);
    const bool addOk = font->addCharacter(c);
    if (!addOk) {
      std::cerr << "Warning: character for space letter was not added to font. "
                   "Already present ?\n";
    }
  }

  return font;
}

void
OCRDialog::writeFont(const QString &filename) const
{
  const QString fontName = QFileInfo(filename).completeBaseName();

  Models::Font *font = getFont();
  font->setName(fontName);

  IOManager::FontFileManager::fontToXml(font, filename);

  delete font;
}
