#ifndef FRAMEWORKTEXT_H
#define FRAMEWORKTEXT_H

#include <QtTest/QtTest>

#include "models/textdocument.h"

using namespace Models;

class FrameworkTest: public QObject
 {
     Q_OBJECT
 private slots:
     void initTestCase();
     void myFirstTest();
     void addCharacters();
     void StringTests();
     void ParagraphsTests();
     void testOffsetPosition();
     void documentString();
     void removeCharacters();
     void cleanupTestCase()
     { qDebug("called after myFirstTest and mySecondTest"); }

 private:
     TextDocument* _document;
     QString constructQStringWithStyle(QList<DocumentString*> list);
     QString constructQString(QList<DocumentCharacter*> list);
 };

QTEST_MAIN(FrameworkTest)
#include "moc_frameworktest.cpp"

#endif // FRAMEWORKTEXT_H
