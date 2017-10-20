#ifndef CURRENTRANDOMDOCUMENTCONTEXT_HPP
#define CURRENTRANDOMDOCUMENTCONTEXT_HPP

#include <QImage>
#include <framework_global.h>
#include <patterns/singleton.h>

namespace Context {

class CurrentRandomDocumentContext;

class CurrentRandomDocumentContext
  : public Patterns::Singleton<CurrentRandomDocumentContext>
{
public:
  CurrentRandomDocumentContext() {}

  virtual ~CurrentRandomDocumentContext() {}

  const QImage &getImage() const { return _image; }

  QMap<QString, QString> getProperties() const { return _properties; }

  void setImage(const QImage &img) { _image = img; }

  void addProperty(const QString &name, const QString &value)
  {
    _properties.insert(name, value);
  }

protected:
  QImage _image;
  QMap<QString, QString> _properties;
};
}

#endif // CURRENTRANDOMDOCUMENTCONTEXT_HPP
