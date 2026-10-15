/*
  Copyright (C) 2011-2012 Van Cuong KIEU, Nicholas JOURNET, van-cuong.kieu@labri.fr
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef DOCCOMPONENTBLOCK_H
#define DOCCOMPONENTBLOCK_H


#include <QList>
#include <framework_global.h>

#include "block.h"
#include "doczone.h"
#include "nodeofnodes.h"

namespace Doc
{
/**
   A block of components of a document image, not recognized yet, grouped in
   zones (cf. DocZone).
*/
class FRAMEWORK_EXPORT DocComponentBlock : public NodeOfNodes<DocZone>, public Block
{
public:
    explicit DocComponentBlock(Document* document);
    DocComponentBlock(Document* document, int w, int h, int x, int y);

    DocComponentBlock* clone() override;

    DocComponentBlock* getSelection() override;

    void add(DocComponent* e);
    void add(const QList<DocComponent*> &l);
    void add(DocZone *e) override;
    void add(const QList<DocZone*> &l) override;

    DocZone* currentZone();
    const QList<DocZone*> &getZones() const;

    QString content() const override;

protected:
    void actionWhenElementIsEmpty(DocZone* empty) override;
    void removeBeforeAndCurrentAtBeginning() override;
    void removeAfterAndCurrentAtEnd() override;

private:

};
}
#endif // DOCCOMPONENTBLOCK_H
