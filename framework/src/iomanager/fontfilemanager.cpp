#include "fontfilemanager.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QImage>
#include <QRgb>
#include <QStringList>
#include <QTextCodec>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "models/character.h"
#include "models/characterdata.h"
#include "models/font.h"

namespace IOManager {


Models::Font *
FontFileManager::readFont(const QString &filepath)
{
  if (isBOFFile(filepath)) {
    return fontFromBinary(filepath);
  }
  return fontFromXml(filepath);
}

bool
FontFileManager::writeFont(const Models::Font *font, const QString &filepath)
{
  if (filepath.endsWith(".of", Qt::CaseInsensitive)) {
    return fontToXml(font, filepath);
  }
  return fontToBinary(font, filepath);
}

Models::Font *
FontFileManager::fontFromXml(const QString &filepath)
{
  //B:TODO:UGLY: errors are not checked correctly ! What if file is corrupt ?

  //qDebug()<<"FontFileManager::fontFromXml("<<filepath<<")";

  QFile file(filepath);
  const bool ok = file.open(QFile::ReadOnly);
  if (!ok) {
    std::cerr << "Warning: unable to open input font file: " << filepath.toStdString()
              << "\n";
    return nullptr;
  }

  auto font = new Models::Font();

  QXmlStreamReader reader(&file);

  //Reading font from xml
  while (!reader.atEnd()) {
    if (reader.name() == "font" &&
        reader.tokenType() == QXmlStreamReader::StartElement) {
      font->setName(
        reader.attributes().value(QStringLiteral("name")).toString());

      while (!(reader.tokenType() == QXmlStreamReader::EndElement &&
               reader.name() == "font") &&
             !reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (reader.name() == "letter" &&
            token == QXmlStreamReader::StartElement) {
          const QString s =
            reader.attributes().value(QStringLiteral("char")).toString();

          Models::Character *ch = characterFromXml(reader);
          ch->setCharacterValue(s);
          if (ch->getCharacterDataCount() == 0) {
            std::cerr << "Warning: No picture found for letter \""
                      << s.toStdString()
                      << "\"\n in font file: " << filepath.toStdString()
                      << "\n";
            delete ch;
          } else {
            const bool addOk = font->addCharacter(ch);
            if (!addOk) {
              std::cerr << "Warning: letter \"" << s.toStdString()
                        << "\" is present several times in font file: "
                        << filepath.toStdString() << "\n";
            }
          }
        }
      }
    }
    reader.readNext();
  }
  //reading from xml finished

  file.close();

  if (font->getCharacters().empty()) {
    delete font;
    return nullptr;
  }

  return font;
}

Models::Font *
FontFileManager::fontFromDirectory(const QString &dirpath,
                                   const QMap<QString, QString> &matches)
{
  qDebug() << "importing font ........";
  QDir dir(dirpath);
  QString fontName = dir.dirName();
  auto font = new Models::Font(fontName);
  QStringList fileList = dir.entryList();

  for (const QString &filename : fileList) {

    //TODO: check Qt supported image formats
    if (filename.endsWith(QStringLiteral(".jpg")) ||
        filename.endsWith(QStringLiteral(".png")) ||
        filename.endsWith(QStringLiteral(".JPG")) ||
        filename.endsWith(QStringLiteral(".PNG")) ||
        filename.endsWith(QStringLiteral(".bmp")) ||
        filename.endsWith(QStringLiteral(
          ".BMP"))) //B:TODO:UGLY: check image formats supported by Qt
    {
      // format : fontname_U+code_order.*
      const int firstUnderscoreIndex = filename.indexOf('_');
      const int secondUnderscoreIndex =
        filename.indexOf('_', firstUnderscoreIndex + 1);
      const int dotExtensionIndex =
        filename.indexOf('.', secondUnderscoreIndex + 1);

      QString character =
        filename.mid(firstUnderscoreIndex + 1,
                     secondUnderscoreIndex - firstUnderscoreIndex - 1);
      if (character == QStringLiteral("[pasdecode]"))
        continue;

      // deal with unicode - 16 bit
      const int plusSeparationFirstIndex = character.indexOf('+');
      const int plusSeparationLastIndex = character.lastIndexOf('+');

      if (plusSeparationFirstIndex >= 0) { // style encoding
        if (plusSeparationFirstIndex !=
            plusSeparationLastIndex) { // combined character
          continue;
        } else { // single character
          QString code_style = character.mid(0, plusSeparationFirstIndex);
          if (code_style.toLower() == QStringLiteral("u")) { // Unicode style
            QString code =
              character.mid(plusSeparationFirstIndex + 1, character.size());
            if (code.length() == 4) { // Unicode style-16 bit

              unsigned long ucs2 =
                strtoul(code.toStdString().c_str(), nullptr, 16);
              QChar qchar = QChar((int)ucs2);
              character = QString(qchar);
              if (character == QLatin1String(""))
                continue; // unknown character.
              qDebug() << "\t char = " << character << " code= " << code;
            } else
              continue;
          } else
            continue;
        }
      } else { // style character ascii
      }

      if (matches.contains(character))
        character = matches.value(character);

      const QString id =
        filename.mid(secondUnderscoreIndex + 1,
                     dotExtensionIndex - secondUnderscoreIndex - 1);

      QImage image(dir.absoluteFilePath(filename));
      Models::CharacterData *charData;

      if (image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);

        const int w = image.width();
        const int h = image.height();

        for (int y = 0; y < h; ++y) {
          QRgb *d = (QRgb *)image.scanLine(y);
          for (int x = 0; x < w; ++x) {
            const QRgb rgb = d[x];
            d[x] = qRgba(qRed(rgb),
                         qGreen(rgb),
                         qBlue(rgb),
                         255 - qGray(rgb)); //B??? why ???
          }
        }
      }
      charData = new Models::CharacterData(image, id.toInt());

      font->addCharacter(character, charData);
    }
  }

