#ifndef DOCSTYLE_H
#define DOCSTYLE_H

#include <QString>
#include <framework_global.h>

namespace Doc
{
    class FRAMEWORK_EXPORT DocStyle
    {
    public:
        explicit DocStyle(const QString &name);
	DocStyle(const QString &name, const QString &fontName);
	
	//B: I don't really undestand why a style has two names ???
	// Is it to have a display name and a real name ???
	// It seems that we always use them with two same names !

        DocStyle* clone() const;

        QString getName() const { return _name; }
        QString getFontName() const { return _fontName; }

	//B: why there is no setName() ???

        void setFontName(const QString &fontName) { _fontName = fontName; }

    private:
        QString _name;
        QString _fontName;
    };
}

#endif // DOCSTYLE_H
