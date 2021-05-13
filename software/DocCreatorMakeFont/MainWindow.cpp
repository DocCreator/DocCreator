#include "MainWindow.hpp"

#include <iostream>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QFontComboBox>
#include <QGuiApplication>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>

#include "CharacterWidget.hpp"


static
RangeVector
Latin_common()
{
  RangeVector v;
  /*
  static const QString charactersToSave = "!\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~¨éèçà";
  for (int i = 0; i < charactersToSave.size(); ++i) {
    const QChar c = charactersToSave.at(i);
    const int k = (int)c.unicode();
    v.push_back(Range(k, k));
  }
  */

  //basic letters (upper & lower case), numbers, punctuation and symbols
  //https://en.wikipedia.org/wiki/Basic_Latin_(Unicode_block)
  //https://fr.wikipedia.org/wiki/Table_des_caract%C3%A8res_Unicode/U0000
  v.push_back(Range("0020", "007E"));

  //Latin-1 Supplement: accented characters and symbols
  //https://en.wikipedia.org/wiki/Latin-1_Supplement_(Unicode_block)
  //https://fr.wikipedia.org/wiki/Table_des_caract%C3%A8res_Unicode/U0080
  v.push_back(Range("00A1", "00AC"));

  return v;
}

static
RangeVector
Latin_extended1()
{
  RangeVector v = Latin_common();

  //other accented letters
  //https://en.wikipedia.org/wiki/Latin_Extended-A
  //https://fr.wikipedia.org/wiki/Table_des_caract%C3%A8res_Unicode/U0100
  v.push_back(Range("0100", "017F"));

  return v;
}

static
RangeVector
Latin_extended2()
{
  RangeVector v = Latin_extended1();

  //other accented letters
  //https://fr.wikipedia.org/wiki/Table_des_caract%C3%A8res_Unicode/U0180
  v.push_back(Range("0180", "024F"));

  //other accented letters
  //https://fr.wikipedia.org/wiki/Table_des_caract%C3%A8res_Unicode/U1E00
  v.push_back(Range("1E00", "1EFF"));

  return v;
}

static
RangeVector
Latin_common_and_ligatures()
{
  RangeVector v = Latin_common();

  //https://en.wikipedia.org/wiki/Ligature_(writing)
  //https://en.wikipedia.org/wiki/Alphabetic_Presentation_Forms
  v.push_back(Range("FB00", "FB06"));  //ff, fi, fl, ffi, ffl, ft, st

  v.push_back(Range("0152")); //OE
  v.push_back(Range("0153")); //oe
  v.push_back(Range("00C6")); //AE
  v.push_back(Range("00E6")); //ae
  v.push_back(Range("1E9E")); //fs
  v.push_back(Range("00DF")); //fz

  v.push_back(Range("017F")); //old S

  return v;
}

static
RangeVector
Greek_and_Coptic_common()
{
  RangeVector v;

  //https://en.wikipedia.org/wiki/Greek_and_Coptic
  //https://fr.wikipedia.org/wiki/Table_des_caract%C3%A8res_Unicode/U0370
  v.push_back(Range("0370", "03FF"));

  return v;
}

static
RangeVector
Greek_and_Coptic_extended()
{
  RangeVector v = Greek_and_Coptic_common();

  //https://en.wikipedia.org/wiki/Greek_Extended
  //https://fr.wikipedia.org/wiki/Table_des_caract%C3%A8res_Unicode/U1F00
  v.push_back(Range("1F00", "1FFF"));

  return v;
}

static
RangeVector
Cyrillic_common()
{
  RangeVector v;

  //https://en.wikipedia.org/wiki/Cyrillic_script
  //https://fr.wikipedia.org/wiki/Alphabet_cyrillique
  v.push_back(Range("0400", "04FF"));

  return v;
}

