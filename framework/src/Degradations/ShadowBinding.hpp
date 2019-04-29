#ifndef SHADOWBINDING_HPP
#define SHADOWBINDING_HPP

#include "DocumentDegradation.hpp"
#include <framework_global.h>
#include <QImage>
#include <opencv2/core/core.hpp>

namespace dc {

/**
   Implements "non-linear illumination model" from :
   "Global and Local Document Degradation Models"
   Tapas Kanungo, Robert M. Haralick and Thsin Phillips
   ICDAR 1993
*/

enum class ShadowBorder {TOP=0, RIGHT, BOTTOM, LEFT};

/*
  @param[in] distance   size in pixels of degradation.
  @param[in] intensity intinsity in [0; 1]
  @param[in] angle angle in degrees in [0; 90]
 */
extern FRAMEWORK_EXPORT QImage shadowBinding(const QImage &imgOriginal, ShadowBorder border, int distance, float intensity, float angle);

/*
  @param[in] distance   size in pixels of degradation.
  @param[in] intensity intinsity in [0; 1]
  @param[in] angle angle in degrees in [0; 90]
 */
extern FRAMEWORK_EXPORT void shadowBinding(cv::Mat &matOut, ShadowBorder border, int distance, float intensity, float angle);

/*
  @param[in] distanceRatio   ratio of width or height used to compute size in pixels of degradation, in [0; 1]
  @param[in] intensity intinsity in [0; 1]
  @param[in] angle angle in degrees in [0; 90]
 */
extern FRAMEWORK_EXPORT QImage shadowBinding(const QImage &imgOriginal, float distanceRatio, ShadowBorder border, float intensity, float angle);

/*
  @param[in] distanceRatio   ratio of width or height used to compute size in pixels of degradation, in [0; 1].
  @param[in] intensity intinsity in [0; 1]
  @param[in] angle angle in degrees in [0; 90]
 */
extern FRAMEWORK_EXPORT void shadowBinding(cv::Mat &matOut, float distanceRatio, ShadowBorder border, float intensity, float angle);



class FRAMEWORK_EXPORT ShadowBinding : public DocumentDegradation
{
  Q_OBJECT

public : 

  /**
     
     @param[in] distance width in pixels on which degradation is applied.
     @param[in] intensity intinsity in [0; 1]
     @param[in] angle angle in degrees in [0; 90]
   */
  explicit ShadowBinding(const QImage &original, ShadowBorder border, int distance, float intensity=0.5, float angle=30, QObject *parent=0) :
    DocumentDegradation(parent), 
    _border(border), 
    _distance(distance), 
    _intensity(intensity), 
    _angle(angle),
    _original(original)
  {}

  /**
     
     @param[in] distanceRatio ratio of width or height used to compute size in pixels of degradation, in [0; 1].
     @param[in] intensity intinsity in [0; 1]
     @param[in] angle angle in degrees in [0; 90]
   */
  explicit ShadowBinding(const QImage &original, float distanceRatio, ShadowBorder border, float intensity=0.5, float angle=30, QObject *parent=0);


public slots :

  virtual QImage apply() override;

signals:

  void imageReady(const QImage &);

protected :
  ShadowBorder _border;
  int _distance;
  float _intensity;
  float _angle;
  QImage _original;

};

} //namespace dc
  
#endif //SHADOWBINDING_HPP
