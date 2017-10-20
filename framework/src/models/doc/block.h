#ifndef BLOCK_H
#define BLOCK_H

#include <QString>
#include <framework_global.h>

namespace Doc
{
    class FRAMEWORK_EXPORT Block
    {
    public:

      //B:TODO:API: counter intuitive to take w,h,x,y & not x,y,w,h !
      explicit Block(int w=0, int h=0, int x=0, int y=0);

      virtual ~Block() {}

      void setWidth(int value);
      int width() const { return _width; }

      void setHeight(int value);
      int height() const { return _height; }

      void setX(int value);
      int x() const { return _x; }

      void setY(int value);
      int y() const { return _y; }
      
      void setMarginLeft(int value) { _marginLeft = value; }
      int marginLeft() const { return _marginLeft; }

      void setMarginRight(int value) { _marginRight = value; }
      int marginRight() const { return _marginRight; }

      void setMarginTop(int value) { _marginTop = value; }
      int marginTop() const { return _marginTop; }

      void setMarginBottom(int value) { _marginBottom = value; }
      int marginBottom() const { return _marginBottom; }
      
      virtual QString content() const = 0;

    private:
      int _width;
      int _height;
      int _x;
      int _y;
      
      int _marginLeft;
      int _marginRight;
      int _marginTop;
      int _marginBottom;

    };
}

#endif // BLOCK_H