static
RangeVector
Cyrillic_extended()
{
  RangeVector v = Cyrillic_common();

  //https://en.wikipedia.org/wiki/Cyrillic_script
  //https://fr.wikipedia.org/wiki/Alphabet_cyrillique
  v.push_back(Range("0500", "052F"));
  v.push_back(Range("02DE0", "02DFF")); //Cyrillic Extended-A
  v.push_back(Range("A640", "A69F"));  // //Cyrillic Extended-B

  return v;
}

static
RangeVector
Cyrillic_extended2()
{
  RangeVector v = Cyrillic_extended();

  //https://en.wikipedia.org/wiki/Cyrillic_script
  //https://fr.wikipedia.org/wiki/Alphabet_cyrillique
  v.push_back(Range("1C80", "1C8F")); //Cyrillic Extended-C
  v.push_back(Range("1D2B", "1D78")); //Phonetic Extensions
  v.push_back(Range("FE2E", "FE2F")); //Combining Half Marks

  return v;
}

static
RangeVector
CJKV_common()
{
  // Chinese, Japanese, Korean, Vietnamese
  // Chinese Hanzi, Japanese Kanji, Korean Hanja, and Vietnamese Chu Nom.
  // https://fr.wikipedia.org/wiki/Codage_des_caract%C3%A8res_chinois
  // https://en.wikipedia.org/wiki/CJK_Unified_Ideographs_(Unicode_block)

  RangeVector v;
  v.push_back(Range("302A", "302D")); //punctuation
  v.push_back(Range("4E00", "9FFF")); //CJK Unified Ideographs
  //=> 20994 characters
  return v;
}

static
RangeVector
CJKV_rare()
{
  // Chinese, Japanese, Korean, Vietnamese
  // Chinese Hanzi, Japanese Kanji, Korean Hanja, and Vietnamese Chu Nom.
  // https://fr.wikipedia.org/wiki/Codage_des_caract%C3%A8res_chinois
  // https://en.wikipedia.org/wiki/CJK_Unified_Ideographs_(Unicode_block)

  RangeVector v;
  v.push_back(Range("302A", "302D")); //punctuation
  v.push_back(Range("4E00", "9FFF")); //CJK Unified Ideographs
  v.push_back(Range("3400", "4DBF")); //extension A
  v.push_back(Range("20000", "2A6DF")); //extension B
  v.push_back(Range("2A700", "2B73F")); //extension C
  v.push_back(Range("2B740", "2B81F")); //extension D
  //=> 74686 characters
  return v;
}

static
RangeVector
CJKV_all()
{
  // Chinese, Japanese, Korean, Vietnamese
  // Chinese Hanzi, Japanese Kanji, Korean Hanja, and Vietnamese Chu Nom.
  // https://fr.wikipedia.org/wiki/Codage_des_caract%C3%A8res_chinois
  // https://en.wikipedia.org/wiki/CJK_Unified_Ideographs_(Unicode_block)

  RangeVector v;
  v.push_back(Range("302A", "302D")); //punctuation
  v.push_back(Range("4E00", "9FFF")); //CJK Unified Ideographs
  v.push_back(Range("3400", "4DBF")); //extension A
  v.push_back(Range("20000", "2A6DF")); //extension B
  v.push_back(Range("2A700", "2B73F")); //extension C
  v.push_back(Range("2B740", "2B81F")); //extension D
  v.push_back(Range("2B820", "2CEAF")); //ext E
  v.push_back(Range("2CEB0", "2EBEF")); //ext F
  v.push_back(Range("30000", "3134F")); //ext G
  v.push_back(Range("FA0E"));
  v.push_back(Range("FA0F"));
  v.push_back(Range("FA11"));
  v.push_back(Range("FA13"));
  v.push_back(Range("FA14"));
  v.push_back(Range("FA1F"));
  v.push_back(Range("FA21"));
  v.push_back(Range("FA23"));
  v.push_back(Range("FA24"));
  v.push_back(Range("FA27"));
  v.push_back(Range("FA28"));
  v.push_back(Range("FA29"));
  //=> 92912 characters
  return v;
}

