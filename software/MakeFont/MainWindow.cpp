#include "MainWindow.hpp"

#include <iostream>
#include <QBoxLayout>
#include <QComboBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QFontComboBox>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QStatusBar>

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
  
  QLabel *outputFileLabel = new QLabel(tr("Output file: "), this);
  m_outputFileLE = new QLineEdit(this);
  m_outputFilePB = new QPushButton(tr("..."), this);
  m_processPB = new QPushButton(tr("run"), this);

  m_statusBar = new QStatusBar(this);
  
  QHBoxLayout *hlayout1 = new QHBoxLayout;
  hlayout1->addWidget(fontLabel);
  hlayout1->addWidget(m_fontCB);
  hlayout1->addWidget(sizeLabel);
  hlayout1->addWidget(m_sizeCB);
  hlayout1->addWidget(styleLabel);
  hlayout1->addWidget(m_styleCB);
  
  QHBoxLayout *hlayout2 = new QHBoxLayout;
  hlayout2->addWidget(outputFileLabel);
  hlayout2->addWidget(m_outputFileLE);
  hlayout2->addWidget(m_outputFilePB);
  QVBoxLayout *vlayout = new QVBoxLayout(this);
  vlayout->addLayout(hlayout1);
  vlayout->addLayout(hlayout2);
  vlayout->addWidget(m_processPB);
  vlayout->addWidget(m_statusBar);
  setLayout(vlayout);

  m_sizeCB->addItem(QVariant(DEFAULT_SIZE).toString()); //will be kept if present
  changeFont(m_fontCB->currentFont());

  connect(m_fontCB, SIGNAL(currentFontChanged(QFont)), this, SLOT(changeFont(QFont)));

  connect(m_sizeCB, SIGNAL(currentTextChanged(QString)), this, SLOT(updateOutputFilename()));
  connect(m_styleCB, SIGNAL(currentTextChanged(QString)), this, SLOT(updateOutputFilename()));
  
  connect(m_outputFileLE, SIGNAL(textChanged(QString)), this, SLOT(updateProcess()));
  connect(m_outputFilePB, SIGNAL(clicked()), this, SLOT(chooseOutputFilename()));
  connect(m_processPB, SIGNAL(clicked()), this, SLOT(produceOutput()));

  
  updateOutputFilename();
  updateProcess();

  m_statusBar->showMessage(tr("Ready"));
}


void
MainWindow::updateProcess()
{
  m_processPB->setEnabled(false);
  std::cerr<<"updateProcess text="<<m_outputFileLE->text().toStdString()<<"\n";
  if (! m_outputFileLE->text().isEmpty()) {
      m_processPB->setEnabled(true);
  }
}

void
MainWindow::chooseOutputFilename()
{
  
  QString filename = getOutputFilename(m_fontCB->currentFont());
  filename = QFileDialog::getSaveFileName(this, tr("Output OF filename"), filename, tr("OF files (*.of)"));
  if (! filename.isEmpty()) {
    m_outputFileLE->setText(filename);
  }
}


QString
MainWindow::getOutputFilename(const QFont &font)
{
  QString outputFilename = font.family() + "__" + m_styleCB->currentText() + "__" + m_sizeCB->currentText() + ".of";
  outputFilename.replace(' ', '_');
  return outputFilename;
}

void
MainWindow::changeFont(const QFont &font)
{
  findStyles(font);
  findSizes(font);
  
  updateOutputFilename(font);
      
}

void
MainWindow::updateOutputFilename(const QFont &font)
{
  QString outputFilename = getOutputFilename(font);
  QString currentOutputFilename = m_outputFileLE->text();
  std::cerr<<"outputFilename="<<outputFilename.toStdString()<<"\n";
  std::cerr<<"currentOutputFilename="<<currentOutputFilename.toStdString()<<"\n";
  if (! currentOutputFilename.isEmpty()) {
    outputFilename = QFileInfo(currentOutputFilename).dir().absoluteFilePath(outputFilename);
    std::cerr<<"=> outputFilename="<<outputFilename.toStdString()<<"\n";
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
    const QSignalBlocker blocker(m_sizeCB);
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
  }

    const int sizeIndex = m_sizeCB->findText(currentSize);
    if(sizeIndex == -1)
      m_sizeCB->setCurrentIndex(qMax(0, (m_sizeCB->count()*2) / 3));
    else
      m_sizeCB->setCurrentIndex(sizeIndex);
}
  
  
static void
saveFont(const QFont &font,
	 const QString &fontName,
	 const QString &outputFilename);

