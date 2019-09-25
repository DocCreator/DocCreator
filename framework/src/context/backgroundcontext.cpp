#include "backgroundcontext.h"

#include <QDir>
#include <QStringList>

#include "Utils/ImageUtils.hpp" //getReadImageFilterList

namespace Context {

BackgroundContext *BackgroundContext::_instance;

BackgroundContext *
BackgroundContext::instance()
{
  if (_instance == nullptr) {
    _instance = new BackgroundContext();
  }
  return _instance;
}

void
BackgroundContext::initialize(const QString &path)
{
  BackgroundContext *inst = instance();
  inst->setPath(path);
  QDir directory(path);
  const QStringList nameFilters = getReadImageFilterList();
  QList<QString> fileList =
    directory.entryList(nameFilters, QDir::Files | QDir::Readable);
  for (const QString &file : fileList) {
    inst->addBackground(directory.relativeFilePath(file));
  }
}

BackgroundContext::BackgroundContext()
  : _changed(false)
{}

QString
BackgroundContext::getCurrentBackground() const
{
  return _currentBackground;
}

QString
BackgroundContext::getPath() const
{
  return _path;
}

bool
BackgroundContext::changed() const
{
  return _changed;
}

void
BackgroundContext::setChanged(bool changed)
{
  _changed = changed;
}

void
BackgroundContext::addBackground(const QString &background)
{
  _backgroundList << background;
}

void
BackgroundContext::setCurrentBackground(const QString &backgroundName)
{
  //B: not added to _backgroundList ???
  //B: If it was, it would be better to store _currentBackground as an index in _backgroundList...

  _currentBackground = backgroundName;
  _changed = true;
  notifyAll();
}

void
BackgroundContext::setPath(const QString &path)
{
  _path = path;
  if (!_path.isEmpty() && _path[_path.length() - 1] != '/') {
    _path.push_back('/');
  }
}

BackgroundList
BackgroundContext::getBackgrounds() const
{
  return _backgroundList;
}

void
BackgroundContext::clear()
{
  _backgroundList.clear();
  _currentBackground = QLatin1String("");
}
} //namespace Context