  return font;
}

bool
FontFileManager::fontToXml(const Models::Font *font, const QString &filepath)
{
  assert(font);

  QFile file(filepath);
  const bool ok = file.open(QFile::WriteOnly);
  if (!ok) {
    std::cerr << "Warning: unable to open output font file: " << filepath.toStdString()
              << "\n";
    return false;
  }

  QXmlStreamWriter writer;
  writer.setAutoFormatting(true);
  writer.setAutoFormattingIndent(2);
  writer.setCodec(QTextCodec::codecForName("utf8"));
  writer.setDevice(&file);

  writer.writeStartDocument();

  writer.writeStartElement(QStringLiteral("font"));
  writer.writeAttribute(QStringLiteral("name"), font->getName());

  for (Models::Character *ch : font->getCharacters())
    characterToXml(ch, writer);

  writer.writeEndElement();

  writer.writeEndDocument();

  file.close();

  return true;
}

/*      PRIVATE METHODS      */
Models::Character *
FontFileManager::characterFromXml(QXmlStreamReader &reader)
{
  auto ch = new Models::Character();

  while (!(reader.tokenType() == QXmlStreamReader::EndElement &&
           reader.name() == "letter") &&
         !reader.atEnd()) {
    QXmlStreamReader::TokenType tokenMain = reader.readNext();

    if (reader.name() == "anchor" &&
        tokenMain == QXmlStreamReader::StartElement) {
      // GETTING ANCHORS (upLine, baseLine, leftLine, and rightLine)
      //int upLine, baseLine, leftLine, rightLine;

      reader.readNext();
      while (!(reader.tokenType() == QXmlStreamReader::EndElement &&
               reader.name() == "anchor") &&
             !reader.atEnd()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
          if (reader.name() == "upLine") {
            reader.readNext();
            ch->setUpLine(reader.text().toString().toDouble());
          }
          if (reader.name() == "baseLine") {
            reader.readNext();
            ch->setBaseLine(reader.text().toString().toDouble());
          }
          if (reader.name() == "leftLine") {
            reader.readNext();
            ch->setLeftLine(reader.text().toString().toDouble());
          }
          if (reader.name() == "rightLine") {
            reader.readNext();
            ch->setRightLine(reader.text().toString().toDouble());
          }
        }
      }
    }
    if (reader.name() == "picture" &&
        tokenMain == QXmlStreamReader::StartElement) {
      Models::CharacterData *charData = characterDataFromXml(reader);
      ch->add(charData);
    }
  }
  return ch;
}

