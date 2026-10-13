#ifndef OPENCV_CONVERTER_HPP
#define OPENCV_CONVERTER_HPP

//inspired from https://github.com/pybind/pybind11/issues/2004

#include <cassert>

#include <opencv2/core/core.hpp>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace pybind11 { namespace detail{

    /*
      The casters below read the elements of a tuple one by one. An element
      that cannot be converted must make load() return false, so that
      pybind11 reports an argument of the wrong type and names the one it
      expected; handle::cast<T>() instead throws, and the caller saw a
      RuntimeError about a C++ type named '?'.
    */
    template <typename T>
    bool
    loadElement(handle obj, T &out)
    {
      try {
	out = obj.template cast<T>();
      }
      catch (const cast_error &) {
	return false;
      }
      return true;
    }

    // cv::Point <=> tuple (x, y)
    template<>
    struct type_caster<cv::Point>{
      PYBIND11_TYPE_CASTER(cv::Point, const_name("tuple_xy"));

      bool load(handle obj, bool){
	if(!py::isinstance<py::tuple>(obj)){
	  return false; //pybind11 reports the expected type, cf const_name() above
        }

	py::tuple pt = reinterpret_borrow<py::tuple>(obj);
        if(pt.size()!=2){
	  return false;
        }

	int x = 0;
	int y = 0;
	if (! loadElement(pt[0], x) || ! loadElement(pt[1], y)) {
	  return false;
	}

	value = cv::Point(x, y);
        return true;
      }

      static handle cast(const cv::Point& pt, return_value_policy, handle){
        return py::make_tuple(pt.x, pt.y).release();
      }
    };

    // cv::Rect <=> tuple (x, y, w, h)
    template<>
    struct type_caster<cv::Rect>{
      PYBIND11_TYPE_CASTER(cv::Rect, const_name("tuple_xywh"));
      
      bool load(handle obj, bool){
        if(!py::isinstance<py::tuple>(obj)){
	  return false; //pybind11 reports the expected type, cf const_name() above
        }
        py::tuple rect = reinterpret_borrow<py::tuple>(obj);
        if(rect.size()!=4){
	  return false;
        }
	
	int v[4] = { 0, 0, 0, 0 };
	for (int i = 0; i < 4; ++i) {
	  if (! loadElement(rect[i], v[i])) {
	    return false;
	  }
	}

        value = cv::Rect(v[0], v[1], v[2], v[3]);
        return true;
      }

      static handle cast(const cv::Rect& rect, return_value_policy, handle){
        return py::make_tuple(rect.x, rect.y, rect.width, rect.height).release();
      }
    };


    // cv::Scalar <=> tuple (x), (y,y), (x,y,z), (x,y,z,w)
    //B:TODO: we handle only doubles ! Should we handle ints ???
    //B:TODO: In OpenCV python wrapper, they can use a single int as a Scalar for a grayscale image
    template<>
    struct type_caster<cv::Scalar>{
      PYBIND11_TYPE_CASTER(cv::Scalar, const_name("tuple_x_y_z_w"));

      bool load(handle obj, bool){
	if(!py::isinstance<py::tuple>(obj)){
	  return false; //pybind11 reports the expected type, cf const_name() above
        }

	py::tuple sc = reinterpret_borrow<py::tuple>(obj);
        if(sc.size()<1 || sc.size()>4){
	  //B: a tuple of size 0 used to match no branch below and to be
	  // silently converted to cv::Scalar(), that is (0, 0, 0, 0).
	  return false;
        }

	double v[4] = { 0., 0., 0., 0. };
	for (size_t i = 0; i < sc.size(); ++i) {
	  if (! loadElement(sc[i], v[i])) {
	    return false;
	  }
	}

	value = cv::Scalar(v[0], v[1], v[2], v[3]);

        return true;
      }

      static handle cast(const cv::Scalar& sc, return_value_policy, handle){
        return py::make_tuple(sc[0], sc[1], sc[2], sc[3]).release();
      }
    };



    
    
  }} //!  end namespace pybind11::detail

	

#endif /* ! OPENCV_CONVERTER_HPP */
