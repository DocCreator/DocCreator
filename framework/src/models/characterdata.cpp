#include "characterdata.h"

namespace Models {
CharacterData::CharacterData(const QImage &image,
                             int id)
  : _image(image)
  , _id(id)
{}

void
CharacterData::setImage(const QImage &image)
{
  _image = image;
}

QImage
CharacterData::getImage() const
{
  return _image;
}

} //namespace Models