void
FontFileManager::characterToXml(const Models::Character *character,
                                QXmlStreamWriter &writer)
{
  writer.writeStartElement(QStringLiteral("letter"));
  writer.writeAttribute(QStringLiteral("char"), character->getCharacterValue());

  writer.writeStartElement(QStringLiteral("anchor"));
  writer.writeTextElement(QStringLiteral("upLine"),
                          QString::number(character->getUpLine()));
  writer.writeTextElement(QStringLiteral("baseLine"),
                          QString::number(character->getBaseLine()));
  writer.writeTextElement(QStringLiteral("leftLine"),
                          QString::number(character->getLeftLine()));
  writer.writeTextElement(QStringLiteral("rightLine"),
                          QString::number(character->getRightLine()));
  writer.writeEndElement();
  for (Models::CharacterData *charData : character->getAllCharacterData()) {
    characterDataToXml(charData, writer);
  }
  writer.writeEndElement();
}

//TODO: better handling of errors. For now, we return -1 on error
int
FontFileManager::saveBaseLineInformation(const QString &path,
                                         qreal base,
                                         qreal right,
                                         const QString &character)
{
  QDomDocument doc(QStringLiteral("fontDocument"));
  QFile file(path);

  if (!file.open(QIODevice::ReadOnly)) {
    std::cerr << "Unable to load font: " << path.toStdString() << std::endl;
    return -1;
  }
  // Parse file
  if (!doc.setContent(&file)) {
    std::cerr << "Unable to parse font:" << path.toStdString() << std::endl;
    file.close();
    return -1;
  }
  file.close();
  //QDomElement docElem = doc.documentElement(); //B:unused
  QDomElement write_elem = doc.createElement(QStringLiteral("information")); //B:unused

  QDomNodeList letters = doc.elementsByTagName(QStringLiteral("letter"));

  if (letters.size() < 1) {
    //       std::cout << "Cannot find letter" << endl;
    return -1;
  }

  for (int i = 0; i < letters.size(); ++i) {
    if (letters.at(i).toElement().attribute(QStringLiteral("char"),
                                            QLatin1String("")) == character) {
      //change the baseline value
      //std::cout << "avant" << letters.at(i).firstChildElement("anchor").elementsByTagName("baseLine").at(0).firstChild().nodeValue().toStdString()  << std::endl;
      letters.at(i)
        .firstChildElement(QStringLiteral("anchor"))
        .elementsByTagName(QStringLiteral("baseLine"))
        .at(0)
        .firstChild()
        .setNodeValue(QString::number(base, 'd', 0));
      letters.at(i)
        .firstChildElement(QStringLiteral("anchor"))
        .elementsByTagName(QStringLiteral("rightLine"))
        .at(0)
        .firstChild()
        .setNodeValue(QString::number(right, 'd', 0));
      // std::cout << "aprs" <<
      // letters.at(i).firstChildElement("anchor").elementsByTagName("baseLine").at(0).firstChild().nodeValue().toStdString()
      // << std::endl;
    }
  }

  if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
    std::cerr << "Unable to re-open font XML file: " << path.toStdString()
              << std::endl;
    return -1;
  }

  QByteArray xml = doc.toByteArray();
  file.write(xml);
  file.close();

  return 0;
}