void
MainWindow::produceOutput()
{
  QString filename = m_outputFileLE->text();

  QFontDatabase fontDatabase;
  QFont font = fontDatabase.font(m_fontCB->currentFont().family(), m_styleCB->currentText(), m_sizeCB->currentText().toInt()); 
  //int size = m_sizeCB->currentText().toInt();
  //font.setPointSize(size);
  QString fontName = getOutputFilename(font);
  saveFont(font, fontName, filename);
  
  m_statusBar->showMessage(tr("saved %1").arg(filename), 2000);
}

#include <models/character.h>
#include <models/characterdata.h>
#include <models/font.h>
#include <iomanager/fontfilemanager.h>

void
saveFont(const QFont &font,
	 const QString &fontName,
	 const QString &outputFilename)
{
  QFontMetrics fontMetrics(font);
  
  Models::Font docFont(fontName);

  long sumWidths = 0;
  long sumHeights = 0;
  int numChars = 0;
  static QString charactersToSave = "!\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~¨éèçà";
  for (int i = 0; i < charactersToSave.size(); ++i) {
    const QChar c = charactersToSave.at(i);

    //std::cerr<<QString(c).toStdString()<<"\n";

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    int width = fontMetrics.horizontalAdvance(c);
#else
    int width = fontMetrics.width(c);
#endif
    int height = fontMetrics.height();
    //QRect boundingRect = fontMetrics.boundingRect(c);
    //std::cerr<<"height="<<height<<"\n";
    //std::cerr<<"width="<<width<<"\n";
    //std::cerr<<"boundingRect: x="<<boundingRect.x()<<" y="<<boundingRect.y()<<" w="<<boundingRect.width()<<" h="<<boundingRect.height()<<"\n";
    //std::cerr<<"fontMetrics.ascent()="<< fontMetrics.ascent()<<"\n";
    
    QImage img(width, height, QImage::Format_ARGB32);
    img.fill(qRgba(0,0,0,0));
    QPainter p(&img);
    p.setFont(font);
    p.setPen(QPen(Qt::black));
    p.drawText(0, fontMetrics.ascent(), QString(c));

    //QString outFilename = outputDir+"/"+c+".png";
    //img.save(outFilename);

    Models::CharacterData *chd = new Models::CharacterData(img, 0);
    qreal upLine = 0;
    qreal baseLine = 100; //TODO
    qreal leftLine = 0;
    qreal rightLine = 100; //TODO
    
    Models::Character *ch = new Models::Character(QString(c),
						  upLine, baseLine, leftLine, rightLine);
    ch->add(chd);
    docFont.addCharacter(ch);

    sumWidths += width;
    sumHeights += height;
    ++numChars;
  }
  //add space
  {
    const int space_width = sumWidths/numChars;
    const int space_height = sumHeights/numChars;
    QImage spaceImg(space_width, space_height, QImage::Format_ARGB32);
    spaceImg.fill(qRgba(0,0,0,0));
    Models::CharacterData *chd = new Models::CharacterData(spaceImg, 0);
    qreal upLine = 0;
    qreal baseLine = 100; //TODO
    qreal leftLine = 0;
    qreal rightLine = 100; //TODO
    
    Models::Character *ch = new Models::Character(QStringLiteral(" "),
						  upLine, baseLine, leftLine, rightLine);
    ch->add(chd);
    docFont.addCharacter(ch);
  }

  //TODO: add space ??? cf software/DocCreator/src/Document/ChooseLabelForComponentForm.cpp ???

  IOManager::FontFileManager::fontToXml(&docFont, outputFilename);

}
