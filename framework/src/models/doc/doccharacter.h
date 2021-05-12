#ifndef DOCCHARACTER_H
#define DOCCHARACTER_H

#include <QObject>
#include <QRectF>
#include <framework_global.h>

#include "leaf.h"

namespace Doc
{
class FRAMEWORK_EXPORT DocCharacter : public Leaf
{
public:
    DocCharacter(const QString &display, int id, Document* document, const QRectF &r)
      : Leaf(document),
    _display(display),
    _id(id),
    _rect(r)
    {
      setLength(1);
    }

    DocCharacter(const QString &display, int id, Document* document)
      : Leaf(document),
    _display(display),
    _id(id),
    _rect(0, 0, 0, 0)
    {
        setLength(1);
    }

    DocCharacter* clone() { 
      DocCharacter* c = new DocCharacter(_display, _id, this->getDocument()); 
      return c; 
    }

    QString getDisplay() const { return _display; }
    int getId() const { return _id; }

    int x() const { return _rect.x(); }
    int y() const { return _rect.y(); }
    int width() const { return _rect.width(); }
    int height() const { return _rect.height(); }

    void setX(int x) { _rect.setX(x); }
    void setY(int y) { _rect.setY(y); }
    void setWidth(int width) { _rect.setWidth(width); }
    void setHeight(int height) { _rect.setHeight(height); }

private:

    QString _display;
    int _id;
    QRectF _rect; //B: why a QRectF and not a QRect ???
};
}


#endif // DOCCHARACTER_H
