/*
  Copyright (C) 2011-2012 Van Cuong KIEU, Nicholas JOURNET, van-cuong.kieu@labri.fr
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef DOCCOMPONENT_H
#define DOCCOMPONENT_H


#include <QObject>
#include <QRectF>
#include <QMap>
#include <framework_global.h>
//#include <QImage>
#include <vector>
#include "leaf.h"

/**
  * \mainpage
  * This class header describe un unknown element of Document Image. At the firt time, the document image is loaded.
  * All components are extracted but they are still unknown elements called component element.
  *
  */
namespace Doc
{
  using Scanlines = QMap<int, std::vector<int> >; // map of scanline in one component.

class FRAMEWORK_EXPORT DocComponent : public Leaf
{
public :
  
 DocComponent(const Scanlines &sls, int id, Document *document) : 
  Leaf(document),
    _id(id),
    _rectF(0, 0, 0, 0),
    _sls(sls)
  {
    setLength(1); // implemented method of Element
  }

  DocComponent* clone() {
    DocComponent* dc = new DocComponent(_sls, _id, this->getDocument());
    return dc;
  }

    const Scanlines &getScanlines() const {return _sls;}
    //QImage* getImage() const {return _imgData;}

    int getId() const { return _id; }
    int x() const { return _rectF.x(); }
    int y() const { return _rectF.y(); }
    int width() const { return _rectF.width(); }
    int height() const { return _rectF.height(); }

    void setX(int x) { _rectF.setX(x); }
    void setY(int y) { _rectF.setY(y); }
    void setWidth(int width) { _rectF.setWidth(width); }
    void setHeight(int height) { _rectF.setHeight(height); }

private :
    int _id;
    QRectF _rectF; //B: Why a QRectF and not a QRect ????
    Scanlines _sls;
    //QImage* _imgData;
};
}
#endif // DOCCOMPONENT_H
