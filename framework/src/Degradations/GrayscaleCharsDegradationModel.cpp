#include "GrayscaleCharsDegradationModel.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <ctime>
#include <limits>
#include <numeric> //accumulate

#include <opencv2/imgproc/imgproc.hpp>

#include "ConnectedComponent.hpp"

namespace dc {

  //#include <iostream>//DEBUG

  /**
   * \mainpage
   We propose a local noise model for grayscale document images.
   The main principle of our model is to locally degrade the image in the
   neighborhoods of “seed-points”, randomly selected. These points define the centers of
   ellipse-shape “noise regions” where the axis of the ellipse are measured by local gradient value.
   A degradation level of each pixel in the noise region is set by a Gaussian random distribution,
   based on its distance towards the center of its noise region. Our model can
   simulate common defects, seen in old documents e.g. splotches, specks and streaks.
   *
   */

  static constexpr int NO_CC = -1;
  //B:TODO: Do we really need this constant ???

#if 0

#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
  //#include <boost/algorithm/string.hpp>

  using MyRNG = boost::mt19937;

  struct NormalDistribution
  {
    NormalDistribution(MyRNG &rng, double mean, double sigma)
      : m_distribution(mean, sigma),
	m_generator(rng, m_distribution)
    {}

    double operator()() { return m_generator(); }

    boost::normal_distribution<> m_distribution;
    boost::variate_generator<MyRNG&, boost::normal_distribution<> > m_generator;
  };

  struct UniformDistribution
  {
    UniformDistribution(MyRNG &rng, double a, double b)
      : m_distribution(a, b),
	m_generator(rng, m_distribution)
    {}

    double operator()() { return m_generator(); }

    boost::uniform_real<> m_distribution;
    boost::variate_generator<MyRNG&, boost::uniform_real<> > m_generator;
  };

#else

  using MyRNG = cv::RNG;

  struct NormalDistribution
  {
    NormalDistribution(MyRNG &rng, double mean, double sigma)
      : m_rng(rng)
      , m_mean(mean)
      , m_sigma(sigma)
    {}

    double operator()() { return m_rng.gaussian(m_sigma) + m_mean; }

    MyRNG m_rng;
    double m_mean;
    double m_sigma;
  };

  struct UniformDistribution
  {
    UniformDistribution(MyRNG &rng, double a, double b)
      : m_rng(rng)
      , m_a(a)
      , m_b(b)
    {}

    double operator()() { return m_rng.uniform(m_a, m_b); }

    MyRNG m_rng;
    double m_a;
    double m_b;
  };

#endif

  //#define TIMING 1
#ifdef TIMING
#include <chrono>
#endif


  GrayscaleCharsDegradationModel::GrayscaleCharsDegradationModel(
								 const cv::Mat &img)
    : _width(0)
    , _height(0)
    , _avgBackground(0)
    , _avgForeground(0)
    , _nb_connectedcomponants(0)
    , _MAX_a0(0)
    , _alpha(0.0)
    , _beta(0.0)
    , _alpha0(0.0)
    , _beta0(0.0)
    , _nf(0.0)
    , _nb(0.0)
    , _a0(0.0f)
    , _g(0.0f)
    , _MAX_Gradient(0.0f)
    , _AVG_Gradient(0.0f)
    , _sigma_gausien(0.0f)
    , _is4connected(false)
    , _Mean_CC_Distance(0.0f)
    , _Max_CC_Distance(0.0f)
    , _Mean_CC_Stroke(0.0f)
    , _Max_CC_Stroke(0.0f)
    , _Total_Degradation(0.0f)
    , _nbSPs_User(0)
    , _local_zone(0)
  {
    initialize(img);
  }

  GrayscaleCharsDegradationModel::~GrayscaleCharsDegradationModel()
  {
    _listSeedPoints.clear();

    _mat_output.release();
    _mat_gray.release();
    _mat_binary.release();
    _mat_CCs.release();
  }

  void
  GrayscaleCharsDegradationModel::initialize(const cv::Mat &imgInput)
  {
    _MAX_Gradient = INT_MIN;
    _AVG_Gradient = 0;

    _MAX_a0 = 20;
    _Mean_CC_Stroke = 0;
    _Max_CC_Stroke = INT_MIN;

    _Total_Degradation = 0.0;

    _is4connected = true;
    _sigma_gausien = 20;
    _a0 = 4;
    _g = 0.6f;
    _alpha0 = 1;
    _beta0 = 1;
    _nbSPs_User = 0;
    _nf = 0;
    _nb = 0;

    _local_zone = 20;

    assert(imgInput.data != nullptr);

    //TODO:Apply degradation on color images
    _inputType = imgInput.type();
    if (imgInput.channels() == 4) {
      cvtColor(imgInput, _mat_gray, cv::COLOR_BGRA2GRAY);
    }
    else if (imgInput.channels() == 3) {
      cvtColor(imgInput, _mat_gray, cv::COLOR_BGR2GRAY);
    }
    else if (imgInput.channels() == 1) {
      _mat_gray = imgInput.clone();
    }
    else {
      _nb_connectedcomponants = NO_CC;
      _mat_output = imgInput.clone(); //need same type as input
      assert(_mat_output.type() == _inputType);
      return;
    }
    assert(_mat_gray.type() == CV_8UC1);


    _width = _mat_gray.cols;
    _height = _mat_gray.rows;

    _mat_binary = binarize(); //binarize _mat_gray
    //set _nb_connectedcomponants to NO_CC if color-uniform image,
    // but CCs are not yet computed.

    if (_nb_connectedcomponants != NO_CC) {

      _mat_output = _mat_gray.clone();

#ifdef TIMING
      auto t0 = std::chrono::steady_clock::now();
#endif

      // get connected componants information
      extractConnectedComponantInfos();

#ifdef TIMING
      auto t1 = std::chrono::steady_clock::now();
#endif

      // calculate the transform distance of pixels.
      calculatePixelsDistanceTransform();

#ifdef TIMING
      auto t2 = std::chrono::steady_clock::now();
      auto time1 = t1 - t0;
      auto time2 = t2 - t1;
      std::cerr << "Time extractConnectedComponantInfos="
		<< std::chrono::duration<double, std::milli>(time1).count()
		<< "ms for _nb_connectedcomponants=" << _nb_connectedcomponants
		<< "\n";
      std::cerr << "Time calculatePixelsDistanceTransform="
		<< std::chrono::duration<double, std::milli>(time2).count()
		<< "ms\n";
      std::cerr << "_listPixels.size()=" << _listPixels.size() << "\n";
#endif
    }
    else {
      _mat_output = imgInput.clone(); //to have same type as input
    }
  }

  float
  GrayscaleCharsDegradationModel::getAvgStrokeWidth() const
  {
    return _Mean_CC_Stroke;
  }

  int
  GrayscaleCharsDegradationModel::getNumberOfConnectedComponants() const
  {
    return _nb_connectedcomponants;
  }

  cv::Mat
  GrayscaleCharsDegradationModel::getImageDegraded_cv()
  {
    return _mat_output;
  }

  //B: is this method useful ?
  cv::Mat
  GrayscaleCharsDegradationModel::getImageGray_cv()
  {
    return _mat_gray;
  }

  cv::Mat
  GrayscaleCharsDegradationModel::degradateByLevel_cv(int level)
  {
    cv::Mat src;
#if 1 //DEBUG
    if (level <= 4) {
      src = degradate_cv(level, 50, 30, 20);
    } else if (level <= 7) {
      src = degradate_cv(level, 30, 50, 20);
    } else {
      src = degradate_cv(level, 20, 30, 50);
    }
#else
    src = degradate_cv(level, 100, 0, 0);
#endif //0

    return src;
  }

  /*
  //DEBUG
  time calculatePixelsProbability= 4.526 ms
  time flipPixelsByProbability= 14.859 ms
  time calculateNoiseRegionType= 55.644 ms
  time separateSeedPointsByTypes= 0.122 ms
  time assignmentSizeOfNoiseRegion= 0.007 ms
  time grayscaleDegradationByTypes= 47.973 ms

  /RELEASE (different run)
  time calculatePixelsProbability= 3.548 ms
  time flipPixelsByProbability= 3.741 ms
  time calculateNoiseRegionType= 99.001 ms
  time separateSeedPointsByTypes= 0.055 ms
  time assignmentSizeOfNoiseRegion= 0.003 ms
  time grayscaleDegradationByTypes= 59.495 ms


  11/04/2015 sur testBoris_BIG2.xml
  gcc 4.8.3 en RelWithDebInfo

  Time extractConnectedComponantInfos=38.76ms
  Time calculatePixelsDistanceTransform=241.519ms
  _listPixels.size()=1325618
  ##### GrayscaleCharsDegradationModel::degradate( 10 ,  20 ,  30 ,  50 )
  getNumberOfConnectedComponants()= 2224
  _nbSPs_User= 8896
  dbg1=9543 dbg2=821 dbg3=311 _listSeedPoints.size()=10675
  time calculatePixelsProbability= 52.451 ms
  time flipPixelsByProbability= 64.981 ms
  time calculateNoiseRegionType= 70.009 ms
  time separateSeedPointsByTypes= 0.817 ms
  time assignmentSizeOfNoiseRegion= 0.07 ms
  time grayscaleDegradationByTypes= 1899.45 ms


  */


