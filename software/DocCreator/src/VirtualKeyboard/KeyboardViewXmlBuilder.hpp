#ifndef KEYBOARDVIEWXMLBUILDER_H
#define KEYBOARDVIEWXMLBUILDER_H

#include "KeyboardViewBuilder.hpp"

class CharacterButtonView;
class QXmlStreamReader;

class KeyboardViewXmlBuilder : public KeyboardViewBuilder
{
public:
  explicit KeyboardViewXmlBuilder(const QString &filepath);

  virtual void buildGeometry() override;
  virtual void buildCharButtons() override;
  virtual void buildControlButtons() override;

protected:
  CharacterButtonView *charButtonFromXml(QXmlStreamReader &reader);

private:
  QString _geometryXml;
  QString _charButtonsXml;
  QString _controlButtonsXml;
};

#endif // KEYBOARDVIEWXMLBUILDER_H
