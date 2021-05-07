#ifndef OPENCV_CONVERTER_HPP
#define OPENCV_CONVERTER_HPP

//inspired from https://github.com/pybind/pybind11/issues/2004

#include <opencv2/core/core.hpp>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace pybind11 { namespace detail{

    // cv::Point <=> tuple (x, y)
    template<>
    struct type_caster<cv::Point>{
      PYBIND11_TYPE_CASTER(cv::Point, _("tuple_xy"));

      bool load(handle obj, bool){
	if(!py::isinstance<py::tuple>(obj)){
	  std::logic_error("Point(x,y) should be a tuple!");
	  return false;
        }

	py::tuple pt = reinterpret_borrow<py::tuple>(obj);
        if(pt.size()!=2){
	  std::logic_error("Point(x,y) tuple should be size of 2");
	  return false;
        }

	value = cv::Point(pt[0].cast<int>(), pt[1].cast<int>());
        return true;
      }

      static handle cast(const cv::Point& pt, return_value_policy, handle){
        return py::make_tuple(pt.x, pt.y).release();
      }
    };

    // cv::Rect <=> tuple (x, y, w, h)
    template<>
    struct type_caster<cv::Rect>{
      PYBIND11_TYPE_CASTER(cv::Rect, _("tuple_xywh"));
      
      bool load(handle obj, bool){
        if(!py::isinstance<py::tuple>(obj)){
	  std::logic_error("Rect should be a tuple!");
	  return false;
        }
        py::tuple rect = reinterpret_borrow<py::tuple>(obj);
        if(rect.size()!=4){
	  std::logic_error("Rect (x,y,w,h) tuple should be size of 4");
	  return false;
        }
	
        value = cv::Rect(rect[0].cast<int>(), rect[1].cast<int>(), rect[2].cast<int>(), rect[3].cast<int>());
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
      PYBIND11_TYPE_CASTER(cv::Scalar, _("tuple_x_y_z_w"));

      bool load(handle obj, bool){
	if(!py::isinstance<py::tuple>(obj)){
	  std::logic_error("Scalar(x[,y[,z[,w]]]) should be a tuple!");
	  return false;
        }

	py::tuple sc = reinterpret_borrow<py::tuple>(obj);
        if(sc.size()>4){
	  std::logic_error("Scalar(x[,y[,z[,w]]]) tuple should be size of 1, 2, 3 or 4.");
	  return false;
        }

	if (sc.size() == 1) {
	  value = cv::Scalar(sc[0].cast<double>());
	}
	else if (sc.size() == 2) {
	  value = cv::Scalar(sc[0].cast<double>(), sc[1].cast<double>());
	}
	else if (sc.size() == 3) {
	  value = cv::Scalar(sc[0].cast<double>(), sc[1].cast<double>(), sc[2].cast<double>());
	}
	else if (sc.size() == 4) {
	  value = cv::Scalar(sc[0].cast<double>(), sc[1].cast<double>(), sc[2].cast<double>(), sc[3].cast<double>());
	}
	
        return true;
      }

      static handle cast(const cv::Scalar& sc, return_value_policy, handle){
        return py::make_tuple(sc[0], sc[1], sc[2], sc[3]).release();
      }
    };



    
    
  }} //!  end namespace pybind11::detail

	

#endif /* ! OPENCV_CONVERTER_HPP */
