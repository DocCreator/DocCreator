#include "configurationmanager.h"

#include <QSettings>

namespace Core {
QSettings *ConfigurationManager::_settings;

void
ConfigurationManager::initialize(const QString &file, QObject *parent)
{
  _settings = new QSettings(file, QSettings::IniFormat, parent);
}

QVariant
ConfigurationManager::get(const QString &group, const QString &key)
{
  QVariant value;
  QString realKey = group;
  realKey.append('/').append(key);
  if (_settings != nullptr && _settings->contains(realKey)) {
    value = _settings->value(realKey);
  }
  return value;
}

void
ConfigurationManager::set(const QString &group,
                          const QString &key,
                          const QVariant &value)
{
  QString realKey = group;
  realKey.append('/').append(key);
  if (_settings != nullptr) {
    _settings->setValue(realKey, value);
  }
}
} //namespace Core
