#ifndef CHARACTERDATA_H
#define CHARACTERDATA_H

#include <QImage>
#include <framework_global.h>

namespace Models {
class FRAMEWORK_EXPORT CharacterData
{
public:
  CharacterData(const QImage &image, int id, float degradationLevel = 0);

  /* Setters */
  void setImage(const QImage &image);
  void setDegradationLevel(float level) { _degradationLevel = level; }

  /* Getters */
  QImage getImage() const;
  int getId() const { return _id; }
  float getDegradationLevel() const { return _degradationLevel; }

  int width() const { return _image.width(); }
  int height() const { return _image.height(); }

private:
  QImage _image;
  int _id;
  float _degradationLevel;
};

} // Models

#endif // CHARACTERDATA_H
