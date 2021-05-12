#ifndef CHARACTERDATA_H
#define CHARACTERDATA_H

#include <QImage>
#include <framework_global.h>

namespace Models {
class FRAMEWORK_EXPORT CharacterData
{
public:
  CharacterData(const QImage &image, int id);

  /* Setters */
  void setImage(const QImage &image);

  /* Getters */
  QImage getImage() const;
  int getId() const { return _id; }

  int width() const { return _image.width(); }
  int height() const { return _image.height(); }

private:
  QImage _image;
  int _id;
};

} // Models

#endif // CHARACTERDATA_H