static
RangeVector
hebrew_range()
{
  //https://en.wikipedia.org/wiki/Unicode_and_HTML_for_the_Hebrew_alphabet
  //https://en.wikipedia.org/wiki/Hebrew_(Unicode_block)
  RangeVector v;
  v.push_back(Range("0591", "05C7"));
  v.push_back(Range("05D0", "05EA"));
  v.push_back(Range("05EF", "05F4"));
  v.push_back(Range("FB1D", "FB36"));
  v.push_back(Range("FB38", "FB3C"));
  v.push_back(Range("FB3E"));
  v.push_back(Range("FB40", "FB41"));
  v.push_back(Range("FB43", "FB44"));
  v.push_back(Range("FB46", "FB4F"));
  return v;
}

static
RangeVector
arabic_range()
{
  //https://en.wikipedia.org/wiki/Arabic_script_in_Unicode
  RangeVector v;
  v.push_back(Range("0600", "06FF"));
  v.push_back(Range("0750", "077F"));
  v.push_back(Range("08A0", "08FF"));
  v.push_back(Range("FB50", "FDFF"));
  v.push_back(Range("FE70", "FEFF"));
  //Rumi Numeral Symbols (10E60–10E7F, 31 characters)
  //Indic Siyaq Numbers (1EC70–1ECBF, 68 characters)
  //Ottoman Siyaq Numbers (1ED00–1ED4F, 61 characters)
  //Arabic Mathematical Alphabetic Symbols (1EE00–1EEFF, 143 characters)
  return v;
}

static
RangeVector
whole_range()
{
  RangeVector v;
  v.push_back(Range("0000", "FFFF"));
  return v;
}


static int DEFAULT_SIZE = 32;

MainWindow::MainWindow()
{
  buildGUI();
}

void
MainWindow::buildGUI()
{
  QLabel *fontLabel = new QLabel(tr("Font:"), this);
  m_fontCB = new QFontComboBox(this);
  QLabel *sizeLabel = new QLabel(tr("Size:"), this);
  m_sizeCB = new QComboBox(this);
  QLabel *styleLabel = new QLabel(tr("Style:"), this);
  m_styleCB = new QComboBox(this);
  QLabel *fontMergingLabel = new QLabel(tr("Automatic Font Merging:"), this);
  fontMergingLabel->setToolTip(tr("If enabled, if the selected font does not contain a character, it is taken from a similar looking font that contains the character."));
  m_fontMergingCB = new QCheckBox(this);
  m_fontMergingCB->setChecked(true);

  QScrollArea *scrollArea = new QScrollArea(this);
  m_characterWidget = new CharacterWidget(this);
  scrollArea->setWidget(m_characterWidget);

  QLabel *outputFileLabel = new QLabel(tr("Output file: "), this);
  m_outputFileLE = new QLineEdit(this);
  m_outputFilePB = new QPushButton(tr("..."), this);
  m_choicesCB = new QComboBox(this);
  m_processPB = new QPushButton(tr("save"), this);

  m_statusBar = new QStatusBar(this);

  populateChoices();

  QHBoxLayout *hlayout1 = new QHBoxLayout;
  hlayout1->addWidget(fontLabel);
  hlayout1->addWidget(m_fontCB);
  hlayout1->addWidget(sizeLabel);
  hlayout1->addWidget(m_sizeCB);
  hlayout1->addWidget(styleLabel);
  hlayout1->addWidget(m_styleCB);
  hlayout1->addWidget(fontMergingLabel);
  hlayout1->addWidget(m_fontMergingCB);


  QHBoxLayout *hlayout2 = new QHBoxLayout;
  hlayout2->addWidget(outputFileLabel);
  hlayout2->addWidget(m_outputFileLE);
  hlayout2->addWidget(m_outputFilePB);

  QVBoxLayout *vlayout = new QVBoxLayout(this);
  vlayout->addLayout(hlayout1);
  vlayout->addWidget(m_choicesCB);
  vlayout->addWidget(scrollArea, 1); //1 is ths stretch factor
  vlayout->addLayout(hlayout2);
  vlayout->addWidget(m_processPB);
  vlayout->addWidget(m_statusBar);
  setLayout(vlayout);

  m_sizeCB->addItem(QVariant(DEFAULT_SIZE).toString()); //will be kept if present
  changeFont(m_fontCB->currentFont());

  connect(m_fontCB, SIGNAL(currentFontChanged(QFont)), this, SLOT(changeFont(QFont)));

  connect(m_sizeCB, SIGNAL(currentTextChanged(QString)), this, SLOT(updateOutputFilename()));
  connect(m_styleCB, SIGNAL(currentTextChanged(QString)), this, SLOT(updateOutputFilename()));
  
  connect(m_fontCB, &QFontComboBox::currentFontChanged,
	  m_characterWidget, &CharacterWidget::updateFont);
  connect(m_sizeCB, &QComboBox::currentTextChanged,
	  m_characterWidget, &CharacterWidget::updateSize);
  connect(m_styleCB, &QComboBox::currentTextChanged,
	  m_characterWidget, &CharacterWidget::updateStyle);
  connect(m_fontMergingCB, &QAbstractButton::toggled,
	  m_characterWidget, &CharacterWidget::updateFontMerging);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)) 
  connect(m_choicesCB, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updateChoice);
