#include "frameworktest.h"

#include <iostream>

using namespace std;

void FrameworkTest::initTestCase()
{
    Style* s = new Style("noStyle");
    this->_document = new TextDocument(s);
    this->_document->add(new DocumentParagraph(s));
}

void FrameworkTest::myFirstTest()
{
    QVERIFY(this->_document->length() == 1);
}

void FrameworkTest::addCharacters()
{
    this->_document->changeStyle(new Style("style1"));
    this->_document->add(new DocumentCharacter("alpha", 1));
    this->_document->add(new DocumentCharacter(" ", 1));
    this->_document->add(new DocumentCharacter("bravo", 1));
    this->_document->add(new DocumentCharacter(" ", 1));
    this->_document->add(new DocumentCharacter("charlie", 1));
    QVERIFY(this->_document->length() == 6);
}

void FrameworkTest::StringTests()
{
    QString src = "", dst = "";
    QString tmp, tmp2;
    Style* no = new Style("noStyle");
    Style* s1 = new Style("*1*");
    Style* s2 = new Style("*2*");
    Style* s3 = new Style("*3*");

    DocumentCharacter* a = new DocumentCharacter("a", 0);
    DocumentCharacter* b = new DocumentCharacter("b", 0);
    DocumentCharacter* c = new DocumentCharacter("c", 0);
    DocumentCharacter* d = new DocumentCharacter("d", 0);
    DocumentCharacter* e = new DocumentCharacter("e", 0);
    DocumentCharacter* f = new DocumentCharacter("f", 0);

    /* DocumentString vide (init + remove before/after) */
    DocumentString* string = new DocumentString(no);
    QVERIFY(string->length() == 0);

    string->removeAfterCursor();
    QVERIFY(string->length() == 0);

    string->removeBeforeCursor();
    QVERIFY(string->length() == 0);
    QVERIFY(string->getOffsetPosition() == 0);

    /* DocumentString "a" (remove before/after, setOffsetPosition) */
    string->add(a);
    QVERIFY(string->getOffsetPosition() == 1);
    QVERIFY(string->length() == 1);

    string->removeBeforeCursor();
    QVERIFY(string->length() == 0);

    string->add(a);
    QVERIFY(string->getOffsetPosition() == 1);
    QVERIFY(string->length() == 1);

    string->removeAfterCursor();
    QVERIFY(string->length() == 1);

    string->setOffsetPosition(0);
    string->removeAfterCursor();
    QVERIFY(string->length() == 0);

    /* DocumentString "abc" (offset position, select, remove selection) */
    string->add(a);
    string->add(b);
    string->add(c);
    string->setOffsetPosition(0);
    QVERIFY(string->getOffsetPosition() == 0);
    QVERIFY(string->length() == 3);

    string->selectText(0, 3);
    string->removeAfterCursor();
    QVERIFY(string->length() == 0);

    /* DocumentString "abcdef" (everything) */
    string->add(a);
    string->add(b);
    string->add(c);
    string->add(d);
    string->add(e);
    string->add(f);
    string->selectText(-1000, 3999);
    src = "abcdef";
    dst = constructQString(string->getSelection()->getCharacters());
    QVERIFY(string->length() == src.length());
    QVERIFY(src == dst);

    string->selectText(1, 2);
    string->removeBeforeCursor();
    src = "adef";
    dst = constructQString(string->getCharacters());
    QVERIFY(string->length() == src.length());
    QVERIFY(src == dst);

    string->removeAfterCursor();
    src = "aef";
    dst = constructQString(string->getCharacters());
    QVERIFY(string->length() == src.length());
    QVERIFY(src == dst);

    string->removeBeforeCursor();
    src = "ef";
    dst = constructQString(string->getCharacters());
    QVERIFY(string->length() == src.length());
    QVERIFY(src == dst);

    string->removeBeforeCursor();
    string->removeBeforeCursor();
    string->removeBeforeCursor();
    string->removeBeforeCursor();
    src = "ef";
    dst = constructQString(string->getCharacters());
    QVERIFY(string->length() == src.length());
    QVERIFY(src == dst);

    string->setOffsetPosition(10000);
    string->removeBeforeCursor();
    src = "e";
    dst = constructQString(string->getCharacters());
    QVERIFY(string->length() == src.length());
    QVERIFY(src == dst);

    string->removeAfterCursor();
    src = "e";
    dst = constructQString(string->getCharacters());
    QVERIFY(string->length() == src.length());
    QVERIFY(src == dst);
    //cout << "Src : " + src.toStdString() << endl;
    //cout << "Dst : " + dst.toStdString() << endl;
}

