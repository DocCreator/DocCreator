#ifndef DOCCHARACTER_H
#define DOCCHARACTER_H

#include <QObject>
#include <QRectF>
#include <framework_global.h>

#include "leaf.h"
#include <QPoint>

namespace Doc
{
class FRAMEWORK_EXPORT DocCharacter : public Leaf
{
public:
    DocCharacter(const QString &display, int id, Document* document, const QRectF &r, float level=0)
      : Leaf(document),
    _display(display),
    _id(id),
    _degradationLevel(level),
    _rect(r)
    {
      setLength(1);
    }

    DocCharacter(const QString &display, int id, Document* document, float level=0)
      : Leaf(document),
    _display(display),
    _id(id),
    _degradationLevel(level),
    _rect(0, 0, 0, 0)
    {
        setLength(1);
    }

    DocCharacter* clone() { 
      DocCharacter* c = new DocCharacter(_display, _id, this->getDocument(), _degradationLevel); 
      return c; 
    }

    QString getDisplay() const { return _display; }
    int getId() const { return _id; }

    float getDegradationLevel() const { return _degradationLevel; }
    void setDegradationLevel(float level) { _degradationLevel=level; }


    int x() const { return _rect.x(); }
    int y() const { return _rect.y(); }
    int width() const { return _rect.width(); }
    int height() const { return _rect.height(); }

    void setX(int x) { _rect.setX(x); }
    void setY(int y) { _rect.setY(y); }
    void setWidth(int width) { _rect.setWidth(width); }
    void setHeight(int height) { _rect.setHeight(height); }

#ifdef STORE_SEEDPOINTS 
    QList<QPoint> getSeedPointsFB() const {return _seedPointsFB;}
    QList<QPoint> getSeedPointsBF() const {return _seedPointsBF;}

    //B:TODO:API: it is not a "set" but a "add" !?!
    void setSeedPointsFB(QList<QPoint> seedPoints) {_seedPointsFB +=(seedPoints);}
    void setSeedPointsBF(QList<QPoint> seedPoints) {_seedPointsBF +=(seedPoints);}
    void setSeedPointFB(QPoint seedPoint) {_seedPointsFB.push_back(seedPoint);}
    void setSeedPointBF(QPoint seedPoint) {_seedPointsBF.push_back(seedPoint);}
#endif //STORE_SEEDPOINTS
    
private:

    QString _display;
    int _id;
    float _degradationLevel;
    QRectF _rect; //B: why a QRectF and not a QRect ???
#ifdef STORE_SEEDPOINTS 
    QList<QPoint> _seedPointsFB;
    QList<QPoint> _seedPointsBF;
#endif //STORE_SEEDPOINTS
};
}


#endif // DOCCHARACTER_H
