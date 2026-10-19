#include <catch2/catch_test_macros.hpp>

#include <QImage>
#include <QStringList>

#include "Utils/ImageUtils.hpp"

/*
  toGray() has a path per image format: 32 bit, 8 bit gray, 8 bit indexed,
  and pixel() for the others. Each must give the same gray, qGray() of the
  colour, in a 32 bit image of the same size.
*/

namespace {

  const int W = 7; //not a multiple of 4: lines are padded in 8 bit images
  const int H = 5;

  QRgb
  colourAt(int x, int y)
  {
    return qRgb((40 * x) % 256, (60 * y) % 256, (x * y * 17) % 256);
  }

  void
  checkGray(const QImage &in, const QImage &out)
  {
    REQUIRE( out.width() == in.width() );
    REQUIRE( out.height() == in.height() );
    REQUIRE( out.format() == QImage::Format_RGB32 );
    for (int y = 0; y < in.height(); ++y) {
      for (int x = 0; x < in.width(); ++x) {
        const int gray = qGray(in.pixel(x, y));
        INFO( "pixel " << x << "," << y );
        REQUIRE( out.pixel(x, y) == qRgb(gray, gray, gray) );
      }
    }
  }

  QImage
  colourImage(QImage::Format format)
  {
    QImage img(W, H, QImage::Format_RGB32);
    for (int y = 0; y < H; ++y)
      for (int x = 0; x < W; ++x)
        img.setPixel(x, y, colourAt(x, y));
    return img.convertToFormat(format);
  }

} //namespace

TEST_CASE( "toGray gives qGray of each pixel, whatever the format" )
{
  SECTION("32 bit")
  {
    for (QImage::Format f : { QImage::Format_RGB32, QImage::Format_ARGB32 }) {
      const QImage in = colourImage(f);
      checkGray(in, toGray(in));
    }
  }

  SECTION("32 bit, not contiguous")
  {
    //a copy of part of an image keeps its own lines, but a view does not
    const QImage big = colourImage(QImage::Format_RGB32);
    const QImage in(big.constBits(), W - 2, H, big.bytesPerLine(), QImage::Format_RGB32);
    checkGray(in, toGray(in));
  }

  SECTION("8 bit gray")
  {
    const QImage in = colourImage(QImage::Format_Grayscale8);
    checkGray(in, toGray(in));
  }

  SECTION("8 bit indexed")
  {
    const QImage in = colourImage(QImage::Format_Indexed8);
    REQUIRE( in.format() == QImage::Format_Indexed8 );
    checkGray(in, toGray(in));
  }

  SECTION("other formats, through pixel()")
  {
    for (QImage::Format f : { QImage::Format_RGB888, QImage::Format_RGB16, QImage::Format_Mono }) {
      const QImage in = colourImage(f);
      checkGray(in, toGray(in));
    }
  }

  SECTION("an index past the color table is black, as QImage::pixel() has it")
  {
    QImage in(W, H, QImage::Format_Indexed8);
    in.setColorCount(2);
    in.setColor(0, qRgb(255, 255, 255));
    in.setColor(1, qRgb(10, 20, 30));
    in.fill(1);
    in.scanLine(2)[3] = 200; //no such colour
    const QImage out = toGray(in);
    REQUIRE( out.pixel(3, 2) == qRgb(0, 0, 0) );
    const int gray = qGray(qRgb(10, 20, 30));
    REQUIRE( out.pixel(0, 0) == qRgb(gray, gray, gray) );
  }

  SECTION("an empty image")
  {
    const QImage out = toGray(QImage());
    REQUIRE( out.isNull() );
  }
}

TEST_CASE( "image filters" )
{
  const QString read = getReadImageFilter();
  const QString write = getWriteImageFilter();
  const QStringList readList = getReadImageFilterList();
  const QStringList writeList = getWriteImageFilterList();

  //png is built in Qt: it is always there
  REQUIRE( readList.contains(QStringLiteral("*.png")) );
  REQUIRE( writeList.contains(QStringLiteral("*.png")) );
  REQUIRE( read.contains(QStringLiteral("*.png")) );
  REQUIRE( write.contains(QStringLiteral("*.png")) );

  //the filter is the list between parentheses, separated by spaces
  REQUIRE( read.endsWith(QStringLiteral(")")) );
  for (const QString &f : readList)
    REQUIRE( read.contains(f) );
  for (const QString &f : readList)
    REQUIRE( f.startsWith(QStringLiteral("*.")) );
}

/*
  Every 32-bit image was read as QRgb: an RGBA8888 one, of bytes R, G, B,
  A, came out with red and blue swapped.
*/
TEST_CASE( "toGray() of the 32-bit formats" )
{
  const QColor red(255, 0, 0);
  const int expected = qGray(red.rgb());
  for (const QImage::Format format : {QImage::Format_RGB32, QImage::Format_ARGB32,
                                      QImage::Format_RGBA8888, QImage::Format_RGBX8888,
                                      QImage::Format_RGB30}) {
    INFO( "format=" << static_cast<int>(format) );
    QImage img(5, 3, format);
    img.fill(red);
    const QImage gray = toGray(img);
    REQUIRE( gray.size() == img.size() );
    REQUIRE( qRed(gray.pixel(2, 1)) == expected );
    REQUIRE( qBlue(gray.pixel(2, 1)) == expected );
  }
}
