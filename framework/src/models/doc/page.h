#ifndef PAGE_H
#define PAGE_H

#include <QList>
#include <QMap>
#include <QString>
#include <framework_global.h>

namespace Doc
{
  class Block;
  class DocCharacter;
  class DocComponent;
  class DocComponentBlock;
  class DocImageBlock;
  class DocParagraph;
  class DocString;
  class DocStyle;
  class DocTestBlock;
  class DocTextBlock;
  class DocZone;
  class Document;
  
  using MapDegradationModelParameters = QMap<QString, QString>;
  using MapDegradationModels = QMap<QString, MapDegradationModelParameters>;

  class FRAMEWORK_EXPORT Page
  {
  public:
    //B:TODO:API: not possible to access document from Page !?!

    explicit Page(Document* document);

    ~Page();

    Page* clone();

    Page* getSelection();

    void changeStyle(DocStyle* style);
    DocStyle* getStyle();

    void add(Block* e);
    void add(DocTextBlock* e);
    void add(DocImageBlock* e);
    void add(DocTestBlock* e);
    // added by kvcuong 09/05/2012
    void add(DocComponentBlock* e);


    void remove(Block* e);
    QList<DocTextBlock*> getTextBlocks() { return _textBlocks; }
    QList<DocImageBlock*> getImageBlocks() { return _imageBlocks; }
    QList<DocTestBlock*> getTestBlocks() { return _testBlocks; }
    // added by kvcuong 09/05/2012
    QList<DocComponentBlock*> getComponentBlocks(){return _componentBlocks;}

    /**
     * Remove all text blocks from page.
     *
     * It is client responsibility to delete the text blocks.
     */
    void removeTextBlocks();

    QList<Block*> getOtherBlocks() { return _otherBlocks; }

    void setCurrentBlock(Block* e);
    Block* currentBlock();

    void add(DocParagraph* e);
    void add(const QList<DocParagraph*> &e);
    void add(DocString* e);
    void add(const QList<DocString*> &e);
    void add(DocCharacter* e);
    void add(const QList<DocCharacter*> &e);
    // added by kvcuong 09/05/2012
    void add(DocComponent* e);
    void add(const QList<DocComponent*> &e);
    void add(DocZone* e);
    void add(const QList<DocZone*> &e);


    QString getBackgroundFileName() { return _backgroundFileName; }
    MapDegradationModels getListDegradationsModels() {return _degradationModels;}

    void setBackgroundFileName(const QString &fileName) { _backgroundFileName = fileName; }
    void setAppliedDegradationModel(const QString &modelName, const MapDegradationModelParameters &paras){_degradationModels.insert(modelName, paras);}

  private:
    Page(const Page &);
    Page &operator=(const Page &);

  private:
    Document* _document;
    QList<DocTextBlock*> _textBlocks;
    QList<DocImageBlock*> _imageBlocks;
    QList<DocTestBlock*> _testBlocks;
    QList<DocComponentBlock*> _componentBlocks;

    QList<Block*> _otherBlocks;
    Block* _currentElement;
    QString _backgroundFileName;

    MapDegradationModels _degradationModels;
  };
}

#endif // PAGE_H