void FrameworkTest::ParagraphsTests()
{
    QString src = "", dst = "", toFind = "";
    QString tmp, tmp2;
    Style* no = new Style("*noStyle*");
    Style* s1 = new Style("*1*");
    Style* s2 = new Style("*2*");
    Style* s3 = new Style("*3*");

    DocumentCharacter* a = new DocumentCharacter("a", 0);
    DocumentCharacter* b = new DocumentCharacter("b", 0);
    DocumentCharacter* c = new DocumentCharacter("c", 0);
    DocumentCharacter* d = new DocumentCharacter("d", 0);
    DocumentCharacter* e = new DocumentCharacter("e", 0);
    DocumentCharacter* f = new DocumentCharacter("f", 0);
    DocumentString* abc1 = new DocumentString(s1);
    abc1->add(a); abc1->add(b); abc1->add(c);
    DocumentString* abc2 = new DocumentString(s2);
    abc2->add(a); abc2->add(b); abc2->add(c);
    DocumentString* abc2_1 = new DocumentString(s2);
    abc2_1->add(a); abc2_1->add(b); abc2_1->add(c);
    DocumentString* abc2_2 = new DocumentString(s2);
    abc2_2->add(a); abc2_2->add(b); abc2_2->add(c);
    DocumentString* abcdef1 = new DocumentString(s1);
    abcdef1->add(a); abcdef1->add(b); abcdef1->add(c); abcdef1->add(d); abcdef1->add(e); abcdef1->add(f);
    DocumentString* abcdef1_1 = new DocumentString(s1);
    abcdef1_1->add(a); abcdef1_1->add(b); abcdef1_1->add(c); abcdef1_1->add(d); abcdef1_1->add(e); abcdef1_1->add(f);
    DocumentString* abcdef1_2 = new DocumentString(s1);
    abcdef1_2->add(a); abcdef1_2->add(b); abcdef1_2->add(c); abcdef1_2->add(d); abcdef1_2->add(e); abcdef1_2->add(f);
    DocumentString* a3 = new DocumentString(s3);
    a3->add(a);
    DocumentString* abcdef3_1 = new DocumentString(s3);
    abcdef3_1->add(a); abcdef3_1->add(b); abcdef3_1->add(c); abcdef3_1->add(d); abcdef3_1->add(e); abcdef3_1->add(f);

    /* DocumentParagraph vide (init + styles) */
    DocumentParagraph* p = new DocumentParagraph(no);
    src = "\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    src = "*noStyle*\n";
    dst = constructQStringWithStyle(p->getStrings());
    QVERIFY(src == dst);

    /* DocumentParagraph "abc" no-style (add, getCharacters) */
    p->add(a);
    p->add(b);
    p->add(c);
    src = "abc\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    src = "*noStyle*abc*noStyle*\n";
    dst = constructQStringWithStyle(p->getStrings());
    QVERIFY(src == dst);

    /* DocumentParagraph "abcabcabc" multiple-style (add, getCharacters, getStrings) */
    p->add(abc1);
    p->add(abc2);
    src = "abcabcabc\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    src = "*noStyle*abc*1*abc*2*abc*noStyle*\n";
    dst = constructQStringWithStyle(p->getStrings());
    QVERIFY(src == dst);

    /* DocumentParagraph vide (select all + remove selection + remove before/after) */
    p->selectText(-1000, 4999);
    p->removeAfterCursor();
    src = "\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    /* DocumentParagraph vide (select all + remove selection + remove before/after) */
    p->add(abcdef1);
    p->add(abc2_2);
    p->add(abcdef1_2);
    src = "abcdefabcabcdef\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    toFind = "cabc";
    p->selectText(src.indexOf(toFind), toFind.length());
    p->removeAfterCursor();
    src = "abcdefabdef\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    src = "*1*abcdef*2*ab*1*def*noStyle*\n";
    dst = constructQStringWithStyle(p->getStrings());
    QVERIFY(src == dst);

    p->removeAfterCursor();
    src = "abcdefabef\n";
    dst = constructQString(p->getCharacters());
    cout << "Src : " + src.toStdString() << endl;
    cout << "Dst : " + dst.toStdString() << endl;
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    QString beforeCursor = "abcdefa";
    p->setOffsetPosition(beforeCursor.length());
    p->removeBeforeCursor();
    src = "abcdefbef\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    p->removeBeforeCursor();
    p->removeBeforeCursor();
    p->removeBeforeCursor();
    p->removeBeforeCursor();
    p->removeBeforeCursor();
    src = "abef\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    p->removeBeforeCursor();
    p->removeBeforeCursor();
    p->removeBeforeCursor();
    src = "bef\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    p->removeAfterCursor();
    p->removeAfterCursor();
    src = "f\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    p->removeAfterCursor();
    p->removeAfterCursor();
    src = "\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    /* DocumentParagraph : "abcabcdefabcdef" (select, changeStyle of selection) */
    p->add(abc2_1);
    p->add(abcdef3_1);
    p->add(abcdef1_1);
    src = "abcabcdefabcdef\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    src = "*2*abc*3*abcdef*1*abcdef*noStyle*\n";
    dst = constructQStringWithStyle(p->getStrings());
    QVERIFY(src == dst);

    cout << "TMPP" << endl;
    src = "abcabcdefabcdef\n";
    toFind = "ef";
    p->selectText(src.indexOf(toFind), toFind.length());
    p->changeStyle(no);
    src = "abcabcdefabcdef\n";
    dst = constructQString(p->getCharacters());
    cout << "Src : " + src.toStdString() << endl;
    cout << "Dst : " + dst.toStdString() << endl;
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    src = "*2*abc*3*abcd*noStyle*ef*1*abcdef*noStyle*\n";
    dst = constructQStringWithStyle(p->getStrings());
    QVERIFY(src == dst);

    src = "abcabcdefabcdef\n";
    toFind = "cabcdef";
    p->selectText(src.indexOf(toFind), toFind.length());
    p->changeStyle(no);
    src = "abcabcdefabcdef\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    src = "*2*ab*noStyle*cabcdef*1*abcdef*noStyle*\n";
    dst = constructQStringWithStyle(p->getStrings());
    QVERIFY(src == dst);

    src = "abcabcdefabcdef\n";
    toFind = "abca";
    p->selectText(src.indexOf(toFind), toFind.length());
    p->changeStyle(s3);
    src = "abcabcdefabcdef\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    src = "*3*abca*noStyle*bcdef*1*abcdef*noStyle*\n";
    dst = constructQStringWithStyle(p->getStrings());
    QVERIFY(src == dst);

    cout << "TEST" << endl;
    src = "abcabcdefabcdef\n";
    toFind = "bcd";
    p->selectText(src.indexOf(toFind), toFind.length());
    p->changeStyle(no);
    src = "abcabcdefabcdef\n";
    dst = constructQString(p->getCharacters());
    QVERIFY(p->length() == src.length());
    QVERIFY(src == dst);

    src = "*3*abca*noStyle*bcdef*1*abcdef*noStyle*\n";
    dst = constructQStringWithStyle(p->getStrings());
    cout << "Src : " + src.toStdString() << endl;
    cout << "Dst : " + dst.toStdString() << endl;
    QVERIFY(src == dst);

    //cout << "Src : " + src.toStdString() << endl;
    //cout << "Dst : " + dst.toStdString() << endl;
}