  cv::Mat
  GrayscaleCharsDegradationModel::degradate_cv(int level,
					       float I,
					       float O,
					       float D)
  {
    if (_nb_connectedcomponants > 0) {

      /*
	qDebug() << "##### GrayscaleCharsDegradationModel::degradate(" << level
	<< ", " << I << ", " << O << ", " << D << ")";
      */

      constexpr int NUMBER_NOISE_PER_CC = 2; // the number of noise per ccs

      _nbSPs_User =
	NUMBER_NOISE_PER_CC * getNumberOfConnectedComponants() * level / 5;
      /*
	qDebug() << "getNumberOfConnectedComponants()="
	<< getNumberOfConnectedComponants();
	qDebug() << "_nbSPs_User=" << _nbSPs_User;
      */
#ifdef TIMING
      auto t0 = std::chrono::steady_clock::now();
#endif

      calculatePixelsProbability();

#ifdef TIMING
      auto t1 = std::chrono::steady_clock::now();
#endif

      flipPixelsByProbability(I);

#ifdef TIMING
      auto t2 = std::chrono::steady_clock::now();
#endif

      //qDebug() << "seed point classification: get type "<< _MAX_Gradient << " " << _listSeedPoints.size() ;

      calculateNoiseRegionType();

#ifdef TIMING
      auto t3 = std::chrono::steady_clock::now();
#endif

      //qDebug() << "seed point classification: separate by type " ;

      separateSeedPointsByTypes(O, D);

#ifdef TIMING
      auto t4 = std::chrono::steady_clock::now();
#endif

      //qDebug() << "seed point classification: asigne size of ellipses";

      assignmentSizeOfNoiseRegion();

#ifdef TIMING
      auto t5 = std::chrono::steady_clock::now();
#endif

      grayscaleDegradationByTypes();

#ifdef TIMING
      auto t6 = std::chrono::steady_clock::now();
#endif


#ifdef TIMING
      {
	auto time1 = t1 - t0;
	auto time2 = t2 - t1;
	auto time3 = t3 - t2;
	auto time4 = t4 - t3;
	auto time5 = t5 - t4;
	auto time6 = t6 - t5;
	std::cerr << "time calculatePixelsProbability="
		  << std::chrono::duration<double, std::milli>(time1).count()
		  << "ms\n";
	std::cerr << "time flipPixelsByProbability="
		  << std::chrono::duration<double, std::milli>(time2).count()
		  << "ms\n";
	std::cerr << "time calculateNoiseRegionType="
		  << std::chrono::duration<double, std::milli>(time3).count()
		  << "ms\n";
	std::cerr << "time separateSeedPointsByTypes="
		  << std::chrono::duration<double, std::milli>(time4).count()
		  << "ms\n";
	std::cerr << "time assignmentSizeOfNoiseRegion="
		  << std::chrono::duration<double, std::milli>(time5).count()
		  << "ms\n";
	std::cerr << "time grayscaleDegradationByTypes="
		  << std::chrono::duration<double, std::milli>(time6).count()
		  << "ms\n";
      }
#endif

    }

    //TODO: not needed if degradation worked on color image
    assert(_inputType == CV_8UC1 ||
	   _inputType == CV_8UC3 ||
	   _inputType == CV_8UC4);
    if (_mat_output.type() != _inputType) {
      if (_inputType == CV_8UC4) {
	cv::Mat out;
	cvtColor(_mat_output, out, cv::COLOR_GRAY2BGRA);
	_mat_output = out;
      }
      else if (_inputType == CV_8UC3) {
	cv::Mat out;
	cvtColor(_mat_output, out, cv::COLOR_GRAY2BGR);
	_mat_output = out;
      }
    }
    assert(_mat_output.type() == _inputType);

    return _mat_output;
  }

  /*
    Fill _listPixels, _MAX_Gradient, _AVG_Gradient

  */
  void
  GrayscaleCharsDegradationModel::calculatePixelsDistanceTransform()
  {
    assert(_mat_reduce_pixels.type() == CV_8UC1);
    assert(_mat_reduce_pixels.rows == _height &&
	   _mat_reduce_pixels.cols == _width);
    assert(_mat_binary.type() == CV_8UC1);
    assert(_mat_binary.rows == _height && _mat_binary.cols == _width);

    _listPixels.reserve((_width * _height) / 5); //arbitrary !
    //We should compute the number of white pixels in _mat_reduce_pixels
    // to have a less arbitrary capacity.

    //B:TODO:OPTIM? better way to compute gradient image.

    const int height = _mat_reduce_pixels.rows;
    const int width = _mat_reduce_pixels.cols;

    //size_t nbPixs = 0;

    for (int y = 0; y < height; ++y) {
      const uchar *m = _mat_reduce_pixels.ptr<uchar>(y);
      const uchar *b = _mat_binary.ptr<uchar>(y);
      for (int x = 0; x < width; ++x) {
	if (m[x] == 255) {

	  //++nbPixs;

	  Pixel pixel(x, y);
	  pixel.isBackground = (b[x] != 0);

	  // Compute approximately the transformation distance
	  pixel.distanceToEdge = calculateApproximatelyDistanceFromBord(pixel);
	  //pixel.distanceToEdge = calculateDistanceFromBord(pixel, _is4connected);

	  if (pixel.distanceToEdge > 0) {

	    //B: small change from Cuong's original code.
	    //Pixels on contour (with pixel.distanceToEdge==0) are not taken into account
	    // for _Max_Gradient & _AVG_Gradient computation.

	    // compute gradient for gray image
	    if (pixel.pos.x > 0 && pixel.pos.x < (width - 1) && pixel.pos.y > 0 &&
		pixel.pos.y < (height - 1)) {

	      const float dx =
		((int)_mat_gray.at<uchar>(pixel.pos.y, pixel.pos.x + 1) -
		 (int)_mat_gray.at<uchar>(pixel.pos.y, pixel.pos.x - 1)) *
		0.5f;
	      const float dy =
		((int)_mat_gray.at<uchar>(pixel.pos.y + 1, pixel.pos.x) -
		 (int)_mat_gray.at<uchar>(pixel.pos.y - 1, pixel.pos.x)) *
		0.5f;

	      pixel.gradient_value = sqrtf(dx * dx + dy * dy);
	      pixel.gradient_angle = atan2f(dy, dx) * 180 / CV_PI;

	    } else {
	      pixel.gradient_value = 0;
	      pixel.gradient_angle = 0;
	    }

	    if (pixel.gradient_angle == 0) {
	      pixel.gradient_angle = 10; //B: why this value ???
	    }

	    // MAX gradient value
	    if (_MAX_Gradient < pixel.gradient_value) {
	      _MAX_Gradient = pixel.gradient_value;
	    }

	    // AVG gradient value
	    _AVG_Gradient += pixel.gradient_value;

	    _listPixels.push_back(pixel);
	  }
	}
      }
    }

    if (!_listPixels.empty())
      _AVG_Gradient = _AVG_Gradient / _listPixels.size();

    _mat_reduce_pixels.release();
  }

  /*
    int GrayscaleCharsDegradationModel::getTypeNoiseRegion(const std::vector<cv::Point> &listElipseEdgePoints)
    {
    assert(! listElipseEdgePoints.empty()); //B

    int type = 0;
    for (size_t i = 1; i<listElipseEdgePoints.size(); ++i) { //start from 1
    const cv::Point p = listElipseEdgePoints[i-1];
    const cv::Point p1 = listElipseEdgePoints[i];
    if (_mat_binary.at<uchar>(p.y, p.x) != _mat_binary.at<uchar>(p1.y, p1.x))
    ++type;
    }

    const cv::Point p = listElipseEdgePoints[listElipseEdgePoints.size()-1];
    const cv::Point p1 = listElipseEdgePoints[0];
    if (_mat_binary.at<uchar>(p.y, p.x) != _mat_binary.at<uchar>(p1.y, p1.x))
    ++type;

    return type;
    }
  */

  int
  GrayscaleCharsDegradationModel::getMinStrokeWidthAtASeedPoint(
								const Seedpoint &sp,
								int &angle,
								cv::Point &A,
								cv::Point &B) const
  {
    //B: some code duplication with calculateApproximatelyDistanceFromBord() ???

    assert(_mat_CCs.rows == _height && _mat_CCs.cols == _width &&
	   _mat_CCs.type() == CV_8UC1);

    int min_stroke_width_at_seed_point = 0;

    const int width = _mat_CCs.cols;
    const int height = _mat_CCs.rows;

    const int xo = sp.pixel.pos.x;
    const int yo = sp.pixel.pos.y;

    // 0 degree
    int d_0 = 0;
    cv::Point A_0, B_0;
    {
      const uchar *m = _mat_CCs.ptr<uchar>(yo);
      for (int x = xo; x < width; ++x) {
	if (m[x] == 0) {
	  ++d_0;
	  A_0.x = x;
	  A_0.y = yo;
	}
	else {
	  break;
	}
      }
      for (int x = xo; x >= 0; --x) {
	if (m[x] == 0) {
	  ++d_0;
	  B_0.x = x;
	  B_0.y = yo;
	}
	else {
	  break;
	}
      }
    }
    // 90 degree
    int d_90 = 0;
    cv::Point A_90, B_90;
    for (int y = yo; y < height; ++y) {
      assert(y >= 0 && y < _mat_CCs.rows && xo >= 0 && xo < _mat_CCs.cols);
      if (_mat_CCs.at<uchar>(y, xo) == 0) {
	++d_90;
	A_90.x = xo;
	A_90.y = y;
      } else
	break;
    }
    for (int y = yo; y >= 0; --y) {
      assert(y >= 0 && y < _mat_CCs.rows && xo >= 0 && xo < _mat_CCs.cols);
      if (_mat_CCs.at<uchar>(y, xo) == 0) {
	++d_90;
	B_90.x = xo;
	B_90.y = y;
      } else
	break;
    }
    // 45 degree
    int d_45 = 0;
    cv::Point A_45, B_45;
    int y = yo;
    for (int x = xo; x < width; ++x) {
      if (y < 0) {
	break;
      }
      assert(y >= 0 && y < _mat_CCs.rows && x >= 0 && x < _mat_CCs.cols);
      if (_mat_CCs.at<uchar>(y, x) == 0) {
	++d_45;
	A_45.x = x;
	A_45.y = y;
      }
      else {
	break;
      }
      --y;
    }
    y = yo;
    for (int x = xo; x >= 0; --x) {
      if (y >= height) {
	break;
      }
      assert(y >= 0 && y < _mat_CCs.rows && x >= 0 && x < _mat_CCs.cols);
      if (_mat_CCs.at<uchar>(y, x) == 0) {
	++d_45;
	B_45.x = x;
	B_45.y = y;
      }
      else {
	break;
      }
      ++y;
    }
    //135 degree
    int d_135 = 0;
    cv::Point A_135, B_135;
    y = yo;
    for (int x = xo; x < width; ++x) {
      if (y >= height) {
	break;
      }
      assert(y >= 0 && y < _mat_CCs.rows && x >= 0 && x < _mat_CCs.cols);
      if (_mat_CCs.at<uchar>(y, x) == 0) {
	++d_135;
	A_135.x = x;
	A_135.y = y;
      }
      else {
	break;
      }
      ++y;
    }
    y = yo;
    for (int x = xo; x >= 0; --x) {
      if (y < 0) {
	break;
      }
      assert(y >= 0 && y < _mat_CCs.rows && x >= 0 && x < _mat_CCs.cols);
      if (_mat_CCs.at<uchar>(y, x) == 0) {
	++d_135;
	B_135.x = x;
	B_135.y = y;
      }
      else {
	break;
      }
      --y;
    }
    --d_0;
    --d_90;
    --d_45;
    --d_135;

    min_stroke_width_at_seed_point = d_0;
    angle = 0;
    A.x = A_0.x;
    A.y = A_0.y;
    B.x = B_0.x;
    B.y = B_0.y;
    if (d_90 < min_stroke_width_at_seed_point) {
      min_stroke_width_at_seed_point = d_90;
      angle = 90;
      A.x = A_90.x;
      A.y = A_90.y;
      B.x = B_90.x;
      B.y = B_90.y;
    }
    if (d_45 < min_stroke_width_at_seed_point) {
      min_stroke_width_at_seed_point = d_45;
      angle = -45;
      A.x = A_45.x;
      A.y = A_45.y;
      B.x = B_45.x;
      B.y = B_45.y;
    }
    if (d_135 < min_stroke_width_at_seed_point) {
      min_stroke_width_at_seed_point = d_135;
      angle = 45;
      A.x = A_135.x;
      A.y = A_135.y;
      B.x = B_135.x;
      B.y = B_135.y;
    }

    return min_stroke_width_at_seed_point;
  }

