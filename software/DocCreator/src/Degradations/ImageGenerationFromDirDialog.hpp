#ifndef IMAGEGENERATIONFROMDIRDIALOG_HPP
#define IMAGEGENERATIONFROMDIRDIALOG_HPP

#include <QDialog>
#include <QDir>

struct LineText
{
  QList<QString> charList;
  int Long;
  int Height;
  float meanCharHeight;
};

namespace Doc {
class DocTextBlock;
class Document;
class Page;
}
namespace Models {
class Character;
class Font;
}
namespace Ui {
class ImageGenerationFromDirDialog;
}
class DocumentController;

class ImageGenerationFromDirDialog : public QDialog
{
  Q_OBJECT
public:
  ImageGenerationFromDirDialog(QWidget *parent,
                               DocumentController *docController);
  ~ImageGenerationFromDirDialog();

public slots:
  void changeFont(const QString &fontName);
  void changeBackground(const QString &background);
  void chooseTextDirectory();
  void generate();
  void accepted();

protected:
  void createNewDocument(const QString &filePath);
  QStringList createNewBigDocument(const QString &filePath);
  QStringList createDocument(const QString &filePath);
  void createFontSelector();
  void createBackgroundSelector();

private:
  QStringList fileSplit(const QString &filePath);

  Doc::Document *createADocument();
  Doc::Page *createAPage(Doc::Document *document);
  Doc::DocTextBlock *createATextBox(Doc::Document *document, int x, int y);
  QImage buildImage(const QString &file);
  QImage getCharacterImage(Models::Character *ch);
  void bindTextForATextBox(QStringList &textLines, Doc::DocTextBlock *tb);
  Doc::Page *bindTextForAPage(Doc::Document *document, QStringList &TEXT_lines);
  QStringList readTextFile(const QString &filePath);

  void degrade(QDir folder);
  void loadBackground();
  void loadFont();
  void setupPageParameters();
  void buildImagesFromXML(const QStringList &xmls);

private:
  Ui::ImageGenerationFromDirDialog *ui;
  DocumentController *_docController;
  Models::Font *_font;
  QImage _bg;
  QString _bgName;
  QString _fontName;
  int _lineSpacing;
  float _word_height;
  //page layout
  int _pageMarginTop;
  int _pageMarginLeft;
  int _pageMarginRight;
  int _pageMarginBottom;
  int _columnSpacing;
  int _width;
  int _height;
  QImage::Format _Char_Format;
  // text block layout
  int _nbColumns;
  int _columnsWidth;
  int _columnsHeight;

  //cv::Mat _bind_text_checker;
};

#endif // IMAGEGENERATIONFROMDIRDIALOG_HPP
