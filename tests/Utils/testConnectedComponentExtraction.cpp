#include <catch2/catch_test_macros.hpp>

#include <QImage>
#include <QPainter>

#include "Utils/connectedcomponentextraction.h"
#include "models/doc/doccomponent.h"
#include "models/doc/document.h"

/*
  ConnectedComponentExtraction::getListComponents() binarises an image and
  gives a DocComponent per group of dark pixels, two pixels being connected
  when they are at most 2 apart in x and in y. A group of a single pixel is
  dropped. Each component carries its pixels as scanlines -- for each row,
  pairs of first and last columns -- and a bounding box whose x and y are
  one less than the component's first column and row, and whose width and
  height are last - first + 2: the box [x, x + width) starts one pixel
  before the component and ends on its last column. The source asks
  whether that is right ("//B:wrong ? +1 ?"); this records what it does.
*/

namespace {

  //@return the number of pixels the scanlines of @a c cover
  int
  pixelsOf(const Doc::DocComponent *c)
  {
    int n = 0;
    const Doc::Scanlines &sls = c->getScanlines();
    for (auto it = sls.begin(); it != sls.end(); ++it) {
      const std::vector<int> &pairs = it.value();
      REQUIRE( pairs.size() % 2 == 0 );
      for (size_t i = 0; i < pairs.size(); i += 2) {
        REQUIRE( pairs[i] <= pairs[i + 1] );
        n += pairs[i + 1] - pairs[i] + 1;
      }
    }
    return n;
  }

} //namespace

TEST_CASE( "getListComponents" )
{
  Doc::Document document;

  QImage img(100, 80, QImage::Format_RGB32);
  img.fill(Qt::white);
  {
    QPainter p(&img);
    p.fillRect(10, 10, 10, 10, Qt::black); //x 10..19, y 10..19
    p.fillRect(40, 30, 20, 5, Qt::black);  //x 40..59, y 30..34
  }
  //two pixels with one between them: connected
  img.setPixel(70, 10, qRgb(0, 0, 0));
  img.setPixel(72, 10, qRgb(0, 0, 0));
  //a pixel alone: dropped
  img.setPixel(90, 70, qRgb(0, 0, 0));

  const QList<Doc::DocComponent *> components =
    ConnectedComponentExtraction::getListComponents(img, &document);

  REQUIRE( components.size() == 3 );

  //found in the order of a scan by rows
  const Doc::DocComponent *square = components.at(0);
  const Doc::DocComponent *pair = components.at(1);
  const Doc::DocComponent *bar = components.at(2);

  REQUIRE( pixelsOf(square) == 100 );
  REQUIRE( pixelsOf(pair) == 2 );
  REQUIRE( pixelsOf(bar) == 100 );

  //bounding boxes, cf above
  REQUIRE( square->x() == 9 );
  REQUIRE( square->y() == 9 );
  REQUIRE( square->width() == 11 );
  REQUIRE( square->height() == 11 );
  REQUIRE( bar->x() == 39 );
  REQUIRE( bar->y() == 29 );
  REQUIRE( bar->width() == 21 );
  REQUIRE( bar->height() == 6 );

  //the pair: one row, two runs of one pixel
  REQUIRE( pair->getScanlines().size() == 1 );
  REQUIRE( pair->getScanlines().value(10) == (std::vector<int>{ 70, 70, 72, 72 }) );

  qDeleteAll(components);
}

TEST_CASE( "getListComponents of a blank image" )
{
  Doc::Document document;
  QImage img(30, 20, QImage::Format_RGB32);
  img.fill(Qt::white);
  const QList<Doc::DocComponent *> components =
    ConnectedComponentExtraction::getListComponents(img, &document);
  REQUIRE( components.empty() );
}
