#include "fontcontext.h"

#include <QDebug>
#include <QDir>
#include <cassert>
#include <iomanager/fontfilemanager.h>
#include <models/font.h>

namespace Context {

FontContext *FontContext::_instance;

FontContext *
FontContext::instance()
{
  if (_instance == nullptr)
    _instance = new FontContext();
  return _instance;
}

FontContext::FontContext()
  : _currentFont(nullptr)
  , _fontMap()
  , _fontChanged(false)
  , _currentFontModified(false)

{}

void
FontContext::initialize(const QString &path, const QString &fileExtension)
{
  FontContext *inst = instance();
  inst->setVars(path, fileExtension);
  QDir directory(path);

  //std::cout<<"########## FontContext::initialize(path=+"<<path.toStdString()<<"+ fileExtension=+"<<fileExtension.toStdString()<<"+\n";
  //std::cout<<"########## FontContext::initialize() dir="<<directory.absolutePath().toStdString()<<"\n";

  QStringList fontFiles;
  QStringList fileList = directory.entryList(QDir::Files);
  for (const QString &file : fileList) {
    if (!file.endsWith(fileExtension))
      continue;
    fontFiles.push_back(directory.absoluteFilePath(file));
  }

  //B:TODO: Can we do :
  // QStringList fontFiles = directory.entryList(QDir::Files, fileExtension);
  //???? We would not have absolute paths ?

  //B: Should we call clear() ? Should we notify observers ?
  //clear(); //will delete previous fonts if any.

  addFontsFromXmlFiles(fontFiles);

  assert(checkInvariant());
}

Models::Font *
FontContext::getCurrentFont()
{
  return _currentFont;
}

//B : may call notifyAll() via setCurrentFont()
void
FontContext::addFontsFromXmlFiles(const QStringList &fontFiles)
{
  //B: Is it ok to have this method here ?
  //B: If called directly, it will not call setVars()...

  FontContext *inst = instance();
  for (const QString &file : fontFiles) {
    Models::Font *font = IOManager::FontFileManager::fontFromXml(file);
    if (font != nullptr)
      inst->addFont(font);
  }

  assert(checkInvariant());
}

void
FontContext::clear()
{
  FontMap::iterator it = _fontMap.begin();
  const FontMap::iterator itEnd = _fontMap.end();
  for (; it != itEnd; ++it) {
    delete it
      .value(); //B: should we notify observers befeore deletion ???? Or should we use shared_ptr ???
  }
  _fontMap.clear();
  _currentFont = nullptr;

  _fontChanged = true;
  _currentFontModified = true;
  notifyAll();
  _fontChanged = false;
  _currentFontModified = false;

  assert(checkInvariant());
}

void
FontContext::addFont(Models::Font *font)
{
  assert(font);
  FontMap::iterator it = _fontMap.find(font->getName());
  if (it != _fontMap.end()) {
    const bool mustChangeCurrentFont = (_currentFont == it.value());
    delete it
      .value(); //B: should we notify observers before deletion ???? Or should we use shared_ptr ???
    it.value() = font;
    if (mustChangeCurrentFont) {
      _currentFont = nullptr;
      setCurrentFont(
        font->getName()); //B: can only be called once the font has been added.
    }
  } else {
    _fontMap.insert(font->getName(), font);
    //B:TODO: we should notify !?
  }
  assert(checkInvariant());
}

Models::Font *
FontContext::removeFont(const QString &fontName)
{
  FontMap::iterator it = _fontMap.find(fontName);
  if (it != _fontMap.end()) {
    _currentFontModified = false;
    if (_currentFont != nullptr && fontName == _currentFont->getName()) {
      _currentFontModified = true;
    }
    _fontChanged = true;

    Models::Font *f = it.value();
    _fontMap.erase(it);

    if (_currentFontModified) {
      if (!_fontMap.isEmpty())
        _currentFont = _fontMap.begin().value();
      else
        _currentFont = nullptr;
    }

    notifyAll();
    _fontChanged = false;
    _currentFontModified = false;

    assert(checkInvariant());

    return f;
  }
  return nullptr;
}

void
FontContext::setCurrentFont(const QString &fontName)
{
  if (_currentFont != nullptr && fontName == _currentFont->getName()) {
    _currentFontModified = true;
  } else {
    Q_ASSERT(!_fontMap.isEmpty());
    _currentFont = _fontMap[fontName];
  }
  _fontChanged = true;
  notifyAll();
  _fontChanged = false;
  _currentFontModified = false;

  assert(checkInvariant());
}

bool
FontContext::checkInvariant() const
{
  //all font pointers are not null
  for (FontMap::const_iterator it = _fontMap.begin(); it != _fontMap.end();
       ++it) {
    if (it.value() == nullptr) {
      qDebug() << "FontContext::checkInvariant: font: " << it.key()
               << " has nullptr font";
      return false;
    }
  }

  //_currentFont is either nullptr or a font in the map.
  if (_currentFont != nullptr) {
    FontMap::const_iterator it = _fontMap.find(_currentFont->getName());
    if (it == _fontMap.end()) {
      qDebug() << "FontContext::checkInvariant: current font " << _currentFont
               << " name=" << _currentFont->getName() << " not found in map";
      return false;
    }
  }

  //TODO: check that all fonts name are not empty

  return true;
}

Models::Font *
FontContext::getFont(const QString &fontName)
{
  FontMap::iterator it = _fontMap.find(fontName);
  if (it != _fontMap.end())
    return it.value();
  return nullptr;
}

QStringList
FontContext::getFontNames() const
{
  return _fontMap.keys();
}

int
FontContext::getNumberOfFonts() const
{
  return _fontMap.count();
}

//B
QString
FontContext::getCurrentFontName() const
{
  //std::cerr<<"############# FontContext::getCurrentFontName() _currentFont=+"<<_currentFont<<" name=+"<<(_currentFont != nullptr ? _currentFont->getName() : QString()).toStdString()<<"+\n";

  return (_currentFont != nullptr ? _currentFont->getName() : QString());
}

bool
FontContext::fontHasChanged() const
{
  return _fontChanged;
}

bool
FontContext::fontCurrentModified() const
{
  return _currentFontModified;
}

void
FontContext::setVars(const QString &path, const QString &fileExtension)
{
  _fontDir = path;
  _fileExtension = fileExtension;
}

QString
FontContext::currentFontPath() const
{
  assert(_currentFont);
  return QDir(_fontDir).absoluteFilePath(_currentFont->getName() +
                                         _fileExtension);
}

} //namespace Context