  /*
    static
    std::vector<cv::Point> getLinePoints(cv::Point A, cv::Point B, int width, int height)
    {
    //B:??? this is to get discrete (Bresenham's) line points ???

    //TODO: OPTIM: we should not have to allocate a temporary image !!!!!!!!!!!

    std::vector<cv::Point> vectorPoints;

    int x_min = A.x;
    int x_max = B.x;
    int y_min = A.y;
    int y_max = B.y;
    if (x_min > x_max)
    std::swap(x_min, x_max);
    if (y_min > y_max)
    std::swap(y_min, y_max);

    x_min = 0;
    if (x_max >= width)
    x_max = width-1;
    y_min = 0;
    if (y_max >= height)
    y_max = height-1;

    if (x_min>=x_max || y_min>=y_max)
    return vectorPoints;

    cv::Mat box = cv::Mat::zeros(cv::Size((x_max-x_min+1), (y_max-y_min+1)), CV_8UC1);

    cv::line(box, A, B, cv::Scalar(WHITE, WHITE, WHITE));

    for (int y=y_min; y<=y_max; ++y)
    for (int x=x_min; x<=x_max; ++x)
    if (box.at<uchar>(y, x) == WHITE)
    vectorPoints.push_back(cv::Point(x, y));

    return vectorPoints;
    }
  */

  //CODE inspired from OpenCV cv::LineIterator.
  //But no need to create a temporary cv::Mat.

  static std::vector<cv::Point>
  getLinePoints(cv::Point pt1, cv::Point pt2, int width, int height)
  {
    std::vector<cv::Point> pts;

    if (static_cast<unsigned>(pt1.x) >= static_cast<unsigned>(width) ||
	static_cast<unsigned>(pt2.x) >= static_cast<unsigned>(width) ||
	static_cast<unsigned>(pt1.y) >= static_cast<unsigned>(height) ||
	static_cast<unsigned>(pt2.y) >= static_cast<unsigned>(height)) {

      const bool inside = cv::clipLine(cv::Size(width, height), pt1, pt2);
      if (!inside)
	return pts;
    }

    int bt_pix = 1;
    constexpr int bt_pix0 = 1;
    int istep = width;

    int dx = pt2.x - pt1.x;
    int dy = pt2.y - pt1.y;
    int s = dx < 0 ? -1 : 0;

    dx = (dx ^ s) - s;
    dy = (dy ^ s) - s;
    pt1.x ^= (pt1.x ^ pt2.x) & s;
    pt1.y ^= (pt1.y ^ pt2.y) & s;

    int ptr = (pt1.y * istep + pt1.x * bt_pix0);

    s = dy < 0 ? -1 : 0;
    dy = (dy ^ s) - s;
    istep = (istep ^ s) - s;

    s = dy > dx ? -1 : 0;

    dx ^= dy & s;
    dy ^= dx & s;
    dx ^= dy & s;

    //swap if s==-1
    bt_pix ^= istep & s;
    istep ^= bt_pix & s;
    bt_pix ^= istep & s;

    assert(dx >= 0 && dy >= 0);

    int err = dx - (dy + dy);
    const int plusDelta = dx + dx;
    const int minusDelta = -(dy + dy);
    const int plusStep = istep;
    const int minusStep = bt_pix;
    const int count = dx + 1;

    pts.reserve(count);

    cv::Point p;
    p.x = pt1.x;
    p.y = pt1.y;

    constexpr int ptr0 = 0;
    const int step = width;

    for (int i = 0; i < count; ++i) {

      pts.push_back(p);

      const int mask = err < 0 ? -1 : 0;
      err += minusDelta + (plusDelta & mask);

      ptr += minusStep + (plusStep & mask);
      p.y = static_cast<int>((ptr - ptr0) / step);
      p.x = static_cast<int>((ptr - ptr0) - p.y * step);
    }

    return pts;
  }

  void
  GrayscaleCharsDegradationModel::calculateNoiseRegionType()
  {
    size_t dbg1 = 0, dbg2 = 0, dbg3 = 0;

    const int localZone = _local_zone;

    for (Seedpoint &sp : _listSeedPoints) {

      cv::Point pB1, pB2;

      int xo = sp.pixel.pos.x;
      int yo = sp.pixel.pos.y;

      const int ax = std::max(xo - localZone, 0);
      const int bx = std::min(xo + localZone, _width - 1);
      const int cy = std::min(yo + localZone, _height - 1);
      const int dy = std::max(yo - localZone, 0);

      std::vector<cv::Point> listPointsInLine;

      const float gradient_angle = sp.pixel.gradient_angle;
      if (gradient_angle != 0 && gradient_angle != 180 && gradient_angle != 90) {

	//B: gradient_angle==0 can not happen cf calculatePixelsDistanceTransform() !

	cv::Point p[4];

	const float k = -tanf(sp.pixel.gradient_angle * CV_PI / 180.0);

	//B:TODO:OPTIM? we have computed sp.pixel.gradient_angle with atan2() & now we apply tan() !?!

	p[0].x = ax;
	p[0].y = k * (p[0].x - xo) + yo;

	p[1].y = dy;
	p[1].x = (p[1].y - yo) / k + xo;

	p[2].x = bx;
	p[2].y = k * (p[2].x - xo) + yo;

	p[3].y = cy;
	p[3].x = (p[3].y - yo) / k + xo;

	std::vector<cv::Point> listOut;
	listOut.reserve(4); //B
	for (int i = 0; i < 4; ++i) {
	  if (p[i].x >= ax && p[i].x <= bx && p[i].y >= dy && p[i].y <= cy)
	    listOut.push_back(p[i]);
	}

	assert(listOut.size() >= 2);

	pB1 = listOut[0];
	pB2 = listOut[1];

	if (listOut.size() > 2 && pB1.x == pB2.x && pB1.y == pB2.y) {
	  pB2 = listOut[2];
	}

	++dbg1;

	listPointsInLine = getLinePoints(pB1, pB2, _width, _height);
      } else if (gradient_angle == 0 || gradient_angle == 180) { //tan =0

	pB1.x = ax;
	pB1.y = yo;

	pB2.x = bx;
	pB2.y = yo;

	const int nbPts = bx - ax + 1;
	listPointsInLine.reserve(nbPts);
	for (int x = ax; x <= bx; ++x)
	  listPointsInLine.emplace_back(x, yo);

	++dbg2;
      } else if (gradient_angle == 90) { // tan = infinity

	pB1.x = xo;
	pB1.y = dy;

	pB2.x = xo;
	pB2.y = cy;

	const int nbPts = cy - dy + 1;
	listPointsInLine.reserve(nbPts);
	for (int y = dy; y <= cy; ++y)
	  listPointsInLine.emplace_back(xo, y);

	++dbg3;
      }

      //B:TODO:OPTIM: could we avoid to call cv::Mat::at() ?? Are we on the same row ? p.y == p1.y ?

      assert(!listPointsInLine.empty());
      const size_t sz = listPointsInLine.size();
      std::vector<cv::Point> listEdgePoints;
      listEdgePoints.reserve(sz);
      for (size_t i = 1; i < sz; ++i) { //start from 1
	const cv::Point p = listPointsInLine[i - 1];
	const cv::Point p1 = listPointsInLine[i];

	const uchar vp = _mat_CCs.at<uchar>(p.y, p.x);
	const uchar vp1 = _mat_CCs.at<uchar>(p1.y, p1.x);

	if (vp != vp1) {
	  if (vp == 0) {
	    listEdgePoints.push_back(p);
	  } else {
	    assert(vp1 == 0);
	    listEdgePoints.push_back(p1);
	  }
	}
      }
      const cv::Point p = listPointsInLine[sz - 1];
      const cv::Point p1 = listPointsInLine[0];
      const uchar vp = _mat_CCs.at<uchar>(p.y, p.x);
      const uchar vp1 = _mat_CCs.at<uchar>(p1.y, p1.x);
      if (vp != vp1) {
	if (vp == 0) {
	  listEdgePoints.push_back(p);
	} else {
	  assert(vp1 == 0);
	  listEdgePoints.push_back(p1);
	}
      }

      listPointsInLine.clear();

      //=========DEBUG================
      //        if (sp.pixel.isBackground)
      //            ;//cv::line(_mat_color, pB1, pB2, cv::Scalar(0, 255, 0));
      //        else cv::line(_mat_color, pB1, pB2, cv::Scalar(0, 0, 255));

      float first_min = std::numeric_limits<float>::max();
      float second_min = std::numeric_limits<float>::max();
      const float dis_max = localZone * sqrtf(2);
      {
	for (const cv::Point pt : listEdgePoints) { //(copy of Point)

	  const float dis =
	    sqrtf((xo - pt.x) * (xo - pt.x) +
		 (yo - pt.y) * (yo - pt.y)); //B:TODO:OPTIM: avoid sqrt
	  if (first_min > dis) {
	    second_min = first_min;
	    first_min = dis;
	  }
	  else {
	    if (second_min > dis)
	      second_min = dis;
	  }
	}
      }
      listEdgePoints.clear();

      if (!sp.pixel.isBackground) {

	int angle = 0;
	cv::Point A, B;
	const int d_stroke_width_min = getMinStrokeWidthAtASeedPoint(sp, angle, A, B);
	if (second_min > d_stroke_width_min)
	  second_min = d_stroke_width_min;

	//=========DEBUG================
	//cv::line(_mat_color, A, B, cv::Scalar(255, 0, 0));

	sp.pixel.gradient_angle = angle + rand() % 15 - 7.5; //B ???
      }

      sp.b_tache = calculateDistanceFromBord(sp.pixel);

      if (sp.b_tache > first_min)
	sp.b_tache = first_min;
      sp.b_cheval = second_min;

      if (first_min < dis_max && second_min < dis_max) {
	sp.type = 5;
      } else if (first_min < dis_max && second_min > dis_max) {
	sp.type = 4;
      } else {
	sp.type = 3;
      }

      //=========DEBUG================
      //Point3_<uchar>* rgb = _mat_color.ptr<Point3_<uchar> >(sp.pixel.pos.y, sp.pixel.pos.x);
      //rgb->x=255;rgb->y=0;rgb->z=0;
    }

    //std::cerr<<"dbg1="<<dbg1<<" dbg2="<<dbg2<<" dbg3="<<dbg3<<" _listSeedPoints.size()="<<_listSeedPoints.size()<<"\n";
  }

