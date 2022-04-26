#include "KeyboardViewXmlBuilder.hpp"

#include "CharacterButtonView.hpp"
#include "ControlButtonView.hpp"
#include "ModeTypeEnum.hpp"
#include <QFile>
#include <QLatin1String>
#include <QTextStream>
#include <QXmlStreamReader>

#include <iostream>

KeyboardViewXmlBuilder::KeyboardViewXmlBuilder(const QString &filepath)
{
  QFile file(filepath);
  const bool ok = file.open(QFile::ReadOnly);
  if (!ok) { //B:TODO: better check for errors !!!
    std::cerr << "Warning: unable to read keyboard file: "
              << filepath.toStdString() << "\n";
    return;
  }

  QTextStream ts(&file);
  QString xmlString = ts.readAll();
  file.close();

  int begin = xmlString.indexOf(QStringLiteral("<geometry"));
  int end = xmlString.indexOf(QStringLiteral("</geometry>")) +
            QStringLiteral("</geometry>").length();
  _geometryXml = xmlString.mid(begin, end - begin);

  begin = xmlString.indexOf(QStringLiteral("<charbuttons>"));
  end = xmlString.indexOf(QStringLiteral("</charbuttons>")) +
        QStringLiteral("</charbuttons>").length();
  _charButtonsXml = xmlString.mid(begin, end - begin);

  begin = xmlString.indexOf(QStringLiteral("<controlbuttons>"));
  end = xmlString.indexOf("</controlbuttons>") +
        QStringLiteral("</controlbuttons>").length();
  _controlButtonsXml = xmlString.mid(begin, end - begin);
}

//Build methods

void
KeyboardViewXmlBuilder::buildGeometry()
{
  QXmlStreamReader reader(_geometryXml);

  while (!(reader.tokenType() == QXmlStreamReader::EndElement &&
           reader.name() == QLatin1String("geometry")) &&
         !reader.atEnd()) {
    if (reader.name() == QLatin1String("geometry") &&
        reader.tokenType() == QXmlStreamReader::StartElement) {
      _keyboardView->setFixedWidth(
        reader.attributes().value(QStringLiteral("width")).toString().toInt());
      _keyboardView->setFixedHeight(
        reader.attributes().value(QStringLiteral("height")).toString().toInt());
    }

    reader.readNext();
  }
}

void
KeyboardViewXmlBuilder::buildCharButtons()
{
  QXmlStreamReader reader(_charButtonsXml);

  while (!(reader.tokenType() == QXmlStreamReader::EndElement &&
           reader.name() == QLatin1String("charbuttons")) &&
         !reader.atEnd()) {
    QXmlStreamReader::TokenType tokenMain = reader.readNext();

    if (reader.name() == QLatin1String("charbutton") &&
        tokenMain == QXmlStreamReader::StartElement) {
      const int width = (reader.attributes()
                           .value(QStringLiteral("width"))
                           .toString()
                           .toDouble() /
                         100) *
                        _keyboardView->width();
      const int height = (reader.attributes()
                            .value(QStringLiteral("height"))
                            .toString()
                            .toDouble() /
                          100) *
                         _keyboardView->height();
      const int x = (reader.attributes()
                       .value(QStringLiteral("positionX"))
                       .toString()
                       .toDouble() /
                     100) *
                    _keyboardView->width();
      const int y = (reader.attributes()
                       .value(QStringLiteral("positionY"))
                       .toString()
                       .toDouble() /
                     100) *
                    _keyboardView->height();

      CharacterButtonView *charButton = charButtonFromXml(reader);
      charButton->setGeometry(x, y, width, height);

      _keyboardView->addCharButtonView(charButton);
    }
  }
}

void
KeyboardViewXmlBuilder::buildControlButtons()
{
  QXmlStreamReader reader(_controlButtonsXml);

  while (!(reader.tokenType() == QXmlStreamReader::EndElement &&
           reader.name() == QLatin1String("controlbuttons")) &&
         !reader.atEnd()) {
    QXmlStreamReader::TokenType tokenMain = reader.readNext();

    if (reader.name() == QLatin1String("button") &&
        tokenMain == QXmlStreamReader::StartElement) {
      const int width = (reader.attributes()
                           .value(QStringLiteral("width"))
                           .toString()
                           .toDouble() /
                         100) *
                        _keyboardView->width();
      const int height = (reader.attributes()
                            .value(QStringLiteral("height"))
                            .toString()
                            .toDouble() /
                          100) *
                         _keyboardView->height();
      const int x = (reader.attributes()
                       .value(QStringLiteral("positionX"))
                       .toString()
                       .toDouble() /
                     100) *
                    _keyboardView->width();
      const int y = (reader.attributes()
                       .value(QStringLiteral("positionY"))
                       .toString()
                       .toDouble() /
                     100) *
                    _keyboardView->height();
      const int id =
        reader.attributes().value(QStringLiteral("id")).toString().toInt();
      const QString name =
        reader.attributes().value(QStringLiteral("name")).toString();

      auto controlButton = new ControlButtonView(_keyboardView, id);
      controlButton->setGeometry(x, y, width, height);
      controlButton->setText(name);

      _keyboardView->addControlButton(id, controlButton);
    }
  }
}

CharacterButtonView *
KeyboardViewXmlBuilder::charButtonFromXml(QXmlStreamReader &reader)
{
  auto charButton = new CharacterButtonView(_keyboardView);

  while (!(reader.tokenType() == QXmlStreamReader::EndElement &&
           reader.name() == QLatin1String("charbutton")) &&
         !reader.atEnd()) {
    QXmlStreamReader::TokenType tokenMain = reader.readNext();

    if (reader.name() == QLatin1String("uppercase") &&
        tokenMain == QXmlStreamReader::StartElement) {
      //const QString s = QString((QChar)reader.attributes().value("value").toString().toInt());
      const int key =
        reader.attributes().value(QStringLiteral("value")).toString().toInt();
      charButton->addModeCharacter(UpperCase, key);
    }

    if (reader.name() == QLatin1String("lowercase") &&
        tokenMain == QXmlStreamReader::StartElement) {
      //const QString s = QString((QChar)reader.attributes().value("value").toString().toInt());
      const int key =
        reader.attributes().value(QStringLiteral("value")).toString().toInt();
      charButton->addModeCharacter(LowerCase, key);
    }

    if (reader.name() == QLatin1String("alternate") &&
        tokenMain == QXmlStreamReader::StartElement) {
      //const QString s = QString((QChar)reader.attributes().value("value").toString().toInt());
      const int key =
        reader.attributes().value(QStringLiteral("value")).toString().toInt();
      charButton->addModeCharacter(Alternate, key);
    }

    //B:TODO: can we use "else if" here ?
  }

  return charButton;
}
