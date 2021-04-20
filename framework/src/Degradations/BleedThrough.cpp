#include "BleedThrough.hpp"

#include <iostream>


/*
  Code inspired from original Cuong's code.
  
  TODO: This is a stencil operation, it could be coded to be better parallelized/faster.
*/

namespace dc {
  namespace BleedThrough {

  static inline
  int
  bleedThrough_kernel(int u, int current, int ixp, int ixn, int iyp, int iyn)
  {
    constexpr float dt = 0.05f;  //B: ?????  //Why isn't it a parameter of the algo ?

    //B: remove useless: / 100.0f * 100.0f
    const float c = (1.0f / (1.0f + (ixn - u) * (ixn - u) )) /* (1.0f / ((1.0f + ixn*ixn)/(0.2f*0.2f)) )*/;
    const float c1 = (1.0f / (1.0f + (ixp - u) * (ixp - u) )) /* (1.0f / ((1.0f + ixp*ixp)/(0.2f*0.2f)) )*/;
    const float c2 = (1.0f / (1.0f + (iyp - u) * (iyp - u) )) /* (1.0f / ((1.0f + iyp*iyp)/(0.2f*0.2f)) )*/;
    const float c3 = (1.0f / (1.0f + (iyn - u) * (iyn - u) )) /* (1.0f / ((1.0f + iyn*iyn)/(0.2f*0.2f)) )*/;
  
    const float delta = (c * (ixn - current)
			 + c1 * (ixp - current)
			 + c2 * (iyp -  current)
			 + c3 * (iyn -  current));
	
    const int g = static_cast<int>(dt * delta + current);

    return g;
  }


  template <typename T>
  class BleedThroughDiffusionThreadA : public cv::ParallelLoopBody
  {
  public:
        
    BleedThroughDiffusionThreadA(const cv::Mat &originalFrontImg,
				 const cv::Mat &front,
				 const cv::Mat &back,
				 cv::Mat &out);


    void operator()(const cv::Range &r) const override;

    //update @a out on Rect(x,y,w,h)
    void process(int _x, int _y, int _w, int _h) const;
  
  private:
    const cv::Mat &_originalFrontImg;
    const cv::Mat &_front;
    const cv::Mat &_back;
    cv::Mat &_out;
  };

  template <typename T>
  BleedThroughDiffusionThreadA<T>::BleedThroughDiffusionThreadA(const cv::Mat &originalFrontImg,
								const cv::Mat &front,
								const cv::Mat &back,
								cv::Mat &out) :
    _originalFrontImg(originalFrontImg),
    _front(front),
    _back(back),
    _out(out)
  {
    assert(_originalFrontImg.size() == _front.size());
    assert(_front.size() == _back.size());

    assert(_originalFrontImg.type() == _front.type());
    assert(_front.type() == _back.type());
  
    assert(_out.type() == _front.type());
    assert(_out.type() == _back.type());
  }

  template <>
  BleedThroughDiffusionThreadA<uchar>::BleedThroughDiffusionThreadA(const cv::Mat &originalFrontImg,
								    const cv::Mat &front,
								    const cv::Mat &back,
								    cv::Mat &out) :
    _originalFrontImg(originalFrontImg),
    _front(front),
    _back(back),
    _out(out)
  {
    assert(_originalFrontImg.size() == _front.size());
    assert(_front.size() == _back.size());

    assert(_originalFrontImg.type() == _front.type());
    assert(_front.type() == _back.type());
    assert(_originalFrontImg.type() == CV_8UC1);
  
    assert(_out.type() == _front.type());
    assert(_out.type() == _back.type());
  }

  //bleedThrough4b
  template <typename T>
  void BleedThroughDiffusionThreadA<T>::process(int _x, int _y, int _w, int _h) const
  {
    assert(_back.size() == _front.size());
    assert(_back.size() == _originalFrontImg.size());
    assert(_back.size() == _out.size());

    assert(_back.type() == _front.type());
    assert(_back.type() == _originalFrontImg.type());
    assert(_back.type() == _out.type());
  
    const int numChan = _back.channels();
  
    for (int y = _y; y<_h; ++y) {
      const T *v = _back.ptr<T>(y);
      const T *sr = _originalFrontImg.ptr<T>(y);
      const T *r = _front.ptr<T>(y);
      T *o = _out.ptr<T>(y);

      const int py = (y > 0 ? y - 1 : 0);
      const int ny = (y < _back.rows-1 ? y + 1 : _back.rows-1);
    
      const T *v_py = _back.ptr<T>(py);
      const T *v_ny = _back.ptr<T>(ny);
    
      for (int x=_x; x<_w; ++x) {

	const T &r_v = r[x];
	const T &sr_v = sr[x];

	const int px = (x > 0 ? x - 1 : 0);
	const int nx = (x < _back.cols-1 ? x + 1 : _back.cols-1);

	const T &iyn_v = v_py[x];
	const T &ixp_v = v[px];
	const T &ixn_v = v[nx];
	const T &iyp_v = v_ny[x];

	T &o_v = o[x];
      
	for (int k=0; k<numChan; ++k) {

	  const int current = r_v[k];
	  const int iyn = iyn_v[k];
	  const int ixp = ixp_v[k];
	  const int ixn = ixn_v[k];
	  const int iyp = iyp_v[k];
	  const int u = sr_v[k];

	  const int g = bleedThrough_kernel(u, current, ixp, ixn, iyp, iyn);
	  o_v[k] = g;
	}

      }
    }
  }