  void
  GrayscaleCharsDegradationModel::assignmentSizeOfNoiseRegion()
  {

    for (Seedpoint &sp : _listSeedPoints) {

      if (sp.type == 2) { // Disconnection spots

	constexpr float a03 = 1;
	sp.size = sp.b_cheval + a03;

	if (sp.size <= 1.f)
	  sp.size = 1.7f;

      } else if (sp.type == 1) { // Overlapping spots

	float delta_a = fabs(sp.b_cheval - sp.b_tache);
	if (sp.b_cheval > _Mean_CC_Stroke)
	  delta_a = _Mean_CC_Stroke - sp.b_tache;

	float randValue = 0.15f;
	if (delta_a <= 3.f)
	  randValue = 0.65f;
	else if (delta_a <= 5.f)
	  randValue = 0.3f;

	sp.size = sp.b_tache + randValue * delta_a;

	if (sp.size <= 1.f)
	  sp.size = 1.7f;
      } else if (sp.type == 0) { // Independent spots

	float randValue = 0.4f;
	if (sp.b_tache <= 3.f)
	  randValue = 0.8f;
	else if (sp.b_tache <= 5.f)
	  randValue = 0.75f;
	else if (sp.b_tache <= 10.f)
	  randValue = 0.55f;
	else
	  randValue = 0.4f * _Mean_CC_Stroke / sp.b_tache;

	sp.size = randValue * sp.b_tache;

	if (sp.size <= 1.f)
	  sp.size = 1.5f;
	else if (sp.size < 2.f)
	  sp.size += 0.5f;
      }
    }
  }

  struct PtSorter_X_Y
  {
    bool operator()(cv::Point p1, cv::Point p2) const noexcept
    {
      return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
    }
  };

  struct PtSorter_Y_X
  {
    bool operator()(cv::Point p1, cv::Point p2) const noexcept
    {
      return p1.y < p2.y || (p1.y == p2.y && p1.x < p2.x);
    }
  };

  /*
    Extract Connected Components and keep only those with a 'correct' size.

    Fill _mat_reduce_pixels, _mat_CCs, _mat_contour && _Mean_CC_Stroke.
    Update _nb_connectedcomponants
  */
  void
  GrayscaleCharsDegradationModel::extractConnectedComponantInfos()
  {
    assert(_mat_binary.type() == CV_8UC1);

    CCs ccsInfo;
    constexpr int connectivity = 4;
    ConnectedComponent::extractAllConnectedComponents(
						      _mat_binary, ccsInfo, connectivity);

    _mat_reduce_pixels = cv::Mat::zeros(_mat_binary.size(), _mat_binary.type());
    _mat_CCs = cv::Mat(_mat_binary.size(), _mat_binary.type());

    cv::Mat mat_Accepted_CCs =
      cv::Mat::zeros(_mat_binary.size(), _mat_binary.type());

    //set _mat_CCs to all white (255).
    int rows = _mat_CCs.rows;
    int cols = _mat_CCs.cols;
    if (_mat_CCs.isContinuous()) {
      cols *= rows;
      rows = 1;
    }
    for (int y = 0; y < rows; ++y) {
      uchar *m = _mat_CCs.ptr<uchar>(y);
      for (int x = 0; x < cols; ++x) {
	m[x] = 255;
      }
    }

    int id_cc = 0;

    constexpr int minCCSize = 20;
    int max_dis = std::max(_width, _height) / 35;
    if (_width * _height >= 1000000)
      max_dis = std::max(_width, _height) / 15;
    //B:TODO: This does not work if we have a small image and a big text.
#if 0
    {
      size_t m0 = 0;
      double m1 = 0.0, m2 = 0.0;
      const size_t minCCSizeForMedian = std::min(3, minCCSize); //to remove noise
      for (const CC &ccItem : ccsInfo) {
	const size_t sz = ccItem.size();
	if (sz > minCCSizeForMedian) {
	  ++m0;
	  m1 += sz;
	  m2 += sz * sz;
	}
      }
      if (m0 > 0) {
	const double inv_m0 = 1. / m0;
	const double mean = m1 * inv_m0;
	const double stdDev = sqrt((m2 - m1 * m1 * inv_m0) * inv_m0);
	int max_disB = static_cast<int>((mean + stdDev) * 3.34);
	//B:UGLY: we estimate area of AABB from CC size...

	max_dis = std::max(max_dis, max_disB);
      }
    }
#else
    {
      /*
	//B:24/09/2019
	Current code produces ugly results on "big" characters.
	Thus we do not want to select too "big" characters.

	This code should give acceptable results if we have a majority of
	small characters (and potentially some "big" characters,
        for the titles for example).
	We do not set an absolute maximum for now. Thus if we only have
        "big" characters on the image, the result will still be ugly...
       */

      const size_t sz = ccsInfo.size();
      std::vector<size_t> ccsSize(sz);
      for (size_t i=0; i<sz; ++i) {
        ccsSize[i] = ccsInfo[i].size();
      }
      const auto itMed = ccsSize.begin()+sz/2;
      std::nth_element(ccsSize.begin(), itMed, ccsSize.end());
      //const auto itMax = std::max_element(ccsSize.begin(), ccsSize.end());
      //const auto itMin = std::min_element(ccsSize.begin(), ccsSize.end());
      auto mean = std::accumulate(std::begin(ccsSize), std::end(ccsSize), 0.0) / ccsSize.size();
      //std::cerr<<"min="<<*itMin<<" median="<<*itMed<<" max="<<*itMax<<"  mean="<<mean<<"\n";

      //std::cerr<<"max_dis="<<max_dis<<"\n";
      max_dis = std::min(*itMed * 2, size_t(mean+0.5));
      //std::cerr<<"max_dis="<<max_dis<<"\n";
    }
#endif //0


    const int max_dis_square = max_dis * max_dis;

    for (const CC &ccItem : ccsInfo) {

      const size_t ccSize = ccItem.size();
      if (ccSize >= minCCSize) { //(to remove small CCs)

	//compute AABB of cc

	int max_x = INT_MIN, max_y = INT_MIN;
	int min_x = INT_MAX, min_y = INT_MAX;

	for (size_t i = 0; i < ccSize; ++i) {

	  const int x = ccItem[i].x;
	  const int y = ccItem[i].y;

	  if (max_x < x)
	    max_x = x;
	  if (max_y < y)
	    max_y = y;

	  if (min_x > x)
	    min_x = x;
	  if (min_y > y)
	    min_y = y;
	}

	const int cc_width = max_x - min_x;
	const int cc_height = max_y - min_y;

	const int dis_square = (cc_width * cc_width + cc_height * cc_height);

	//remove boundary connected componants //B???

	if (dis_square <= max_dis_square) { //(to remove big CCs)

	  for (size_t i = 0; i < ccSize; ++i) {
	    const int x = ccItem[i].x;
	    const int y = ccItem[i].y;
	    _mat_CCs.at<uchar>(y, x) = 0;
	    mat_Accepted_CCs.at<uchar>(y, x) = 255;
	  }

	  //count continuous segments of cc,
	  //both vertically and horizontally,
	  //to compute _Mean_CC_Stroke.

	  int width_of_CC = 0;
	  int counter = 0;

#if 1
	  //Cuong's version
	  //It traverses the bounding box and not only the CC
	  // (thus if several CC bounding boxes intersect, we count their pixels
	  // several times)
	  // [In the original version we could also go outside the CC bounding
	  // box]
	  //Besides, it accesses the image to do so, and thus is slower.

	  for (int y = min_y; y <= max_y; ++y) {
	    const uchar *m = _mat_binary.ptr<uchar>(y);
	    for (int x = min_x; x <= max_x; ++x) {
	      assert(x >= 0 && x < _width && y >= 0 && y < _height);
	      {
		int d = 0;
		while (x <= max_x && m[x] == 0) {
		  ++d;
		  ++x;
		}
		if (d > 0) {
		  width_of_CC += d;
		  ++counter;
		}
	      }
	    }
	  }

	  for (int x = min_x; x <= max_x; ++x) {
	    for (int y = min_y; y <= max_y; ++y) {
	      assert(x >= 0 && x < _width && y >= 0 && y < _height);
	      {
		int d = 0;
		while (y < _height && _mat_binary.at<uchar>(y, x) == 0) {
		  ++d;
		  ++y;
		}
		if (d > 0) {
		  width_of_CC += d;
		  ++counter;
		}
	      }
	    }
	  }

#else

	  //Boris' version.
	  //Only traverse the CC.
	  //which version is correct ??? [it should change only _Mean_CC_Stroke]
	  {

	    CC cc = ccItem;
	    std::sort(cc.begin(), cc.end(), PtSorter_Y_X());

	    int prev_x = cc[0].x;
	    int prev_y = cc[0].y;
	    width_of_CC = ccSize;
	    counter = 1;
	    for (size_t i = 1; i < ccSize; ++i) { //start from 1
	      const int x = cc[i].x;
	      const int y = cc[i].y;
	      if (y == prev_y) {
		if (x > prev_x + 1) {
		  ++counter;
		}
	      } else {
		assert(y > prev_y);
		++counter;
	      }
	      prev_x = x;
	      prev_y = y;
	    }

	    std::sort(cc.begin(), cc.end(), PtSorter_X_Y());
	    prev_x = cc[0].x;
	    prev_y = cc[0].y;
	    width_of_CC += ccSize;
	    counter += 1;
	    for (size_t i = 1; i < ccSize; ++i) { //start from 1
	      const int x = cc[i].x;
	      const int y = cc[i].y;
	      if (x == prev_x) {
		if (y > prev_y + 1)
		  ++counter;
	      } else {
		assert(x > prev_x);
		++counter;
	      }
	      prev_x = x;
	      prev_y = y;
	    }
	  }

#endif //0

	  assert(counter > 0);
	  _Mean_CC_Stroke += width_of_CC / static_cast<float>(counter);

	  ++id_cc;

	  //paint enlarged AABB in _mat_reduce_pixels
	  const int y0 = std::max(min_y - cc_height / 2, 0);
	  const int y1 = std::min(max_y + cc_height / 2, _height - 1);
	  const int x0 = std::max(min_x - cc_width / 2, 0);
	  const int x1 = std::min(max_x + cc_width / 2, _width - 1);
	  for (int y = y0; y <= y1; ++y) {
	    uchar *m = _mat_reduce_pixels.ptr<uchar>(y);
	    for (int x = x0; x <= x1; ++x) {
	      assert(x >= 0 && x < _width && y >= 0 && y < _height);
	      m[x] = 255;
	    }
	  }
	}
      }
    }

    _Mean_CC_Stroke = _Mean_CC_Stroke / id_cc;
    _nb_connectedcomponants = id_cc;

    // get contour pixels of CCs
    std::vector<cv::Vec4i> hierarchy;
    findContours(mat_Accepted_CCs,
		 _contours,
		 hierarchy,
		 cv::RETR_TREE,
		 cv::CHAIN_APPROX_NONE,
		 cv::Point(0, 0));

    _mat_contour = cv::Mat::zeros(_mat_binary.size(), _mat_binary.type());

    for (const std::vector<cv::Point> &v : _contours)
      for (const cv::Point p : v) //(copy of Point)
	_mat_contour.at<uchar>(p.y, p.x) = 255;

    mat_Accepted_CCs.release();
  }