void FrameworkTest::testOffsetPosition()
{
    return;
    Style* s = new Style("noStyle");
    this->_document = new TextDocument();

    const QString abc = "abc";
    DocumentString* abcStr = new DocumentString(s);
    foreach (const QChar &c, abc)
    {
        abcStr->add(new DocumentCharacter(c.toAscii(), 1));
    }
    QString abcd = "abcd";

    this->_document->addParagraph();
    this->_document->add( abc);
    this->_document->addParagraph();
    this->_document->addString(abcd);

    QString tmp, tmp2;
    QAtomicInt occurABC = 0;
    this->_document->setOffsetPosition(occurABC);
    tmp = tmp.setNum(this->_document->getOffsetPosition());
    tmp2 = tmp2.setNum(this->_document->getParagraph()->length());
    cout << "Position : " + tmp.toStdString() + " ; " + tmp2.toStdString() << endl;

    this->_document->setOffsetPosition(occurABC+1);
    tmp = tmp.setNum(this->_document->getOffsetPosition());
    tmp2 = tmp2.setNum(this->_document->getParagraph()->length());
    cout << "Position : " + tmp.toStdString() + " ; " + tmp2.toStdString() << endl;

    QAtomicInt occurABCD = abc.length()+1;
    this->_document->setOffsetPosition(occurABCD);
    tmp = tmp.setNum(this->_document->getOffsetPosition());
    tmp2 = tmp2.setNum(this->_document->getParagraph()->length());
    cout << "Position : " + tmp.toStdString() + " ; " + tmp2.toStdString() << endl;

    QAtomicInt occurBeforeEnd = occurABCD+abcd.length()-1;
    this->_document->setOffsetPosition(occurBeforeEnd);
    tmp = tmp.setNum(this->_document->getOffsetPosition());
    tmp2 = tmp2.setNum(this->_document->getParagraph()->length());
    cout << "Position : " + tmp.toStdString() + " ; " + tmp2.toStdString() << endl;

    QAtomicInt occurEnd = occurABCD+abcd.length();
    this->_document->setOffsetPosition(occurEnd);
    tmp = tmp.setNum(this->_document->getOffsetPosition());
    tmp2 = tmp2.setNum(this->_document->getParagraph()->length());
    cout << "Position : " + tmp.toStdString() + " ; " + tmp2.toStdString() << endl;

    QAtomicInt occurAfterEnd = 1100;
    this->_document->setOffsetPosition(occurAfterEnd);
    tmp = tmp.setNum(this->_document->getOffsetPosition());
    tmp2 = tmp2.setNum(this->_document->getParagraph()->length());
    cout << "Position : " + tmp.toStdString() + " ; " + tmp2.toStdString() << endl;

    tmp = tmp.setNum(this->_document->length());
    cout << "Size : " + tmp.toStdString() << endl;
}

