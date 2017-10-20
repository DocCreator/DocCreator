#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H

#include <QString>
#include <QVariant>
#include <framework_global.h>
class QObject;
class QSettings;

namespace Core {
class FRAMEWORK_EXPORT ConfigurationManager
{
public:
  static void initialize(const QString &file, QObject *parent = 0);
  static QVariant get(const QString &group, const QString &key);
  static void set(const QString &group,
                  const QString &key,
                  const QVariant &value);

private:
  static QSettings *_settings;
};
}

#endif // CONFIGURATIONMANAGER_H
