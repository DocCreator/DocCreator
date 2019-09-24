#ifndef DOCSTYLEMANAGER_H
#define DOCSTYLEMANAGER_H

#include <framework_global.h>

#include <QHash>


namespace Doc
{
  class DocStyle;

  struct styleReferenceCounter //B:TODO: remove ! use shared_ptrs !!!
  {
  public:
    
    explicit styleReferenceCounter(Doc::DocStyle* s) :
      _style(s),
      _counter(0)
    {}

    Doc::DocStyle* style() { return _style; }

    int counter() const { return _counter; }
    
    void add() { _counter = _counter + 1; }
    
    void remove() { _counter = _counter - 1; }

  private:

    Doc::DocStyle* _style;
    int _counter;
  };

  class FRAMEWORK_EXPORT DocStyleManager
  {
  public:
    DocStyleManager() = default;
    
    ~DocStyleManager(); //B
    
    QString addReference(Doc::DocStyle* value);
    void removeReference(Doc::DocStyle* old);
    void removeReference(const QString &key);
    
    Doc::DocStyle* getStyle(const QString &key);
    QList<Doc::DocStyle*> getStyles();
    
  private:
    using HashType = QHash<QString, styleReferenceCounter*>;
    HashType _styles;
  };

}

#endif // DOCSTYLEMANAGER_H