void FrameworkTest::documentString()
{
    return;
    cout << "**** documentString ****" << endl;
    this->_document = new TextDocument(new Style("noStyle"));
    this->_document->addParagraph();

    Style* s1 = new Style("style1");
    Style* s2 = new Style("style2");

    QString line1 = "0alpha bravo charlie1";//20
    QString line2 = "delta echo fockstrot2";//21
    QString line3 = "a3 b c";//6
    QString line4 = "d e f4";//6
    QString paragraph = "[noStyle]\n[\\noStyle]";
    QAtomicInt occur0 = line1.indexOf("0");
    QAtomicInt occur1 = line1.indexOf("1");
    QAtomicInt occur2 = line1.count()+line2.indexOf("2");
    QAtomicInt occur3 = line1.count()+1+line2.count()+line3.indexOf("3");
    QAtomicInt occur4 = line1.count()+1+line2.count()+1+line3.count()+line4.indexOf("4");

    QString src;
    src = src + "[noStyle]"+line1+"[\\noStyle]";
    src = src + "[style2]"+line2+"[\\style2]" + paragraph;
    src = src + "[style1]"+line3+"[\\style1]" + paragraph;
    src = src + "[style2]"+line4+"[\\style2]" + paragraph;

    //this->_document->changeStyle(s1);
    this->_document->addCharacter("a");
    this->_document->addCharacter("l");
    this->_document->addCharacter("p");
    this->_document->addCharacter("h");
    this->_document->addCharacter("a");
    this->_document->addCharacter(" ");
    this->_document->addCharacter("b");
    this->_document->addCharacter("r");
    this->_document->addCharacter("a");
    this->_document->addCharacter("v");
    this->_document->addCharacter("o");
    this->_document->addCharacter(" ");
    this->_document->addCharacter("c");
    this->_document->addCharacter("h");
    this->_document->addCharacter("a");
    this->_document->addCharacter("r");
    this->_document->addCharacter("l");
    this->_document->addCharacter("i");
    this->_document->addCharacter("e");

    this->_document->changeStyle(s2);
    this->_document->addCharacter("d");
    this->_document->addCharacter("e");
    this->_document->addCharacter("l");
    this->_document->addCharacter("t");
    this->_document->addCharacter("a");
    this->_document->addCharacter(" ");
    this->_document->addCharacter("e");
    this->_document->addCharacter("c");
    this->_document->addCharacter("h");
    this->_document->addCharacter("o");
    this->_document->addCharacter(" ");
    this->_document->addCharacter("f");
    this->_document->addCharacter("o");
    this->_document->addCharacter("c");
    this->_document->addCharacter("k");
    this->_document->addCharacter("s");
    this->_document->addCharacter("t");
    this->_document->addCharacter("r");
    this->_document->addCharacter("o");
    this->_document->addCharacter("t");

    this->_document->addParagraph();
    this->_document->changeStyle(s1);
    this->_document->addCharacter("a");
    this->_document->addCharacter(" ");
    this->_document->addCharacter("b");
    this->_document->addCharacter(" ");
    this->_document->addCharacter("c");

    this->_document->addParagraph();
    this->_document->changeStyle(s2);
    this->_document->addCharacter("d");
    this->_document->addCharacter(" ");
    this->_document->addCharacter("e");
    this->_document->addCharacter(" ");
    this->_document->addCharacter("f");

    this->_document->setOffsetPosition(occur0);
    this->_document->addCharacter("0");
    this->_document->setOffsetPosition(occur1);
    this->_document->addCharacter("1");
    this->_document->setOffsetPosition(occur2);
    this->_document->addCharacter("2");
    this->_document->setOffsetPosition(occur3);
    this->_document->addCharacter("3");
    this->_document->setOffsetPosition(occur4);
    this->_document->addCharacter("4");

    QString dst;

    foreach (const DocumentParagraph* p, this->_document->getParagraphs())
    {
        foreach (const DocumentString* s, p->getStrings())
        {
            dst = dst + "[" + s->getStyle()->getName() + "]";
            foreach (const DocumentCharacter* c, s->getCharacters())
            {
                dst = dst + c->getDisplay();
            }
            dst = dst + "[\\" + s->getStyle()->getName() + "]";
        }
    }

    cout << "Src : " + src.toStdString() << endl;
    cout << "Dst : " + dst.toStdString() << endl;

    QVERIFY(src == dst);
}