#else
  connect(m_choicesCB, SIGNAL(currentIndexChanged(int)), this, SLOT(updateChoice(int)));
#endif //QT_VERSION
  connect(m_choicesCB, SIGNAL(currentIndexChanged(int)), this, SLOT(updateOutputFilename()));

  connect(m_outputFileLE, SIGNAL(textChanged(QString)), this, SLOT(updateProcess()));
  connect(m_outputFilePB, SIGNAL(clicked()), this, SLOT(chooseOutputFilename()));
  connect(m_processPB, SIGNAL(clicked()), this, SLOT(saveCharacters()));


  updateOutputFilename();
  updateProcess();

  updateChoice(m_choicesCB->currentIndex());

  m_statusBar->showMessage(tr("Ready"));
}

void
MainWindow::populateChoices()
{
  m_choicesCB->addItem(tr("Latin [common characters and ligatures]"));
  m_choices.push_back(Latin_common_and_ligatures());
  m_choicesCB->addItem(tr("Latin [common characters]"));
  m_choices.push_back(Latin_common());
  m_choicesCB->addItem(tr("Latin [common characters + extension A]"));
  m_choices.push_back(Latin_extended1());
  m_choicesCB->addItem(tr("Latin [common characters + extensions A,B]"));
  m_choices.push_back(Latin_extended2());

  m_choicesCB->addItem(tr("Greek and Coptic [common characters]"));
  m_choices.push_back(Greek_and_Coptic_common());
  m_choicesCB->addItem(tr("Greek and Coptic [common characters + extension]"));
  m_choices.push_back(Greek_and_Coptic_extended());

  m_choicesCB->addItem(tr("Cyrillic [common characters]"));
  m_choices.push_back(Cyrillic_common());
  m_choicesCB->addItem(tr("Cyrillic [common characters + extensions A,B]"));
  m_choices.push_back(Cyrillic_extended());
  m_choicesCB->addItem(tr("Cyrillic [common characters + extensions A,B,C,...]"));
  m_choices.push_back(Cyrillic_extended2());

  m_choicesCB->addItem(tr("Chinese, Japanese, Korean, Vietnamese [common characters]"));
  m_choices.push_back(CJKV_common());
  m_choicesCB->addItem(tr("Chinese, Japanese, Korean, Vietnamese [common characters + extensions A,B,C,D]"));
  m_choices.push_back(CJKV_rare());
  m_choicesCB->addItem(tr("Chinese, Japanese, Korean, Vietnamese [common characters + extensions A,B,C,D,E,F,G]"));
  m_choices.push_back(CJKV_all());

  m_choicesCB->addItem(tr("Hebrew characters"));
  m_choices.push_back(hebrew_range());

  m_choicesCB->addItem(tr("Arabic characters"));
  m_choices.push_back(arabic_range());

  m_choicesCB->addItem(tr("All characters"));
  m_choices.push_back(whole_range());

  assert((size_t)m_choicesCB->count() == m_choices.size());
}