//int idx = 0;
Models::CharacterData *
FontFileManager::characterDataFromXml(QXmlStreamReader &reader)
{
  //GETTING CHAR IMAGE DATA
  int imgWidth = 0, imgHeight = 0, format = 0;
  QString pixelsData;

  //Get the id
  const int id =
    reader.attributes().value(QStringLiteral("id")).toString().toInt();

  while (!(reader.tokenType() == QXmlStreamReader::EndElement &&
           reader.name() == "imageData") &&
         !reader.atEnd()) {
    QXmlStreamReader::TokenType token = reader.readNext();
    if (token == QXmlStreamReader::StartElement) {
      if (reader.name() == "width") {
        reader.readNext();
        imgWidth = reader.text().toString().toInt();
      }
      if (reader.name() == "height") {
        reader.readNext();
        imgHeight = reader.text().toString().toInt();
      }
      if (reader.name() == "format") {
        reader.readNext();
        format = reader.text().toString().toInt();
      }
      if (reader.name() == "data") {
        reader.readNext();
        pixelsData = reader.text().toString();
      }
    }
  }

  if (imgWidth <= 0 || imgHeight <= 0) {
    qDebug()<<"Warning: while reading font, found invalid image dimensions: imgWidth="<<imgWidth<<" imgHeight="<<imgHeight<<" pixels.size()="<<pixelsData.split(QStringLiteral(",")).size()<<"\n";
  }

  //building QImage
  QImage img(imgWidth, imgHeight, static_cast<QImage::Format>(format));

  //B:TODO:OPTIM: who thought it would be a good idea to save images in ASCII format ???...
  // We should save a binary blob, or even better, a compressed png image...

  QStringList pixels = pixelsData.split(QStringLiteral(","));
  assert(pixels.size() == imgWidth * imgHeight);
  QStringList::const_iterator it = pixels.constBegin();

  if (img.format() == QImage::Format_ARGB32) {
    for (int y = 0; y < imgHeight; ++y) {
      QRgb *d = (QRgb *)img.scanLine(y);
      for (int x = 0; x < imgWidth; ++x) {
        d[x] = it->toUInt(); //B: conversion from QString to UInt !
        ++it;
      }
    }
  } else {
    for (int y = 0; y < imgHeight; ++y) {
      for (int x = 0; x < imgWidth; ++x) {
        //img.setPixel(x, y, pixels.at(y*imgWidth+x).toUInt()); //B: conversion from QString to UInt !

        img.setPixel(x, y, it->toUInt()); //B: conversion from QString to UInt !
        ++it;
      }
    }
  }

  //    QString
  //    name_idx("/home/kvcuong/AncientDocumentCreator/trunk/documentImageCreator/ImageOfFonts/letter_");
  //    name_idx.append(QString::number(idx)); name_idx.append(".png"); idx++;
  //    img->save(name_idx);

  return new Models::CharacterData(img, id);
}

void
FontFileManager::characterDataToXml(const Models::CharacterData *charData,
                                    QXmlStreamWriter &writer)
{
  writer.writeStartElement(QStringLiteral("picture"));
  writer.writeAttribute(QStringLiteral("id"),
                        QString::number(charData->getId()));
  writer.writeStartElement(QStringLiteral("imageData"));
  writer.writeTextElement(QStringLiteral("width"),
                          QString::number(charData->getImage().width()));
  writer.writeTextElement(QStringLiteral("height"),
                          QString::number(charData->getImage().height()));
  writer.writeTextElement(QStringLiteral("format"),
                          QString::number(charData->getImage().format()));
  writer.writeStartElement(QStringLiteral("data"));

  //B:TODO:OPTIM: storing each pixel as a QString is not clever.
  //B:TODO:OPTIM: could we directly write the "joined" QString ?
  //B:TODO:OPTIM: do not access image with pixel() !
  QStringList pixelData;
  const int h = charData->getImage().height();
  const int w = charData->getImage().width();
  pixelData.reserve(w * h);
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      pixelData << QString::number(charData->getImage().pixel(x, y));
    }
  }
  writer.writeCharacters(pixelData.join(QStringLiteral(",")));

  writer.writeEndElement();
  writer.writeEndElement();
  writer.writeEndElement();
}

const int HEADER = ('B' << 24 | 'O' << 16 | 'F' << 8);
const int HEADER_MASK = 0xFFFFFF00;