void FrameworkTest::removeCharacters()
{
    return;
    TextDocument* document = new TextDocument(new Style(""));

    Style* s1 = new Style("*1*");
    Style* s2 = new Style("*2*");
    Style* s3 = new Style("*3*");

    document->addParagraph();
    document->changeStyle(s1);
    document->addString("ligne1");
    document->changeStyle(s2);
    document->addString("ligne2");
    document->changeStyle(s3);
    document->addString("ligne3");
    document->addParagraph();
    document->changeStyle(s1);
    document->addString("ligne1");
    document->changeStyle(s2);
    document->addString("ligne2");
    document->changeStyle(s3);
    document->addString("ligne3");
    document->addParagraph();
    document->changeStyle(s1);
    document->addString("ligne1");
    document->addParagraph();
    document->changeStyle(s1);
    document->addString("ligne1");
    document->changeStyle(s2);
    document->addString("ligne2");

    /*
*1*ligne1*2*ligne2*3*ligne3
*1*ligne1*2*ligne2*3*ligne3
*1*ligne1
*1*ligne1*2*ligne2
    */


    QString src;
    src = src + "ligne1ligne2ligne3\n";
    src = src + "ligne1ligne2ligne3\n";
    src = src + "ligne1\n";
    src = src + "ligne1ligne2\n";

    QString toRemove = "ligne1\nli";
    QAtomicInt removeFrom = src.indexOf(toRemove);
    QAtomicInt removeTo = removeFrom + toRemove.count();

    document->remove(removeFrom, removeTo);
    document->remove(removeFrom-1, removeFrom+1);

    src = "";
    src = src + "*1*ligne1*2*ligne2*3*ligne3\n";
    src = src + "*1*ligne1*2*ligne2*3*ligne3\n";
    src = src + "*1*ligne1*2*ligne2\n";

    QString dst;
    foreach (const DocumentParagraph* p, document->getParagraphs())
    {
        foreach (const DocumentString* s, p->getStrings())
        {
            dst = dst + s->getStyle()->getName();
            foreach (const DocumentCharacter* c, s->getCharacters())
            {
                dst = dst + c->getDisplay();
            }
        }
    }

    cout << "Src : " + src.toStdString() << endl;
    cout << "Dst : " + dst.toStdString() << endl;

    QVERIFY(src == dst);
}

QString FrameworkTest::constructQStringWithStyle(QList<DocumentString*> list)
{
    QString res = "";
    foreach (const DocumentString* s, list)
    {
        res = res + s->getStyle()->getName();
        foreach (const DocumentCharacter* c, s->getCharacters())
            res = res + c->getDisplay();
    }

    return res;
}

QString FrameworkTest::constructQString(QList<DocumentCharacter*> list)
{
    QString res = "";
    foreach (const DocumentCharacter* c, list)
        res = res + c->getDisplay();

    return res;
}