void
MainWindow::updateChoice(int index)
{
  assert((size_t)index < m_choices.size());
  m_characterWidget->setChoices(m_choices[index]);
}

void
MainWindow::updateProcess()
{
  m_processPB->setEnabled(false);
  if (! m_outputFileLE->text().isEmpty()) {
    m_processPB->setEnabled(true);
  }
}

void
MainWindow::chooseOutputFilename()
{
  QString filename = getOutputFilename(m_fontCB->currentFont());
  filename = QFileDialog::getSaveFileName(this, tr("Output OF filename"), filename, tr("binary OF files (*.bof);;OF files (*.of)"));
  if (! filename.isEmpty()) {
    m_outputFileLE->setText(filename);
  }
}


QString
MainWindow::getOutputFilename(const QFont &font)
{
  QString suffix;
  if (m_choicesCB->currentText() != tr("All characters")) {
    suffix = "_"+m_choicesCB->currentText().replace("Chinese, Japanese, Korean, Vietnamese", "CJKV").replace(" + ","_").replace(" ", "_").replace(",", "").replace("[", "").replace("]", "");
  }

  QString outputFilename = font.family() + "__" + m_styleCB->currentText() + "__" + m_sizeCB->currentText() + suffix+".bof";
  outputFilename.replace(' ', '_');
  return outputFilename;
}

void
MainWindow::changeFont(const QFont &font)
{
  findStyles(font);
  findSizes(font);

  m_characterWidget->updateSize(m_sizeCB->currentText());
  m_characterWidget->updateStyle(m_styleCB->currentText());
  
  updateOutputFilename(font);
}


void
MainWindow::updateOutputFilename(const QFont &font)
{
  QString outputFilename = getOutputFilename(font);
  QString currentOutputFilename = m_outputFileLE->text();
  if (! currentOutputFilename.isEmpty()) {
    outputFilename = QFileInfo(currentOutputFilename).dir().absoluteFilePath(outputFilename);
  }
  m_outputFileLE->setText(outputFilename);
}

void
MainWindow::updateOutputFilename()
{
  updateOutputFilename(m_fontCB->currentFont());
}
  
void
MainWindow::findStyles(const QFont &font)
{
  QFontDatabase fontDatabase;
  const QString currentItem = m_styleCB->currentText();
  m_styleCB->clear();

  const QStringList styles = fontDatabase.styles(font.family());
  for (const QString &style : styles)
    m_styleCB->addItem(style);

  const int styleIndex = m_styleCB->findText(currentItem);
  if (styleIndex == -1)
    m_styleCB->setCurrentIndex(0);
  else
    m_styleCB->setCurrentIndex(styleIndex);
}