  void
  GrayscaleCharsDegradationModel::separateSeedPointsByTypes(
							    float percent_cheval,
							    float percent_diffusion)
  {

    //assert(0.f<= percent_cheval && percent_cheval <= 100.f);
    //assert(0.f<= percent_diffusion && percent_diffusion <= 100.f);

    /**/
    size_t nbDiffusion =
      static_cast<size_t>(percent_diffusion * _nbSPs_User * 0.01f);
    size_t nbCheval = static_cast<size_t>(percent_cheval * _nbSPs_User * 0.01f);
    assert(nbDiffusion + nbCheval <= (size_t)_nbSPs_User);
    const size_t nbIndependent = _nbSPs_User - nbDiffusion - nbCheval;

    std::vector<float> listFirstThreslhold;
    std::vector<float> listSecondThreslhold;
    listFirstThreslhold.reserve(_nbSPs_User);  //B
    listSecondThreslhold.reserve(_nbSPs_User); //B
    size_t counter = 0;
    // Only the black seed-point pixels will become disconnection spots

    for (const Seedpoint &sp : _listSeedPoints) {
      if (sp.type == 5 && !sp.pixel.isBackground) {
	listSecondThreslhold.push_back(sp.b_cheval);
      }
    }

    if (!listSecondThreslhold.empty() && nbDiffusion > 0) {

      if (nbDiffusion > listSecondThreslhold.size())
	nbDiffusion = listSecondThreslhold.size();

      std::sort(listSecondThreslhold.begin(), listSecondThreslhold.end());
      assert(nbDiffusion > 0); //B
      const float secondThreshold = listSecondThreslhold[nbDiffusion - 1];

      for (Seedpoint &sp : _listSeedPoints) {
	if (sp.type == 5) {
	  if (sp.b_cheval <= secondThreshold && !sp.pixel.isBackground) {
	    sp.type = 2;
	    ++counter;
	  }
	  if (counter > nbDiffusion)
	    break;
	}
      }

      for (Seedpoint &sp : _listSeedPoints) {
	if (sp.type == 5) {
	  sp.type = 4;
	}
      }
    }

    // Overlapping type
    listFirstThreslhold.clear();
    counter = 0;
    if (nbCheval > 0) {
      for (const Seedpoint &sp : _listSeedPoints) {
	if (sp.type == 4)
	  listFirstThreslhold.push_back(sp.b_tache);
      }

      if (!listFirstThreslhold.empty()) {
	std::sort(listFirstThreslhold.begin(), listFirstThreslhold.end());

	if (nbCheval > listFirstThreslhold.size())
	  nbCheval = listFirstThreslhold.size();
	assert(nbCheval > 0); //B
	const float firstThreshold = listFirstThreslhold[nbCheval - 1];

	for (Seedpoint &sp : _listSeedPoints) {
	  if (sp.type == 4 && sp.b_tache <= firstThreshold) {
	    sp.type = 1;
	    ++counter;
	  }
	  if (counter > nbCheval)
	    break;
	}
	for (Seedpoint &sp : _listSeedPoints)
	  if (sp.type == 4)
	    sp.type = 3;
      }
    }

    //// Independent spot
    // All most of background pixels have smallest distance and subtraction of two
    // thresholds
    // Therefore, we choose a half of background pixels and another of foreground
    // pixels.
    listFirstThreslhold.clear();
    listSecondThreslhold.clear();

    if (nbIndependent > 0) {
      for (Seedpoint &sp : _listSeedPoints) {
	if (sp.type == 3) {
	  if (sp.b_tache > _MAX_a0)
	    sp.b_tache = _MAX_a0;

	  if (sp.pixel.isBackground)
	    listFirstThreslhold.push_back(sp.b_tache);
	  else
	    listSecondThreslhold.push_back(sp.b_tache);
	}
      }
    }

    counter = 0;

    if ((nbIndependent / 2) < listFirstThreslhold.size() &&
	!listFirstThreslhold.empty()) {

      std::sort(listFirstThreslhold.begin(), listFirstThreslhold.end());

      assert(nbIndependent / 2 < listFirstThreslhold.size());
      const float thres =
	listFirstThreslhold[listFirstThreslhold.size() - nbIndependent / 2];

      for (Seedpoint &sp : _listSeedPoints) {
	if (sp.type == 3 && sp.pixel.isBackground && sp.b_tache >= thres) {
	  sp.type = 0;
	  ++counter;
	}
	if (counter > nbIndependent / 2)
	  break;
      }
    } else {
      for (Seedpoint &sp : _listSeedPoints) {
	if (sp.type == 3 && sp.pixel.isBackground) {
	  sp.type = 0;
	  ++counter;
	}
      }
    }

    const int selectedBGSPs = counter;
    counter = 0;
    if ((nbIndependent - selectedBGSPs) < listSecondThreslhold.size() &&
	!listSecondThreslhold.empty()) {

      sort(listSecondThreslhold.begin(), listSecondThreslhold.end());

      assert((nbIndependent - selectedBGSPs) < listSecondThreslhold.size());
      const float thres = listSecondThreslhold[listSecondThreslhold.size() -
					       (nbIndependent - selectedBGSPs)];

      for (Seedpoint &sp : _listSeedPoints) {
	if (sp.type == 3 && !sp.pixel.isBackground && sp.b_tache >= thres) {
	  sp.type = 0;
	  ++counter;
	}
	if (counter > (nbIndependent - selectedBGSPs))
	  break;
      }
    } else {
      for (Seedpoint &sp : _listSeedPoints) {
	if (sp.type == 3 && !sp.pixel.isBackground) {
	  sp.type = 0;
	  ++counter;
	}
      }
    }
    // if not enough Independent spots
    listFirstThreslhold.clear();

    if ((selectedBGSPs + counter) < nbIndependent) {
      size_t nbNeedMore = nbIndependent - selectedBGSPs - counter;
      counter = 0;
      for (const Seedpoint &sp : _listSeedPoints) {
	if (sp.type == 3)
	  listFirstThreslhold.push_back(sp.b_tache);
      }

      if (!listFirstThreslhold.empty()) {
	std::sort(listFirstThreslhold.begin(), listFirstThreslhold.end());

	if (listFirstThreslhold.size() < nbNeedMore)
	  nbNeedMore = listFirstThreslhold.size();
	assert(nbNeedMore <= listFirstThreslhold.size()); //B
	const float thres =
	  listFirstThreslhold[listFirstThreslhold.size() - nbNeedMore];

	for (Seedpoint &sp : _listSeedPoints) {
	  if (sp.type == 3 && sp.b_tache >= thres) {
	    sp.type = 0;
	    ++counter;
	  }
	  if (counter > nbNeedMore)
	    break;
	}
      }
    }

    // remove useless seedpoints
    // that is seedpoints with type not in [0; 2].
#if 0
    std::vector<int> removeIds;
    for (size_t i = 0; i<_listSeedPoints.size(); ++i) {
      const Seedpoint &sp = _listSeedPoints[i];
      if (sp.type < 0 || sp.type > 2) {
	removeIds.push_back(i);
      }
    }

    std::sort(removeIds.begin(), removeIds.end());

    if (! removeIds.empty())
      for (int i=removeIds.size()-1; i>=0; --i) {
	assert(size_t(removeIds[i]) < _listSeedPoints.size());
	_listSeedPoints.erase(_listSeedPoints.begin()+removeIds.at(i));
      }

#else

    const size_t sz0 = _listSeedPoints.size();
    size_t sz = sz0; //not const !
    for (size_t i = 0; i < sz;) {
      const Seedpoint &sp = _listSeedPoints[i];
      if (sp.type < 0 || sp.type > 2) {
	sz -= 1;
	if (sz > sz0) {
	  break;
	}
	std::swap(_listSeedPoints[i], _listSeedPoints[sz]);
      }
      else {
	++i;
      }
    }
    sz = (sz <= sz0 ? sz : 0);
    _listSeedPoints.resize(sz);

#endif //0

    /*
      removeIds.clear();
      listFirstThreslhold.clear();
      listSecondThreslhold.clear();
    */
  }