  //bleedThrough4b
  template <>
  void BleedThroughDiffusionThreadA<uchar>::process(int _x, int _y, int _w, int _h) const
  {
    assert(_back.size() == _front.size());
    assert(_back.size() == _originalFrontImg.size());
    assert(_back.size() == _out.size());
  
    assert(_back.type() == _front.type());
    assert(_back.type() == _originalFrontImg.type());
    assert(_back.type() == _out.type());
  
    assert(_back.type() == CV_8UC1);
  
    for (int y = _y; y<_h; ++y) {
      const uchar *v = _back.ptr<uchar>(y);
      const uchar *sr = _originalFrontImg.ptr<uchar>(y);
      const uchar *r = _front.ptr<uchar>(y);
      uchar *o = _out.ptr<uchar>(y);

      const int py = (y > 0 ? y - 1 : 0);
      const int ny = (y < _back.rows-1 ? y + 1 : _back.rows-1);
    
      const uchar *v_py = _back.ptr<uchar>(py);
      const uchar *v_ny = _back.ptr<uchar>(ny);
    
      for (int x=_x; x<_w; ++x) {

	const int current = r[x];
	const int u = sr[x];

	const int px = (x > 0 ? x - 1 : 0);
	const int nx = (x < _back.cols-1 ? x + 1 : _back.cols-1);

	const int iyn = v_py[x];
	const int  ixp = v[px];
	const int ixn = v[nx];
	const int iyp = v_ny[x];


	const int g = bleedThrough_kernel(u, current, ixp, ixn, iyp, iyn);
	o[x] = g;

      }
    }
  }

  template <typename T>
  void BleedThroughDiffusionThreadA<T>::operator()(const cv::Range &r) const
  {
    process(0, r.start, _front.cols, r.end);
  }

  template <>
  void BleedThroughDiffusionThreadA<uchar>::operator()(const cv::Range &r) const
  {
    process(0, r.start, _front.cols, r.end);
  }



  template <typename T>
  cv::Mat
  bleedThroughMT0(const cv::Mat &originalFrontImg, const cv::Mat &frontImg, const cv::Mat &backImg, int numIter, int numThreads=5)
  {
    assert(frontImg.size() == backImg.size());
    assert(originalFrontImg.size() == backImg.size());
  
    assert(frontImg.type() == backImg.type());
    assert(originalFrontImg.type() == backImg.type());
  
    const int height = frontImg.rows;

    cv::Mat out(frontImg.size(), frontImg.type());
    cv::Mat currFront = frontImg.clone();
  

    const int lNbIter = numIter;
    for (int p=0; p<lNbIter; ++p) {


      cv::parallel_for_(cv::Range(0, height),
			BleedThroughDiffusionThreadA<T>(originalFrontImg, currFront, backImg, out),
			numThreads);

      cv::swap(currFront, out);
    }

    out = currFront;

    return out;
  }

  cv::Mat
  bleedThroughMT0(const cv::Mat &originalFrontImg, const cv::Mat &frontImg, const cv::Mat &backImg, int numIter, int numThreads=16)
  {
    assert(frontImg.size() == backImg.size());
    assert(originalFrontImg.size() == backImg.size());

    if (numThreads < 0) {
      numThreads = cv::getNumThreads(); //B:TODO: use also height of images to decide numThreads ?
    }

    if (frontImg.type() == CV_8UC1)
      return bleedThroughMT0<uchar>(originalFrontImg, frontImg, backImg, numIter, numThreads);
    else if (frontImg.type() == CV_8UC3)
      return bleedThroughMT0<cv::Vec3b>(originalFrontImg, frontImg, backImg, numIter, numThreads);
    else if (frontImg.type() == CV_8UC4)
      return bleedThroughMT0<cv::Vec4b>(originalFrontImg, frontImg, backImg, numIter, numThreads); //B:TODO: does it make sense to do bleedThrough in alpha channel ????
    else {
      std::cerr<<"ERROR: bleedThrough: unhandled image type\n";
      assert(false);
      return originalFrontImg;
    }
  }

