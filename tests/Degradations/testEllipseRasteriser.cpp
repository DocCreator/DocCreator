#include <catch2/catch_test_macros.hpp>

#include <algorithm>

#include <opencv2/imgproc/imgproc.hpp>

#include "Degradations/EllipseRasteriser.hpp"
#include "Degradations/GrayscaleCharsDegradationModel.hpp"

/*
  The rasteriser cuts the elliptical noise region of a seed point into the
  segments that go from its centre to each point of its contour. It used to
  be inlined in the degradation, where:

   - the contour was taken with contours.at(0) without checking that one was
     found, so an ellipse that fell entirely outside the window threw;
   - the number of segments was kept in a uchar, so an ellipse with more
     than 255 contour points wrapped around and lost most of them;
   - the buffer the ellipse is drawn in was allocated on the first call and
     reused, so a later call with a larger window read and wrote past it.
*/

namespace {

  const int ROWS = 200;
  const int COLS = 200;
  const int LOCAL_ZONE = 20;

  cv::Mat
  makeGrayImage()
  {
    cv::Mat gray(ROWS, COLS, CV_8UC1, cv::Scalar(200));
    //a few darker blobs, so that the gray levels at the contour differ
    cv::rectangle(gray, cv::Rect(40, 40, 30, 30), cv::Scalar(40), -1);
    cv::rectangle(gray, cv::Rect(120, 120, 30, 30), cv::Scalar(90), -1);
    return gray;
  }

  struct Result
  {
    size_t numLines = 0;
    cv::Rect rec;
    int maxGray = INT_MIN;
    int minGray = INT_MAX;
    std::vector<dc::NoiseLine> lines;
  };

  Result
  rasterise(dc::EllipseRasteriser &rasteriser,
	    const cv::Mat &gray,
	    cv::Point centre,
	    double semiMinor,
	    double semiMajor,
	    double theta = 0.,
	    int localZone = LOCAL_ZONE)
  {
    Result r;
    r.numLines = rasteriser.rasterise(gray, localZone, r.lines, r.rec,
				      r.maxGray, r.minGray, centre,
				      semiMinor, semiMajor, theta);
    return r;
  }

  //every point of every line must be inside the image
  void
  checkPointsInImage(const Result &r, const cv::Mat &gray)
  {
    for (size_t i = 0; i < r.numLines; ++i) {
      const dc::NoiseLine &line = r.lines[i];
      REQUIRE_FALSE( line.points.empty() );
      for (const cv::Point &p : line.points) {
	REQUIRE( p.x >= 0 );
	REQUIRE( p.y >= 0 );
	REQUIRE( p.x < gray.cols );
	REQUIRE( p.y < gray.rows );
      }
    }
  }

} //namespace

