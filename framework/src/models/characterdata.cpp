#include "characterdata.h"

namespace Models {
CharacterData::CharacterData(const QImage &image,
                             int id,
                             float degradationLevel)
  : _image(image)
  , _id(id)
  , _degradationLevel(degradationLevel)
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
