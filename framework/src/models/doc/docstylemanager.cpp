#include "docstylemanager.h"

#include "docstyle.h"


namespace Doc
{

  DocStyleManager::~DocStyleManager()
  {
    HashType::iterator it = _styles.begin();
    const HashType::iterator itEnd = _styles.end();
    for ( ; it != itEnd; ++it) {
      delete it.value()->style();
      delete it.value();
    }
    
  }

    QString DocStyleManager::addReference(DocStyle* value)
    {
      //B:TODO:API is it necessary to return a QString (already available from passed parameter)

      if (value == nullptr) {
	return QString();
      }

      QString key = value->getName();

      HashType::iterator it = _styles.find(key);
      if (it == _styles.end()) {
	it = _styles.insert(key, new styleReferenceCounter(value));
      }
      styleReferenceCounter* ref = it.value();

      ref->add();
      return key;
    }

    void DocStyleManager::removeReference(DocStyle* old)
    {
      if (old == nullptr) {
            return;
      }

        removeReference(old->getName());
    }

    void DocStyleManager::removeReference(const QString &key)
    {
      HashType::iterator it = _styles.find(key);
      if (it == _styles.end()) {
	return;
      }
      styleReferenceCounter* ref = it.value();

      ref->remove();
      
      //B:TODO: keys never removed from _styles ???
      // We can have styleReferenceCounter with negative counter...
    }

    DocStyle* DocStyleManager::getStyle(const QString &key)
    {
      HashType::iterator it = _styles.find(key);
      if (it == _styles.end()) {
	return nullptr;
      }
      styleReferenceCounter* ref = it.value();

      return ref->style();
    }

    QList<DocStyle*> DocStyleManager::getStyles()
    {
        QList<DocStyle*> list;
	const auto &styles = _styles;
	list.reserve(styles.size());
	auto it = styles.begin();
	const auto itEnd = styles.end();
	for ( ; it != itEnd; ++it) {
	  styleReferenceCounter* ref = it.value();
	  if (ref->counter() > 0) {
	    list.append(ref->style());
	  }
        }
        return list;
    }

} //namespace Doc