TEST_CASE( "Testing EllipseRasteriser" )
{
  const cv::Mat gray = makeGrayImage();

  SECTION("a plain ellipse gives one line per contour point")
  {
    dc::EllipseRasteriser rasteriser;
    const Result r = rasterise(rasteriser, gray, cv::Point(100, 100), 4, 9);

    REQUIRE( r.numLines > 0 );
    REQUIRE( r.numLines <= r.lines.size() );
    checkPointsInImage(r, gray);

    //each line starts at the centre of the region
    for (size_t i = 0; i < r.numLines; ++i)
      REQUIRE( r.lines[i].points.front() == cv::Point(100, 100) );

    //and no contour point is farther than the major axis
    for (size_t i = 0; i < r.numLines; ++i)
      REQUIRE( r.lines[i].distanceToCentre <= 9.f + 2.f );

    REQUIRE( r.minGray <= r.maxGray );
    REQUIRE( r.rec.width > 0 );
    REQUIRE( r.rec.height > 0 );
  }

  SECTION("degenerate axes do not throw")
  {
    //Null axes draw a single pixel or a one pixel wide segment, and the
    //contour can come back empty for a region that falls outside the
    //window. [The contour was taken with contours.at(0), which threw.]
    dc::EllipseRasteriser rasteriser;
    const double axes[][2] = { { 0, 0 }, { 0, 1 }, { 1, 0 } };
    for (const auto &a : axes) {
      Result r;
      REQUIRE_NOTHROW(
	r = rasterise(rasteriser, gray, cv::Point(100, 100), a[0], a[1]) );
      REQUIRE( r.numLines <= r.lines.size() );
      checkPointsInImage(r, gray);
    }
  }

  SECTION("an ellipse with more than 255 contour points keeps them all")
  {
    //The count was held in a uchar: a contour of n points gave n % 256
    //lines, so a large region lost almost all of its segments.
    dc::EllipseRasteriser rasteriser;
    const Result r =
      rasterise(rasteriser, gray, cv::Point(100, 100), 60, 80, 0., 80);

    REQUIRE( r.numLines > 255 );
    checkPointsInImage(r, gray);
  }

  SECTION("the result does not depend on the calls that came before")
  {
    //The point vectors and the drawing buffer are reused from one seed
    //point to the next: a second identical call must give the same thing.
    dc::EllipseRasteriser rasteriser;
    const Result first = rasterise(rasteriser, gray, cv::Point(60, 60), 5, 11, 30.);
    //something else in between, to dirty the buffers
    rasterise(rasteriser, gray, cv::Point(130, 130), 3, 17, 70.);
    const Result again = rasterise(rasteriser, gray, cv::Point(60, 60), 5, 11, 30.);

    REQUIRE( again.numLines == first.numLines );
    REQUIRE( again.rec == first.rec );
    REQUIRE( again.maxGray == first.maxGray );
    REQUIRE( again.minGray == first.minGray );
    for (size_t i = 0; i < first.numLines; ++i) {
      REQUIRE( again.lines[i].points == first.lines[i].points );
      REQUIRE( again.lines[i].borderGrayLevel == first.lines[i].borderGrayLevel );
    }
  }

  SECTION("a larger window than the first one is handled")
  {
    //The buffer was allocated on the first call only, from its localZone.
    dc::EllipseRasteriser rasteriser;
    rasterise(rasteriser, gray, cv::Point(100, 100), 4, 9, 0., 5);

    Result r;
    REQUIRE_NOTHROW(
      r = rasterise(rasteriser, gray, cv::Point(100, 100), 20, 40, 0., 60) );
    REQUIRE( r.numLines > 0 );
    checkPointsInImage(r, gray);
  }

  SECTION("a centre on the border of the image")
  {
    dc::EllipseRasteriser rasteriser;
    const cv::Point corners[] = { cv::Point(0, 0),
				  cv::Point(COLS - 1, 0),
				  cv::Point(0, ROWS - 1),
				  cv::Point(COLS - 1, ROWS - 1) };
    for (const cv::Point &centre : corners) {
      Result r;
      REQUIRE_NOTHROW( r = rasterise(rasteriser, gray, centre, 6, 14, 45.) );
      checkPointsInImage(r, gray);
    }
  }
}

/*
  The two cases above, reached through the degradation itself: a component
  in the extreme corner of the image, so that its noise regions fall partly
  or wholly outside, and a component large enough for the semi major axis to
  exceed 40 and the contour to exceed 255 points.
*/
TEST_CASE( "Testing CharactersDegradation on extreme components" )
{
  SECTION("a component in each corner of the image")
  {
    for (int imageType : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      cv::Mat img(60, 60, imageType, cv::Scalar(210, 210, 210, 255));
      cv::rectangle(img, cv::Rect(0, 0, 6, 6), cv::Scalar(40, 40, 40, 255), -1);
      cv::rectangle(img, cv::Rect(54, 0, 6, 6), cv::Scalar(40, 40, 40, 255), -1);
      cv::rectangle(img, cv::Rect(0, 54, 6, 6), cv::Scalar(40, 40, 40, 255), -1);
      cv::rectangle(img, cv::Rect(54, 54, 6, 6), cv::Scalar(40, 40, 40, 255), -1);

      for (int level = 1; level <= 10; ++level) {
	for (uint32_t seed = 1; seed <= 4; ++seed) {
	  cv::Mat out;
	  REQUIRE_NOTHROW( out = dc::GrayscaleCharsDegradation::degradation(
			     img, level, .33f, .33f, seed) );
	  REQUIRE( out.type() == imageType );
	  REQUIRE( out.size() == img.size() );
	}
      }
    }
  }

  SECTION("a component whose noise regions have a large contour")
  {
    //The semi major axis grows with the size of the component; above 40 the
    //contour of the region exceeds 255 points.
    for (int imageType : { CV_8UC1, CV_8UC3, CV_8UC4 }) {
      cv::Mat img(900, 900, imageType, cv::Scalar(210, 210, 210, 255));
      cv::rectangle(img,
		    cv::Rect(100, 100, 400, 400),
		    cv::Scalar(50, 50, 50, 255),
		    -1);
      const cv::Mat imgClone = img.clone();

      cv::Mat out;
      REQUIRE_NOTHROW( out = dc::GrayscaleCharsDegradation::degradation(
			 img, 10, .33f, .33f, 7) );
      REQUIRE( out.type() == imageType );
      REQUIRE( out.size() == img.size() );
      REQUIRE( cv::countNonZero(cv::Mat(out != img).reshape(1)) > 0 );

      //and the same seed gives the same image on this size too
      const cv::Mat out2 =
	dc::GrayscaleCharsDegradation::degradation(img, 10, .33f, .33f, 7);
      REQUIRE( cv::countNonZero(cv::Mat(out != out2).reshape(1)) == 0 );
    }
  }
}