  /*
    void GrayscaleCharsDegradationModel::RapidlyGrayscaleDegradationByTypes()
    {

    std::vector<cv::Rect> listRecs;
    listRecs.reserve(_listSeedPoints.size());

    // calculate mean by region
    int meanB = _avgBackground;
    int meanF = _avgForeground;

    MyRNG rng((unsigned int) time(nullptr));
    // generator for background
    NormalDistribution var_nor_Background(rng, meanB, _sigma_gausien);
    // generator for foreground
    NormalDistribution var_nor_Forground(rng, meanF, _sigma_gausien);

    for (Seedpoint& sp : _listSeedPoints) {

    // size of ellipse
    _g = 0.6f;
    int semi_major_axis = floorf(sp.size + 0.5);
    int semi_minor_axis = floorf((1 - _g)*sp.size + 0.5);
    if (semi_major_axis <= 1)
    semi_major_axis = 2;

    // Get all pixels inside ellipse
    cv::Rect rec;
    int max_B = INT_MIN, min_B = INT_MAX;
    std::vector<float*> lines = getEllipsePoints(rec, max_B, min_B, sp.pixel.pos, semi_minor_axis, semi_major_axis, sp.pixel.gradient_angle);
    listRecs.push_back(rec);

    //==========DEBUG===================
    //        for (float* line : lines) {
    //            int id = 3;
    //            int n = line[0];
    //            while (id<n) {
    //                int x = line[id]; ++id;
    //                int y = line[id]; ++id;

    //                Point3_<uchar>* rgb = _mat_color.ptr<Point3_<uchar> >(y, x);
    //                if (sp.type == 2) {rgb->x=255;rgb->y=0;rgb->z=0;}
    //                if (sp.type == 1) {rgb->x=0;rgb->y=255;rgb->z=0;}
    //                if (sp.type == 0) {rgb->x=0;rgb->y=0;rgb->z=255;}
    //            }
    //        }
    //
    //        rectangle(_mat_color,rec, cv::Scalar(0, 0, 255));

    // foreground to background
    if (!sp.pixel.isBackground) {

    // generate the gray level of the centre
    int C_gris=var_nor_Background();
    // The difference between generated number and MEAN is not upper than SIGMA.

    if (std::max(abs(C_gris - min_B), abs(C_gris - max_B))>_MAX_Gradient && C_gris>(_avgBackground - 15))
    C_gris = _avgBackground - 15;

    if (C_gris<BLACK)
    C_gris = BLACK;
    if (C_gris>WHITE)
    C_gris=WHITE;


    if (lines.size()==1 && (lines.at(0))[0] < 4)
    _mat_output.at<uchar>(sp.pixel.pos.y, sp.pixel.pos.x) = meanB;
    else{
    for ( float* line : lines) {

    RapidlyDegradeLine(line, sp.pixel.pos, C_gris, _mat_output);
    }
    for ( float* line : lines)
    free(line);
    lines.clear();
    }
    }
    // background to foreground
    else{
    // generate the gray level of the centre
    int C_gris=var_nor_Forground();
    // The difference between generated number and MEAN is not upper than SIGMA.
    if (std::max(abs(C_gris - min_B), abs(C_gris - max_B))>_MAX_Gradient)
    C_gris = _avgForeground;

    if (C_gris<BLACK)
    C_gris = BLACK;
    if (C_gris>WHITE)
    C_gris=WHITE;

    if (lines.size()<=1 && (lines.at(0))[0] < 4)
    _mat_output.at<uchar>(sp.pixel.pos.y, sp.pixel.pos.x) = meanF;
    else{

    for ( float* line : lines) {
    RapidlyDegradeLine(line, sp.pixel.pos, C_gris, _mat_output);
    }

    for ( float* line : lines)
    free(line);
    lines.clear();
    }

    }




    }
    //===================== APPLY THE FILTER GAUSIAN AT A NOISE REGION ===========================
    int mask = 1;
    for (const cv::Rect &r : listRecs) {
    if (r.x>=0 && r.y>= 0 && (r.x + r.width)>=0 && (r.x + r.width) <_width
    && (r.y+ r.height)>=0 && (r.y+ r.height) <_height
    && r.width >= mask && r.height>=mask) {
    cv::Mat roiImg = _mat_output(r);
    cv::Mat roiImg_Blur;
    cv::GaussianBlur(roiImg, roiImg_Blur, cv::Size(mask,mask), 0);

    for (int x=0;x<r.width;++x)
    for (int y = 0;y<r.height;++y)
    if ((r.y+y)<_height && (r.y + y) >=0 && (r.x + x) < _width && (r.x + x)>=0)
    _mat_output.at<uchar>(r.y + y, r.x +x) = roiImg_Blur.at<uchar>(y, x);

    roiImg.release();
    roiImg_Blur.release();
    }
    }

    listRecs.clear();
    //==========DEBUG====================
    //    imwrite("rec_color.png",_mat_color);
    }
  */

  void
  GrayscaleCharsDegradationModel::grayscaleDegradationByTypes()
  {

    std::vector<cv::Rect> listRecs;
    listRecs.reserve(_listSeedPoints.size());

    // calculate mean by region
    int meanB = _avgBackground;
    int meanF = _avgForeground;

    MyRNG rng(static_cast<unsigned int>(time(nullptr)));
    // generator for background
    NormalDistribution var_nor_Background(rng, meanB, _sigma_gausien);
    // generator for foreground
    NormalDistribution var_nor_Forground(rng, meanF, _sigma_gausien);

    for (const Seedpoint &sp : _listSeedPoints) {

      // generator for foreground
      double mean_g = 15; //(1 - 1/(double)sp.size);
      double sigma_g = 0.15;

      if (sp.type == 0)
	mean_g = 0.15;
      else if (sp.type == 1) {
	mean_g = 0.5;
	sigma_g = 0.5;
      } else if (sp.type == 2) {
	mean_g = 0.6;
	sigma_g = 2.5;
      }

      //==========DEBUG===================
      //        Point3_<uchar>* rgb = _mat_color.ptr<Point3_<uchar>
      //        >(sp.pixel.pos.y, sp.pixel.pos.x);
      //        rgb->x=255;rgb->y=0;rgb->z=0;

      // flattenning factor : need for overlapping and disconnection spots
      NormalDistribution var_nor_g(rng, mean_g, sigma_g);
      _g = var_nor_g();
      int limite_g = 0;
      //while (_g<=(mean_g-sigma_g) || _g>=(mean_g+sigma_g || _g <0 || _g>1) ) {
      while (_g <= (mean_g - sigma_g) || _g >= (mean_g + sigma_g) || _g < 0 ||
	     _g > 1) { //B ???
	_g = var_nor_g();
	++limite_g;
	if (limite_g > 500) {
	  _g = 0.5;
	  break;
	}
      }

      if (_g > 0.9f)
	_g = 0.9f;
      else if (_g < 0.1f && sp.pixel.isBackground)
	_g = 0.1f;
      if (_g < 0.f)
	_g = mean_g;

      //B: BUG? 27/10/2017:
      //For small rectangles, we may have (but not always) an ugly "+" shapes
      //To avoid these "+" shapes, we change min size of semi_major_axis to 3


      // size of ellipse
      int semi_major_axis = floorf(sp.size + 0.5f);
#if 0
      if (semi_major_axis <= 1)
	semi_major_axis = 2;
#else
      if (semi_major_axis < 3) //B: 27/10/2017
	semi_major_axis = 3;
#endif
      const int semi_minor_axis = floorf((1 - _g) * sp.size + 0.5f);


      // Get all pixels inside ellipse
      cv::Rect rec;
      int max_B = INT_MIN, min_B = INT_MAX;

      std::vector<float *> lines = getEllipsePoints(rec,
						    max_B,
						    min_B,
						    sp.pixel.pos,
						    semi_minor_axis,
						    semi_major_axis,
						    sp.pixel.gradient_angle);
      listRecs.push_back(rec);

      //std::cerr<<"semi_major_axis="<<semi_major_axis<<" semi_minor_axis="<<semi_minor_axis<<" pos="<<sp.pixel.pos<<" gradient_angle="<<sp.pixel.gradient_angle<<" => rec="<<rec<<" min_B="<<min_B<<" max_B="<<max_B<<"\n";

      //==========DEBUG===================
      //        for (float* line : lines) {
      //            int id = 3;
      //            int n = line[0];
      //            while (id<n) {
      //                int x = line[id]; ++id;
      //                int y = line[id]; ++id;

      //                Point3_<uchar>* rgb = _mat_color.ptr<Point3_<uchar> >(y,
      //                x);
      //                if (sp.type == 2) {rgb->x=255;rgb->y=0;rgb->z=0;}
      //                if (sp.type == 1) {rgb->x=0;rgb->y=255;rgb->z=0;}
      //                if (sp.type == 0) {rgb->x=0;rgb->y=0;rgb->z=255;}
      //            }
      //        }
      //rectangle(_mat_color,rec, cv::Scalar(0, 0, 255));

      // foreground to background
      if (!sp.pixel.isBackground) {

	// generate the gray level of the centre
	int C_gris = var_nor_Background();
	// The difference between generated number and MEAN is not upper than
	// SIGMA.
	int limiter = 0;
	while (std::max(abs(C_gris - min_B), abs(C_gris - max_B)) >
               _MAX_Gradient &&
	       C_gris > (_avgBackground - 15)) {
	  C_gris = var_nor_Background();
	  ++limiter;
	  if (limiter > 1000)
	    break;
	}
	if (C_gris < BLACK)
	  C_gris = BLACK;
	else if (C_gris > WHITE)
	  C_gris = WHITE;

	if (lines.size() == 1 && (lines[0])[0] < 4) {
	  _mat_output.at<uchar>(sp.pixel.pos.y, sp.pixel.pos.x) = meanB;
	} else {
	  for (const float *line : lines) {
	    DegradedLine(
			 line, sp.pixel.pos, C_gris, 10, _sigma_gausien, _mat_output, true);
	  }
	}
      }
      // background to foreground
      else {
	// generate the gray level of the centre
	int C_gris = var_nor_Forground();
	// The difference between generated number and MEAN is not upper than
	// SIGMA.
	int limiter = 0;
	while (std::max(abs(C_gris - min_B), abs(C_gris - max_B)) >
	       _MAX_Gradient) {
	  C_gris = var_nor_Forground();
	  ++limiter;
	  if (limiter > 100)
	    break;
	}
	if (C_gris < BLACK)
	  C_gris = BLACK;
	else if (C_gris > WHITE)
	  C_gris = WHITE;

	if (lines.size() <= 1 && (lines.at(0))[0] < 4) {
	  _mat_output.at<uchar>(sp.pixel.pos.y, sp.pixel.pos.x) = meanF;
	} else {
	  for (const float *line : lines) {
	    DegradedLine(
			 line, sp.pixel.pos, C_gris, 10, _sigma_gausien, _mat_output, false);
	  }
	}
      }

      for (float *line : lines)
	free(line);
      lines.clear();
    }

    //=========== APPLY THE FILTER GAUSIAN AT A NOISE REGION =================
    constexpr int mask = 1;
    for (const cv::Rect &r : listRecs) {
      if (r.x >= 0 && r.y >= 0 && (r.x + r.width) < _width &&
	  (r.y + r.height) < _height && r.width >= mask && r.height >= mask) {
	assert((r.x + r.width) >= 0);
	assert((r.y + r.height) >= 0);
	cv::Mat roiImg = _mat_output(r);
	cv::Mat roiImg_Blur;
	cv::GaussianBlur(roiImg, roiImg_Blur, cv::Size(mask, mask), 0);

	//copy roiImg in _mat_output
	for (int y = 0; y < r.height; ++y) {
	  uchar *dst = _mat_output.ptr<uchar>(r.y + y);
	  const uchar *src = roiImg_Blur.ptr<uchar>(y);
	  for (int x = 0; x < r.width; ++x) {
	    assert((r.y + y) < _height && (r.y + y) >= 0 && (r.x + x) < _width &&
		   (r.x + x) >= 0);
	    dst[r.x + x] = src[x];
	  }
	}
      }
    }
    //B: Can we have overlapping rectangles ?
    // If we have overlapping rectangles, the same pixels may be blurred several
    // times...

    listRecs.clear();
  }

