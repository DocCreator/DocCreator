#ifndef SHADOWBINDINGQ_HPP
#define SHADOWBINDINGQ_HPP

#include <QImage>
#include "Degradations/DocumentDegradation.hpp"
#include "Degradations/ShadowBinding.hpp"

namespace dc {

  namespace ShadowBinding {

    /*
      @param[in] distance   size in pixels of degradation.
      @param[in] intensity intinsity in [0; 1]
      @param[in] angle angle in degrees in [0; 90]
    */
    extern FRAMEWORK_EXPORT QImage shadowBinding(const QImage &imgOriginal, dc::ShadowBinding::Border border, int distance, float intensity, float angle);


    /*
      @param[in] distanceRatio   ratio of width or height used to compute size in pixels of degradation, in [0; 1]
      @param[in] intensity intinsity in [0; 1]
      @param[in] angle angle in degrees in [0; 90]
    */
    extern FRAMEWORK_EXPORT QImage shadowBinding2(const QImage &imgOriginal, float distanceRatio, dc::ShadowBinding::Border border, float intensity, float angle);

    
  } //namespace ShadowBinding
  
  class FRAMEWORK_EXPORT ShadowBindingQ : public DocumentDegradation
  {
    Q_OBJECT

  public : 

    /**
     
       @param[in] distance width in pixels on which degradation is applied.
       @param[in] intensity intinsity in [0; 1]
       @param[in] angle angle in degrees in [0; 90]
    */
    explicit ShadowBindingQ(const QImage &original, dc::ShadowBinding::Border border, int distance, float intensity=0.5, float angle=30, QObject *parent=0) :
      DocumentDegradation(parent), 
      _border(border), 
      _distance(distance), 
      _intensity(intensity), 
      _angle(angle),
      _original(original)
      {}



  public slots :

    QImage apply() override;

  signals:

    void imageReady(const QImage &);

  protected :
    dc::ShadowBinding::Border _border;
    int _distance;
    float _intensity;
    float _angle;
    QImage _original;

  };


} //namespace dc


#endif /* ! SHADOWBINDINGQ_HPP */
