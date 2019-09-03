#ifndef CHARACTERSDEGRADATIONQ_HPP
#define CHARACTERSDEGRADATIONQ_HPP

#include <QImage>
#include <QObject>

#include "Degradations/GrayscaleCharsDegradationModel.hpp"


namespace dc {

  /*
  namespace CharactersDegradation {



  } //namespace CharactersDegradation
  */

  class FRAMEWORK_EXPORT GrayscaleCharsDegradationModelQ
  {
  public:
    explicit GrayscaleCharsDegradationModelQ(const QImage &img);

    QImage degradate(int level = 1, float I = 33, float O = 33, float D = 34);
    QImage degradateByLevel(int level);
  

  private:
  //void initialize(const QImage &input);

  private:
    GrayscaleCharsDegradationModel _cdm;
  };


} //namespace dc


#endif /* ! CHARACTERSDEGRADATIONQ_HPP */
