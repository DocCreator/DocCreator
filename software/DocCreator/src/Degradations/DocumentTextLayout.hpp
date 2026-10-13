#ifndef DOCUMENTTEXTLAYOUT_HPP
#define DOCUMENTTEXTLAYOUT_HPP

#include <string>

#include <QList>
#include <QString>
#include <QVector>

namespace Models {
class Font;
}

/*
  One paragraph of the text to lay out: the characters of a line of the
  input file that the font holds, and what the layout needs to know about
  them.

  B: this was held by pointer, in a QList<LineText *> whose elements were
  deleted by hand at the end of each of the three functions below. A
  paragraph that was split in two was not deleted, and not reachable
  anymore.
*/
struct LineText
{
  QList<QString> charList;
  int Long = 0;
  int Height = 0;
  float meanCharHeight = 0.f;
};


/*
  What measureText() reads from a text file: one LineText per line, the raw
  lines, and the sizes the layout is computed from.
*/
struct TextMeasurement
{
  QList<LineText> lines;
  QVector<std::string> rawLines;

  int meanCharHeight = 0;  //over all the characters, not per line
  int maxCharHeight = 0;
  int maxLineLong = 0;
  int totalLineLong = 0;
  int numCharacters = 0;   //characters the font holds

  bool isEmpty() const { return numCharacters == 0; }
};


/*
  Read @a filePath line by line and measure each line with @a font: the
  width of a line is the sum of the widths of the character images the font
  holds for it, and the characters it does not hold are left out.

  B: this was the same forty lines in fileSplit(), createNewDocument() and
  createNewBigDocument(). Each of the three also measured the largest
  distance of a baseline to 100 and never read it.
*/
TextMeasurement measureText(const QString &filePath, const Models::Font &font);


/*
  Cut in two the paragraphs of @a lines that do not fit in what is left of
  a column of @a columnsHeight, at the last word that fits.

  B: this was the same sixty lines in createNewDocument() and
  createNewBigDocument().
*/
void splitParagraphsToFitColumns(QList<LineText> &lines,
				 const Models::Font &font,
				 int columnsWidth,
				 int columnsHeight,
				 int lineSpacing,
				 int meanCharHeight);

#endif /* ! DOCUMENTTEXTLAYOUT_HPP */
