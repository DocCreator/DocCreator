#ifndef ROTATIONDEGRADATIONQ_HPP
#define ROTATIONDEGRADATIONQ_HPP

#include <QImage>
#include "Degradations/DocumentDegradation.hpp"
#include "Degradations/RotationDegradation.hpp"

namespace dc {

  namespace RotationDegradation {

    extern FRAMEWORK_EXPORT QImage rotateFillColor(const QImage &img,
						    float angle,
						    const QColor &color);

    extern FRAMEWORK_EXPORT QImage rotateFillImage(const QImage &img,
						    float angle,
						    const QImage &backgroundImg);

    extern FRAMEWORK_EXPORT QImage rotateFillImage(const QImage &img,
						    float angle,
						    const QImage &backgroundImg,
						    int repeats);

    extern FRAMEWORK_EXPORT QImage rotateFillImage(const QImage &img,
						    float angle,
						    const std::vector<QImage> &backgroundImgs);
    
    extern FRAMEWORK_EXPORT QImage rotateFillBorder(const QImage &img,
						     float angle,
						     BorderReplication borderMode = BorderReplication::WRAP);

    extern FRAMEWORK_EXPORT QImage rotateFillInpaint1(const QImage &img,
						       float angle,
						       float inpaintingRatio = 0.05);

    extern FRAMEWORK_EXPORT QImage rotateFillInpaint2(const QImage &img,
						       float angle,
						       const QImage &backgroundImg = QImage());

    extern FRAMEWORK_EXPORT QImage rotateFillInpaint3(const QImage &img,
						       float angle,
						       const QImage &backgroundImg = QImage());
  } //namespace ShadowBinding
  

} //namespace dc


#endif /* ! ROTATIONDEGRADATIONQ_HPP */