void MainWindow::findSizes(const QFont &font)
{
  QFontDatabase fontDatabase;
  const QString currentSize = m_sizeCB->currentText();

  {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    const QSignalBlocker blocker(m_sizeCB);
#else
    const bool wasBlocked = m_sizeCB->blockSignals(true);
#endif //QT_VERSION
    // m_sizeCB signals are now blocked until end of scope
    m_sizeCB->clear();
    
    if (fontDatabase.isSmoothlyScalable(font.family(), fontDatabase.styleString(font))) {
      const QList<int> sizes = QFontDatabase::standardSizes();
      for (const int size : sizes) {
	m_sizeCB->addItem(QVariant(size).toString());
	m_sizeCB->setEditable(true);
      }
      
    }
    else {
      const QList<int> sizes = fontDatabase.smoothSizes(font.family(), fontDatabase.styleString(font));
      for (const int size : sizes ) {
	m_sizeCB->addItem(QVariant(size).toString());
	m_sizeCB->setEditable(false);
      }
    }

#if (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
    m_sizeCB->blockSignals(wasBlocked);
#endif //QT_VERSION
  }

  const int sizeIndex = m_sizeCB->findText(currentSize);
  if(sizeIndex == -1)
    m_sizeCB->setCurrentIndex(qMax(0, (m_sizeCB->count()*2) / 3));
  else
    m_sizeCB->setCurrentIndex(sizeIndex);
}


#include <models/character.h>
#include <models/characterdata.h>
#include <models/font.h>
#include <iomanager/fontfilemanager.h>

static
void
getLeftRight(const QImage &img,
	     qreal &leftLine, qreal &rightLine)
{
  const int w = img.width();
  const int h = img.height();

  assert(img.depth() == 32);

  int minX = w;
  int maxX = 0;

  const QRgb WHITE = qRgb(255, 255, 255);
  for (int i=0; i<h; ++i) {
    const QRgb *s = (const QRgb*)img.constScanLine(i);
    for (int j=0; j<minX; ++j) {
      if (s[j] != WHITE) {
	  minX = j;
      }
    }
    for (int j=maxX+1; j<w; ++j) {
      if (s[j] != WHITE) {
	  maxX = j;
      }
    }
  }

  assert(0 <= minX && minX < w);
  assert(0 <= maxX && maxX < w);
  leftLine = (minX * 100)/(qreal)w;
  rightLine = (maxX * 100)/(qreal)w;
}