bool
FontFileManager::isBOFFile(const QString &filename)
{
  QFile file(filename);
  file.open(QIODevice::ReadOnly);
  QDataStream in(&file);

  if (in.status() != QDataStream::Ok) {
    std::cerr<<"ERROR: unable to open: "<<filename.toStdString()<<"\n";
    return false;
  }
  uint32_t h;
  in >> h;
  if (in.status() != QDataStream::Ok) {
    return false;
  }

  if ((h & HEADER_MASK) != HEADER) {
    return false;
  }
  return true;
}


bool
FontFileManager::fontToBinary(const Models::Font *font, const QString &filepath)
{
  assert(font);

  QFile file(filepath);

  if (! file.open(QFile::WriteOnly)) {
    std::cerr<<"Error: unable to open output font file: "<<filepath.toStdString()<<"\n";
    return false;
  }

  QDataStream out(&file);

  const uint32_t header = HEADER;
  out << header;
  if (out.status() != QDataStream::Ok)
    return false;

  out.setVersion(QDataStream::Qt_4_0);

  out << font->getName();

  const Models::CharacterMap &characters = font->getCharacters();
  const uint32_t numCharacters = characters.size();
  out << numCharacters;
  for (Models::Character *ch : characters) {
    out<<ch->getCharacterValue();
    out<<ch->getUpLine()<<ch->getBaseLine()<<ch->getLeftLine()<<ch->getRightLine();
    const Models::CharacterDataList &cD = ch->getAllCharacterData();
    const uint32_t numVersions = cD.size();
    out << numVersions;
    for (Models::CharacterData *charData : cD) {
      out<<charData->getId();
      out<<charData->getImage();
    }

  }

  if (out.status() != QDataStream::Ok)
    return false;

  return true;
}

Models::Font *
FontFileManager::fontFromBinary(const QString &filepath)
{
  QFile file(filepath);

  if (! file.open(QFile::ReadOnly)) {
    std::cerr<<"Error: unable to open input font file: "<<filepath.toStdString()<<"\n";
    return nullptr;
  }

  QDataStream in(&file);

  uint32_t header = 0;
  in >> header;
  if (in.status() != QDataStream::Ok)
    return nullptr;
  if ((header & HEADER_MASK) != HEADER) {
    return nullptr;
  }

  in.setVersion(QDataStream::Qt_4_0);

  QString fontName;
  in >> fontName;

  uint32_t numCharacters = 0;
  in >> numCharacters;

  if (numCharacters == 0) {
    return nullptr;
  }

  auto font = new Models::Font();
  font->setName(fontName);

  for (uint32_t i=0; i<numCharacters; ++i) {
    QString s;
    in >> s;
    qreal upLine, baseLine, leftLine, rightLine;
    in >> upLine >> baseLine >> leftLine >> rightLine;
    if (in.status() != QDataStream::Ok)
      break;
    auto ch = new Models::Character();
    ch->setCharacterValue(s);
    ch->setUpLine(upLine);
    ch->setBaseLine(baseLine);
    ch->setLeftLine(leftLine);
    ch->setRightLine(rightLine);
    uint32_t numVersions = 0;
    in >> numVersions;
    for (uint32_t j=0; j<numVersions; ++j) {
      int id;
      in >> id;
      QImage img;
      in >> img;
      Models::CharacterData *charData = new Models::CharacterData(img, id);
      ch->add(charData);
    }

    if (ch->getCharacterDataCount() == 0) {
      std::cerr << "Warning: No picture found for letter \""
		<< s.toStdString()
		<< "\"\n in font file: " << filepath.toStdString()
		<< "\n";
      delete ch;
    }
    else {
      const bool addOk = font->addCharacter(ch);
      if (!addOk) {
	std::cerr << "Warning: letter \"" << s.toStdString()
		  << "\" is present several times in font file: "
		  << filepath.toStdString() << "\n";
      }
    }

  }

  return font;
}


} //namespace IOManager