  void
  GrayscaleCharsDegradationModel::RapidlyDegradeLine(const float *Line,
						     cv::Point Centre,
						     int Centre_gray,
						     cv::Mat &imgGrayOutput)
  {
    const int n = Line[0];
    double deltaDis = Line[1];
    int B_gris = Line[2];

    if (deltaDis > 0) {
      int id = 3;

      while (id < n) {
	cv::Point B1;
	B1.x = Line[id];
	++id;
	B1.y = Line[id];
	++id;

	const double dis = sqrt((B1.x - Centre.x) * (B1.x - Centre.x) +
				(B1.y - Centre.y) * (B1.y - Centre.y));
	imgGrayOutput.at<uchar>(B1.y, B1.x) =
	  Centre_gray + (B_gris - Centre_gray) * dis / deltaDis;
      }
    }
  }

  void
  GrayscaleCharsDegradationModel::DegradedLine(const float *Line,
					       cv::Point Centre,
					       int Centre_gray,
					       int pixel_signma,
					       int sigma_gaussien,
					       cv::Mat &imgGrayOutput,
					       bool isFtoB)
  {
    MyRNG rng(static_cast<unsigned int>(time(nullptr)));

    const int n = Line[0];
    const float deltaDis = Line[1];
    int B_gris = Line[2];

    //UGLY: we should have a struct Line !!!

    if (deltaDis > 0) {
      int id = 3;

      while (id < n) {
	cv::Point B1;
	B1.x = Line[id];
	++id;
	B1.y = Line[id];
	++id;

	const float dis = sqrtf((B1.x - Centre.x) * (B1.x - Centre.x) +
				(B1.y - Centre.y) * (B1.y - Centre.y));
	int B1_moyen_gris = 0, limiter1 = 0;

	if (isFtoB) {
	  B1_moyen_gris = Centre_gray + (B_gris - Centre_gray) * dis / deltaDis;
	} else {
	  if (B_gris == 0) {
	    B_gris = 1;
	  }
	  if (Centre_gray == 0) {
	    Centre_gray = 1;
	  }
	  B1_moyen_gris = /*C_gris + (B_gris - C_gris)*dis/deltaDis;}*/ exp(
									    (log(B_gris) - log(Centre_gray)) * (dis / deltaDis) +
									    log(Centre_gray));
	} //C_gris + (B_gris - C_gris)*dis/deltaDis;

	//NUMBER GENERATOR : normal distribution
	NormalDistribution var_nor_ForPixelEcllipse(
						    rng, B1_moyen_gris, pixel_signma);

	int grisGenerator = var_nor_ForPixelEcllipse();

	if (isFtoB) {
	  while (grisGenerator < (B1_moyen_gris - pixel_signma) ||
		 grisGenerator > (B1_moyen_gris + sigma_gaussien)) {
	    grisGenerator = var_nor_ForPixelEcllipse();
	    ++limiter1;
	    if (limiter1 > 100) {
	      break;
	    }
	  }
	}
	else {
	  while (grisGenerator < 0 ||
		 grisGenerator < (B1_moyen_gris - pixel_signma) ||
		 grisGenerator > (B1_moyen_gris + pixel_signma)) {
	    grisGenerator = var_nor_ForPixelEcllipse();
	    ++limiter1;
	    if (limiter1 > 100) {
	      break;
	    }
	  }
	}
	//
	if (grisGenerator < BLACK) {
	  grisGenerator = BLACK;
	}
	else if (grisGenerator > WHITE) {
	  grisGenerator = WHITE;
	}
	//

	assert(B1.y >= 0 && B1.y < imgGrayOutput.rows && B1.x >= 0 &&
	       B1.x < imgGrayOutput.cols); //B

	if (!isFtoB) {
	  if (imgGrayOutput.at<uchar>(B1.y, B1.x) < Centre_gray) {
	    continue;
	  }
	}
	imgGrayOutput.at<uchar>(B1.y, B1.x) = grisGenerator;
      }
    }
  }

  /**
     Compute minimal distance from @a pixel to the closest white contour in _mat_contour,
     in eight directions.

  */
  int
  GrayscaleCharsDegradationModel::calculateApproximatelyDistanceFromBord(
									 const Pixel &pixel) const
  {
    const int x = pixel.pos.x;
    const int y = pixel.pos.y;

    assert(x >= 0 && x < _width && y >= 0 && y < _height);

    if (_mat_contour.at<uchar>(y, x) == WHITE) //is edge pixel
      return 0;

    int minDist = 0;

    int dist = 0;
    const int x0 = std::max(x - _local_zone, 0);
    {
      const uchar *mc = _mat_contour.ptr<uchar>(y);
      for (int i = x; i >= x0; --i) {
	assert(i >= 0);
	if (mc[i] == WHITE) {
	  break;
	}
	++dist; //0 degree
      }
    }
    minDist = dist;

    dist = 0;
    const int x1 = std::min(x + _local_zone, _width);
    {
      assert(y < _mat_contour.rows);
      const uchar *mc = _mat_contour.ptr<uchar>(y);
      for (int i = x; i < x1; ++i) {
	assert(i < _width);
	if (mc[i] == WHITE) {
	  break;
	}
	++dist; //0 degree
	if (dist >= minDist) {
	  break;
	}
      }
    }
    minDist = std::min(dist, minDist);

    dist = 0;
    const int y0 = std::max(y - _local_zone, 0);
    for (int i = y; i >= y0; --i) {
      assert(i >= 0);
      assert(i >= 0 && i < _mat_contour.rows && x >= 0 && x < _mat_contour.cols);
      if (_mat_contour.at<uchar>(i, x) == WHITE) {
	break;
      }
      ++dist; // 90 degree
      if (dist >= minDist) {
	break;
      }
    }
    minDist = std::min(dist, minDist);

    dist = 0;
    const int y1 = std::min(y + _local_zone, _height);
    for (int i = y; i < y1; ++i) {
      assert(i < _height);
      assert(i >= 0 && i < _mat_contour.rows && x >= 0 && x < _mat_contour.cols);
      if (_mat_contour.at<uchar>(i, x) == WHITE) {
	break;
      }
      ++dist; // 90 degree
      if (dist >= minDist) {
	break;
      }
    }
    minDist = std::min(dist, minDist);

    dist = 0;
    int j = y;
    for (int i = x; i < x1; ++i) {
      --j;
      if (j >= y0) {
	assert(i < _width);
	assert(j >= 0 && j < _mat_contour.rows && i >= 0 &&
	       i < _mat_contour.cols);
	if (_mat_contour.at<uchar>(j, i) == WHITE) {
	  break;
	}
	++dist; //45 degree
	if (dist >= minDist) {
	  break;
	}
      }
      else {
	break;
      }
    }
    minDist = std::min(dist, minDist);

    dist = 0;
    j = y;
    for (int i = x; i < x1; ++i) {
      ++j;
      if (j < y1) {
	assert(i < _width);
	assert(j >= 0 && j < _mat_contour.rows && i >= 0 &&
	       i < _mat_contour.cols);
	if (_mat_contour.at<uchar>(j, i) == WHITE) {
	  break;
	}
	++dist; // -45 degree
	if (dist >= minDist) {
	  break;
	}
      }
      else {
	break;
      }
    }
    minDist = std::min(dist, minDist);

    dist = 0;
    j = y;
    for (int i = x; i >= x0; --i) {
      ++j;
      if (j < y1) {
	assert(i >= 0);
	assert(j >= 0 && j < _mat_contour.rows && i >= 0 &&
	       i < _mat_contour.cols);
	if (_mat_contour.at<uchar>(j, i) == WHITE)
	  break;
	++dist; // 45 degree
	if (dist >= minDist) {
	  break;
	}
      }
      else {
	break;
      }
    }
    minDist = std::min(dist, minDist);

    dist = 0;
    j = y;
    for (int i = x; i >= x0; --i) {
      --j;
      if (j > y0) {
	assert(i >= 0);
	assert(j >= 0 && j < _mat_contour.rows && i >= 0 &&
	       i < _mat_contour.cols);
	if (_mat_contour.at<uchar>(j, i) == WHITE) {
	  break;
	}
	++dist; // -45 degree
	if (dist >= minDist) {
	  break;
	}
      }
      else {
	break;
      }
    }
    minDist = std::min(dist, minDist);

    return minDist;
  }

  int
  GrayscaleCharsDegradationModel::calculateDistanceFromBord(
							    const Pixel &pixel) const
  {
    constexpr float rate = 1.5f;

    const int localZone = rate * _local_zone;
    const int y0 = std::max(pixel.pos.y - localZone, 0);
    const int y1 = std::min(pixel.pos.y + localZone, _height);
    const int x0 = std::max(pixel.pos.x - localZone, 0);
    const int x1 = std::min(pixel.pos.x + localZone, _width);

    float minDistSq = FLT_MAX;

    for (int y = y0; y < y1; ++y) {
      const uchar *m = _mat_contour.ptr<uchar>(y);
      for (int x = x0; x < x1; ++x) {
	assert(x >= 0 && y >= 0 && x < _width && y < _height);
	if (m[x] == 255) {
	  const float distSq = (pixel.pos.x - x) * (pixel.pos.x - x) +
	    (pixel.pos.y - y) * (pixel.pos.y - y);
	  if (distSq < minDistSq) {
	    minDistSq = distSq;
	  }
	}
      }
    }

    return sqrt(minDistSq);  //B:CONVERSION TO INT ? Is it really what we want ???
  }