  /*
  cv::Mat
  bleedThroughMT0(const cv::Mat &frontImg, const cv::Mat &backImg, int numIter, int numThreads=16)
  {
    assert(frontImg.size() == backImg.size());

    return bleedThroughMT0(frontImg.clone(), frontImg, backImg, numIter, numThreads);
  }
  */


  /*
    compute crop of @a img copied onto destination of image of size @a width x @a height, 
    such as origin of &a img is at position @a x, &a y onto destination image.
    (Origin/top left corner of dstImg is supposed at (0,0)).
    If there is intersection between @a img and destination image, update @a roi, and returns true.
    Otherwise, returns false.
  
    @param[in] stainImg stain image to insert. It must be of type CV_8UC3.
    @param[in] dstImg destination image to copy onto. It must be of type 8UC3.
    @param[in] posCenter position of center of @a stainImg on destination image @a dstImg.

    @param[out] roi  produced region-of-interest of @a img if intersection.
    @param[out] newPos  produced new center position of @a img in destination image if intersection.
    @return true if intersection, false otherwise
  */
  //Some CODE DUPLICATION with GradientDomainDegradation.cpp:testCrop()
  bool
  getOverlappingPart(int width, int height, const cv::Size &imgSize, int x, int y, cv::Rect &roi)
  {
    //dstImg in img coordinate system
    const int xi0 = -x; 
    const int yi0 = -y; 
    const int xi1 = xi0 + width;
    const int yi1 = yi0 + height;

    //crop coords
    const int xc0 = std::max(xi0, 0);
    const int yc0 = std::max(yi0, 0);
    const int xc1 = std::min(xi1, imgSize.width);
    const int yc1 = std::min(yi1, imgSize.height);

    if (xc1<=xc0 || yc1<=yc0) {
      return false;
    }

    roi = cv::Rect(xc0, yc0, xc1-xc0, yc1-yc0);

    return true;
  }


  cv::Mat
  bleedThrough(const cv::Mat &frontImg, const cv::Mat &backImg, int numIter, cv::Point pos, int numThreads)
  {
    cv::Mat res = frontImg.clone();

    cv::Rect backROI;
    const bool intersection = getOverlappingPart(frontImg.cols, frontImg.rows, backImg.size(), pos.x, pos.y, backROI);
    if (intersection) {
      const cv::Mat backImgROI = backImg(backROI);

      const cv::Rect frontROI(pos.x+backROI.x, pos.y+backROI.y, backROI.width, backROI.height);
      const cv::Mat frontImgROI = frontImg(frontROI);
      assert(backImgROI.size() == frontImgROI.size());

      const cv::Mat outROI = bleedThroughMT0(frontImgROI.clone(), frontImgROI, backImgROI, numIter, numThreads);

      cv::Mat dst_roi = res(frontROI);
      outROI.copyTo(dst_roi);
    }
    return res;
  }

  cv::Mat
  bleedThroughInc(const cv::Mat &originalFrontImg, const cv::Mat &frontImg, const cv::Mat &backImg, int numIter, cv::Point pos, int numThreads)
  {
    assert(originalFrontImg.size() == frontImg.size());

    cv::Mat res = frontImg.clone();

    cv::Rect backROI;
    const bool intersection = getOverlappingPart(frontImg.cols, frontImg.rows, backImg.size(), pos.x, pos.y, backROI);
    if (intersection) {
      cv::Mat backImgROI = backImg(backROI);

      const cv::Rect frontROI(pos.x+backROI.x, pos.y+backROI.y, backROI.width, backROI.height);
      cv::Mat frontImgROI = frontImg(frontROI);
      cv::Mat originalFrontImgROI = originalFrontImg(frontROI);
      assert(backImgROI.size() == frontImgROI.size());
      assert(originalFrontImgROI.size() == frontImgROI.size());

      const cv::Mat outROI = bleedThroughMT0(originalFrontImgROI, frontImgROI, backImgROI, numIter, numThreads);

      cv::Mat dst_roi = res(frontROI);
      outROI.copyTo(dst_roi);
    }
    return res;
  }


  } //namespace BleedThrough
  
} //namespace dc