static
int
saveCharactersFromFont(const RangeVector &rv,
		       const QFont &font,
		       const QString &fontName,
		       const QString &outputFilename,
		       bool updateLeftRight = false)
{
  QFontMetrics fontMetrics(font);

  Models::Font docFont(fontName);

  long sumWidths = 0;
  long sumHeights = 0;
  int numChars = 0;

  for (const Range &r : rv) {

    for (int key=r.start; key<=r.end; ++key) {

      const QChar c(key);

      if (fontMetrics.inFont(c)) {

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
	int width = fontMetrics.horizontalAdvance(c);
#else
	int width = fontMetrics.width(c);
#endif
	int height = fontMetrics.height();

	const QRect boundingRect = fontMetrics.boundingRect(c);
	if (width == 0)
	  width = boundingRect.width();
	if (height == 0)
	  height = boundingRect.height();

	if (width > 0 && height > 0) {

	  QImage img(width, height, QImage::Format_ARGB32);
	  img.fill(qRgba(255,255,255,0));
	  QPainter p(&img);
	  p.setFont(font);
	  p.setPen(QPen(Qt::black));
	  p.drawText(0, fontMetrics.ascent(), QString(c));

	  qreal upLine = 0;
	  qreal baseLine = 100;
	  qreal leftLine = 0;
	  qreal rightLine = 100;
	  if (updateLeftRight) {
	    getLeftRight(img, leftLine, rightLine);
	    //qDebug()<<"char="<<c<<" leftLine="<<leftLine<<" rightLine="<<rightLine<<"\n";
	  }
	  Models::CharacterData *chd = new Models::CharacterData(img, 0);

	  Models::Character *ch = new Models::Character(QString(c),
							upLine, baseLine, leftLine, rightLine);
	  ch->add(chd);
	  docFont.addCharacter(ch);

	  sumWidths += width;
	  sumHeights += height;
	  ++numChars;
	}
      }
    }

  }

  //Add space
  const int spaceUnicode = hex2int("0020");
  if (! hasValue(rv, spaceUnicode)) {
    bool spaceAdded = false;
    const QChar c(spaceUnicode);
    if (fontMetrics.inFont(c)) {
      //TODO: take into account fontstyleStrategy ?
      //if (fontstyleStrategy() == QFont::NoFontMerging) {

      #if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
	int width = fontMetrics.horizontalAdvance(c);
#else
	int width = fontMetrics.width(c);
#endif
	int height = fontMetrics.height();

	const QRect boundingRect = fontMetrics.boundingRect(c);
	if (width == 0)
	  width = boundingRect.width();
	if (height == 0)
	  height = boundingRect.height();

	if (width > 0 && height > 0) {

	  QImage img(width, height, QImage::Format_ARGB32);
	  img.fill(qRgba(255,255,255,0));
	  QPainter p(&img);
	  p.setFont(font);
	  p.setPen(QPen(Qt::black));
	  p.drawText(0, fontMetrics.ascent(), QString(c));

	  qreal upLine = 0;
	  qreal baseLine = 100;
	  qreal leftLine = 0;
	  qreal rightLine = 100;
	  if (updateLeftRight) {
	    getLeftRight(img, leftLine, rightLine);
	    //qDebug()<<"char="<<c<<" leftLine="<<leftLine<<" rightLine="<<rightLine<<"\n";
	  }
	  Models::CharacterData *chd = new Models::CharacterData(img, 0);

	  Models::Character *ch = new Models::Character(QString(c),
							upLine, baseLine, leftLine, rightLine);
	  ch->add(chd);
	  docFont.addCharacter(ch);

	  ++numChars;
	  spaceAdded = true;
	}
    }
    if (! spaceAdded) {

      const int space_width = sumWidths/numChars;
      const int space_height = sumHeights/numChars;
      QImage spaceImg(space_width, space_height, QImage::Format_ARGB32);
      spaceImg.fill(qRgba(0, 0, 0, 0));
      Models::CharacterData *chd = new Models::CharacterData(spaceImg, 0);
      qreal upLine = 0;
      qreal baseLine = 100; //TODO
      qreal leftLine = 0;
      qreal rightLine = 100; //TODO

      Models::Character *ch = new Models::Character(QStringLiteral(" "),
						  upLine, baseLine, leftLine, rightLine);
      ch->add(chd);
      docFont.addCharacter(ch);

      ++numChars;
    }

  }

  const bool writeOk = IOManager::FontFileManager::writeFont(&docFont, outputFilename);
  if (! writeOk) {
    qDebug()<<"ERROR: unable to write font: "<<outputFilename;
    return 0;
  }

  return numChars;
}

void
MainWindow::saveCharacters()
{
  QGuiApplication::setOverrideCursor(Qt::BusyCursor);

  const QString filename = m_outputFileLE->text();

  QFontDatabase fontDatabase;
  QFont font = fontDatabase.font(m_fontCB->currentFont().family(), m_styleCB->currentText(), m_sizeCB->currentText().toInt());

  if (m_fontMergingCB->isChecked())
    font.setStyleStrategy(QFont::PreferDefault);
  else
    font.setStyleStrategy(QFont::NoFontMerging);

  //const int size = m_sizeCB->currentText().toInt();
  //font.setPointSize(size);

  const QString fontName = QFileInfo(filename).completeBaseName();

  const int currentChoice = m_choicesCB->currentIndex();
  assert((size_t)currentChoice < m_choices.size());
  const RangeVector &rv = m_choices[currentChoice];

  const bool updateLeftRight = m_choicesCB->currentText().contains(tr("Arabic"));

  const int numSavedCharacters = saveCharactersFromFont(rv, font, fontName, filename, updateLeftRight);

  m_statusBar->showMessage(tr("saved %1 characters to %2").arg(QString::number(numSavedCharacters), QFileInfo(filename).fileName()), 2000);

  QGuiApplication::restoreOverrideCursor();
}
