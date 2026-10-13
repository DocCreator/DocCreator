#include "DocumentTextLayout.hpp"

#include <fstream>

#include <QImage>

#include "models/character.h"
#include "models/characterdata.h"
#include "models/font.h"

TextMeasurement
measureText(const QString &filePath, const Models::Font &font)
{
  TextMeasurement measurement;

  std::ifstream file(filePath.toStdString().c_str());
  if (! file.is_open()) {
    return measurement;
  }

  std::string line;
  while (getline(file, line)) {

    measurement.rawLines.push_back(line);

    LineText lineText;
    int lineLong = 0;
    const size_t lineSize = line.size();

    /*
      B: this counts every character of the line, including those the font
      does not hold, while the sum of the heights below only counts those
      it does. The mean is then smaller than any of the heights it
      averages. Kept as it is: it decides the layout of every document the
      generator has produced so far.
    */
    measurement.numCharacters += static_cast<int>(lineSize);

    for (size_t i = 0; i < lineSize; ++i) {

      const QString charDisplay(line.at(i));
      const Models::Character *character = font.getCharacter(charDisplay);

      //a character the font does not hold is left out
      if (character != nullptr && ! charDisplay.isEmpty()) {

	const QImage charImage =
	  character->getRandomCharacterData().getImage();

	const int h = charImage.height();
	if (measurement.maxCharHeight < h) {
	  measurement.maxCharHeight = h;
	}

	measurement.meanCharHeight += h;
	lineText.meanCharHeight += h;
	lineLong += charImage.width();
	lineText.charList.push_back(charDisplay);
      }
    }

    if (measurement.maxLineLong < lineLong) {
      measurement.maxLineLong = lineLong;
    }
    lineText.Long = lineLong;
    //B: same divisor as above, and a NaN for an empty line, 0 divided by 0.
    //Kept as it is, for the same reason.
    lineText.meanCharHeight /= lineSize;

    measurement.totalLineLong += lineLong;
    measurement.lines.push_back(lineText);
  }

  if (measurement.numCharacters > 0) {
    measurement.meanCharHeight /= measurement.numCharacters;
  }

  return measurement;
}


void
splitParagraphsToFitColumns(QList<LineText> &lines,
			    const Models::Font &font,
			    int columnsWidth,
			    int columnsHeight,
			    int lineSpacing,
			    int meanCharHeight)
{
  int heightFilledTextBlock = 0;
  QList<LineText> result;
  result.reserve(lines.size());

  for (LineText &paragraph : lines) {

    // calculate paragraph height
    const int nbLinesOfParagraph = paragraph.Long / columnsWidth + 1;
    paragraph.Height =
      (nbLinesOfParagraph - 1) * (paragraph.meanCharHeight + lineSpacing);
    paragraph.Height = paragraph.Height * 0.85;

    const int heightChecker =
      heightFilledTextBlock + paragraph.Height - lineSpacing;

    // at the end of text block
    if (heightChecker > columnsHeight) { // split this paragraph into 2

      LineText head;
      head.meanCharHeight = paragraph.meanCharHeight;
      LineText tail;
      tail.meanCharHeight = paragraph.meanCharHeight;

      const int delta_height = columnsHeight - heightFilledTextBlock;

      //find the splitted position
      int spaceIndex = 0;
      int spaceIndexNext = paragraph.charList.indexOf(QStringLiteral(" "));
      while (spaceIndexNext != -1) {
	// check word long
	int wordLong = 0;
	for (int i = spaceIndex; i <= spaceIndexNext; ++i) {
	  const Models::Character *character =
	    font.getCharacter(paragraph.charList.at(i));
	  if (character != nullptr) {
	    wordLong += character->getRandomCharacterData().getImage().width();
	  }
	}
	const int numLines = (head.Long + wordLong) / columnsWidth + 1;
	const int checkerH = numLines * (meanCharHeight + lineSpacing);
	if (checkerH < delta_height) {
	  for (int i = spaceIndex; i <= spaceIndexNext; ++i) {
	    head.charList << paragraph.charList.at(i);
	  }
	  head.Long += wordLong;
	  head.Height = checkerH;
	}
	else {
	  for (int i = spaceIndex; i <= spaceIndexNext; ++i) {
	    tail.charList << paragraph.charList.at(i);
	  }
	  tail.Long += wordLong;
	}
	// move to next word
	spaceIndex = spaceIndexNext;
	spaceIndexNext =
	  paragraph.charList.indexOf(QStringLiteral(" "), spaceIndex + 1);
      }

      // update the height of filled text
      heightFilledTextBlock = paragraph.Height - head.Height;

      result.push_back(head);
      result.push_back(tail);
    }
    else { // continue to add paragraph to the current textblock
      heightFilledTextBlock = heightChecker;
      result.push_back(paragraph);
    }
  }

  lines = result;
}
