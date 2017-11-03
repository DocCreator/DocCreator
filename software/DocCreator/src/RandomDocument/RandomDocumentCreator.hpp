#ifndef RANDOMDOCUMENTCREATOR_HPP
#define RANDOMDOCUMENTCREATOR_HPP

#include <QObject>

class DocumentController;

namespace Context {
class CurrentRandomDocumentContext;
}
class RandomDocumentParameters;

class RandomDocumentCreator : public QObject
{
  Q_OBJECT
public:
  explicit RandomDocumentCreator(DocumentController *ctrl,
                                 const RandomDocumentParameters &params,
                                 QObject *parent = 0);

signals:
  void imageReady();

public slots:

  //create document(s) with one random background/font/text among those in @a params.
  void create();

  //create all documents with one random font and one random background for all texts in @a params.
  void createAllTextsOneFontBackground();

  //create all documents with all combinations of background/font/text in @a params.
  void createAllTexts();

protected:
  void create_aux(Context::CurrentRandomDocumentContext *randDoc,
                  const QString &fontName,
                  int lineSpacing,
                  int textIndex,
                  bool useRandomTextFile);

protected:
  DocumentController *_ctrl;
  const RandomDocumentParameters &_params;
};

#endif // RANDOMDOCUMENTCREATOR_HPP
