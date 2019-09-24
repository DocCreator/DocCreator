#ifndef BACKGROUNDCONTEXT_H
#define BACKGROUNDCONTEXT_H

#include <QList>
#include <QString>
#include <framework_global.h>
#include <patterns/observable.h>
#include <patterns/singleton.h>

namespace Context {

using BackgroundList = QList<QString>;

class FRAMEWORK_EXPORT BackgroundContext
  : public Patterns::Observable
  , public Patterns::Singleton<BackgroundContext>
{
public:
  static void initialize(const QString &path);
  static BackgroundContext *instance();

  BackgroundContext();

  QString getCurrentBackground() const;
  QString getPath() const;
  bool changed() const;
  void setChanged(bool changed);
  void setCurrentBackground(const QString &backgroundName);
  BackgroundList getBackgrounds() const;

  void clear();

private:
  void addBackground(const QString &background);
  void setPath(const QString &path);

private:
  static BackgroundContext *_instance;
  bool _changed;
  QString _path;
  QString _currentBackground;
  BackgroundList _backgroundList;
};
}

#endif // BACKGROUNDCONTEXT_H