  std::vector<float *>
  GrayscaleCharsDegradationModel::getEllipsePoints(cv::Rect &rec,
						   int &max_B,
						   int &min_B,
						   cv::Point centre,
						   double semi_minor_axis,
						   double semi_major_axis,
						   double theta)
  {
    //B: This code is UGLY & very slow !!!
    //1) We allocate a BIG cv::Mat each time
    //2) We draw instead of finding values mathematically
    //3) There is a lot of overdraw each time we draw the lines.
    //   Thus there are a lot of duplicate points in returned vector
    //4) Returned vector contains malloced lines that we must free... Not cache friendly at all...

    std::vector<float *> Ls;

    const int ex = semi_major_axis + 10;
    cv::Mat box = cv::Mat::zeros(cv::Size(_width + ex, _height + ex),
				 CV_8UC1); //B: TOO BIG ??????
    const cv::Point newCentre(centre.x + ex / 2, centre.y + ex / 2);

    cv::ellipse(box,
		newCentre,
		cv::Size(semi_major_axis, semi_minor_axis),
		theta,
		0,
		360,
		cv::Scalar(WHITE, WHITE, WHITE),
		-1);

    const int localZone2 = 2 * _local_zone;
    const int xr =
      (newCentre.x - localZone2) >= 0 ? (newCentre.x - localZone2) : (0);
    const int yr =
      (newCentre.y - localZone2) >= 0 ? (newCentre.y - localZone2) : (0);

    const int wo = (newCentre.x - localZone2) >= 0 ? (localZone2) : (newCentre.x);
    const int ho = (newCentre.y - localZone2) >= 0 ? (localZone2) : (newCentre.y);
    const int wr =
      wo + ((newCentre.x + localZone2) < box.cols ? (localZone2)
	    : (box.cols - newCentre.x));
    const int hr =
      ho + ((newCentre.y + localZone2) < box.rows ? (localZone2)
	    : (box.rows - newCentre.y));

    const cv::Rect r(xr, yr, wr, hr);

    cv::Mat mask = box(r);

    std::vector<std::vector<cv::Point>> contours(1);
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(mask,
		     contours,
		     hierarchy,
		     cv::RETR_TREE,
		     cv::CHAIN_APPROX_NONE,
		     cv::Point(0, 0));
    mask.release();

    assert(contours.size() > 0);
    const std::vector<cv::Point> &v = contours[0];
    uchar scalar = 10;
    int min_x = INT_MAX, min_y = INT_MAX;
    int max_x = INT_MIN, max_y = INT_MIN;
    for (cv::Point p : v) {
      p.x = p.x + r.x;
      p.y = p.y + r.y;

      if (min_x > p.x)
	min_x = p.x;
      if (max_x < p.x)
	max_x = p.x;
      if (min_y > p.y)
	min_y = p.y;
      if (max_y < p.y)
	max_y = p.y;

      // draw a line
      cv::line(box, newCentre, p, cv::Scalar(scalar, scalar, scalar));
      //

      //std::cerr<<"min_x="<<min_x<<" max_x="<<max_x<<" ; min_y="<<min_y<<" max_y="<<max_y<<"\n";

      //B:BUG ? Why do we search with a partial bounding box ????? [AABB not yet fully computed !!!]
      // Shouldn't we search along the line ???

      std::vector<cv::Point> pointInLine;
      {
	const int y0 = std::max(std::max(min_y - 5, 0), ex / 2);
	const int y1 = std::min(std::min(max_y + 5, box.rows), _mat_gray.rows + ex / 2);
	const int x0 = std::max(std::max(min_x - 5, 0), ex / 2);
	const int x1 = std::min(std::min(max_x + 5, box.cols), _mat_gray.cols + ex / 2);
	for (int y = y0; y < y1; ++y) {
	  const uchar *b = box.ptr<uchar>(y);
	  const int yt = y - ex / 2;
	  assert(yt >= 0 && yt < _mat_gray.rows);

	  for (int x = x0; x < x1; ++x) {
	    assert(y >= 0 && y < box.rows && x >= 0 && x < box.cols); //B
	    if (b[x] == scalar) {
	      const int xt = x - ex / 2;
	      assert(yt >= 0 &&
		     yt < _mat_gray.rows &&
		     xt >= 0 &&
		     xt < _mat_gray.cols);
	      //B: added 05/2015: to avoid point outside image
	      pointInLine.emplace_back(xt, yt);
	    }
	  }
	}
      }

      // new line
      if (!pointInLine.empty()) {

	//B: added 03/2015 : check translated point is really in image
	cv::Point pt;
	pt.x = p.x - ex / 2;
	pt.y = p.y - ex / 2;

#if 0  //#ifndef NDEBUG
	if (! (pt.y >= 0 && pt.y < _mat_gray.rows 
	       && pt.x >= 0 && pt.x < _mat_gray.cols)) {
	  std::cerr<<"pt.x="<<pt.x<<" _mat_gray.cols="<<_mat_gray.cols<<"\n";
	  std::cerr<<"pt.y="<<pt.y<<" _mat_gray.rows="<<_mat_gray.rows<<"\n";
	}
#endif //NDEBUG

	if (pt.y >= _mat_gray.rows)
	  pt.y = _mat_gray.rows - 1;
	else if (pt.y < 0)
	  pt.y = 0;
	if (pt.x >= _mat_gray.cols)
	  pt.x = _mat_gray.cols - 1;
	else if (pt.x < 0)
	  pt.x = 0;

	assert(pt.y >= 0 && pt.y < _mat_gray.rows && pt.x >= 0 &&
	       pt.x < _mat_gray.cols);
	const int gray = _mat_gray.at<uchar>(pt.y, pt.x);

	if (max_B < gray)
	  max_B = gray;
	if (min_B > gray)
	  min_B = gray;

	const int n = 2 * static_cast<int>(pointInLine.size()) + 3;
	float *line = static_cast<float *>(malloc(sizeof(float) * n));
	if (line != nullptr) {
	  int id = 0;
      //0
	  line[id] = n;
	  ++id;
      //1
	  //B: should we use p or pt here ???
	  line[id] = sqrtf((p.x - newCentre.x) * (p.x - newCentre.x) +
			  (p.y - newCentre.y) * (p.y - newCentre.y));
	  ++id;
      //2
	  line[id] = gray;
	  ++id;
	  for (const cv::Point px : pointInLine) {

	    assert(px.y >= 0 && px.y < _mat_gray.rows && px.x >= 0 &&
		   px.x < _mat_gray.cols); //B

	    line[id] = px.x;
	    ++id;
	    line[id] = px.y;
	    ++id;
	  }

	  Ls.push_back(line);
	}
      }

      //pointInLine.clear();

      ++scalar;
    }

    rec.x = min_x - ex / 2;
    rec.y = min_y - ex / 2;
    rec.width = max_x - min_x;
    rec.height = max_y - min_y;

    //contours.clear();

    //box.release();

    return Ls;
  }

  /*
    unsigned int* GrayscaleCharsDegradationModel::calHist(cv::Mat src)
    {
    unsigned int* hist = new unsigned int[256];
    for (int i=0;i<256;++i)
    {
    hist[i]=0;
    }

    for (int i=0;i<src.rows;++i)
    {
    for (int j=0;j<src.cols;++j)
    {
    uchar grayValue = src.at<uchar>(i, j);
    hist[grayValue] = hist[grayValue] + 1;
    }
    }
    return hist;
    }
  */

  /*
    @return binarized version of _mat_gray
    Will also update _avgBackground & _avgForeground
    with black pixels (in binarized image) considered as foregound
    and the rest considered as background.

    If no foreground pixel is found, set _nb_connectedcomponants to NO_CC.
  */
  cv::Mat
  GrayscaleCharsDegradationModel::binarize()
  {
    cv::Mat img_bin;
    cv::threshold(_mat_gray, img_bin, 0, 255, cv::THRESH_OTSU);

    size_t totalBackgroundGrayValue = 0;
    size_t nbPixelBackground = 0;
    size_t totalForegroundGrayValue = 0;

    int rows = img_bin.rows;
    int cols = img_bin.cols;
    if (img_bin.isContinuous() && _mat_gray.isContinuous()) {
      cols *= rows;
      rows = 1;
    }
    for (int i = 0; i < rows; ++i) {
      const uchar *m = _mat_gray.ptr<uchar>(i);
      const uchar *b = img_bin.ptr<uchar>(i);
      for (int j = 0; j < cols; ++j) {
	if (b[j] > 0) {
	  totalBackgroundGrayValue += m[j];
	  ++nbPixelBackground;
	} else {
	  totalForegroundGrayValue += m[j];
	}
      }
    }
    assert( nbPixelBackground <= (size_t(rows)*size_t(cols)) );
    const size_t nbPixelForeground = (static_cast<size_t>(rows)*static_cast<size_t>(cols)) - nbPixelBackground;

    if (nbPixelForeground > 0 && nbPixelBackground > 0) {
      _avgBackground =
	totalBackgroundGrayValue / static_cast<float>(nbPixelBackground);
      _avgForeground =
	totalForegroundGrayValue / static_cast<float>(nbPixelForeground);
    }
    else {
      _nb_connectedcomponants = NO_CC;
      _avgBackground = 255;
      _avgForeground = 0;
    }

    return img_bin;
  }

  /*
    Update pixels, in _listPixels, probability according to their distanceToEdge & isBackground.

  */
  void
  GrayscaleCharsDegradationModel::calculatePixelsProbability()
  {
    MyRNG rng(static_cast<unsigned int>(time(nullptr)));
    UniformDistribution generator(rng, 0, 50);

    _beta = 0.000;
    _alpha = 0.000;

    for (Pixel &pixel : _listPixels) {

      const float d = pixel.distanceToEdge;

      assert(d > 0); //B pixels on borders have not been kept.

      // Compute probability to inverse a pixel.

      if (!pixel.isBackground) { // FORGROUND

	_beta = generator();

	//P(0|d, beta, f) = exp(-beta*d*d)
	pixel.probability = _beta0 * static_cast<float>(exp((-_beta) * d * d)) + _nf;

      } else { // BACKGROUND

	_alpha = generator();

	//P(1|d, anpha, f) = exp(-alpha*d*d)
	pixel.probability = _alpha0 * static_cast<float>(exp((-_alpha) * d * d)) + _nb;
      }
    }
  }

  void
  GrayscaleCharsDegradationModel::flipPixelsByProbability(
							  float percent_independent)
  {
    const size_t sz = _listPixels.size();
    if (sz == 0) {
      return;
    }

    //estimate r by using histogram
    std::vector<float> listProbability;
    listProbability.reserve(sz);

    for (const Pixel &pixel : _listPixels) {
      listProbability.push_back(pixel.probability);
    }
    assert(_listPixels.size() == listProbability.size());

    std::sort(listProbability.begin(), listProbability.end());

    const size_t nbSps_Over =
      static_cast<size_t>(_nbSPs_User + _nbSPs_User * percent_independent / 100.f);

    float flip_random = 0.0000000f;
    assert(sz == listProbability.size());
    assert(!listProbability.empty());
    if (nbSps_Over <= sz)
      flip_random = listProbability[sz - nbSps_Over];
    else
      flip_random = listProbability[0];

    if (flip_random > 1) { //B: is it possible ?????
      flip_random = 1;
    }

    _listSeedPoints.reserve(
			    std::max(sz / 100, static_cast<size_t>(_height))); //arbitrary

    //size_t countBF = 0;
    for (const Pixel &pixel : _listPixels) {
      if (pixel.probability >= flip_random && pixel.probability <= 1) {
	//if (pixel.isBackground)
	//++countBF;
	_listSeedPoints.emplace_back(pixel);
      }
    }

    _listPixels.clear();
  }


} //namespace dc
