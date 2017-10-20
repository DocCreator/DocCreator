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

/**
  * \mainpage
  * This class header describe a block of unknown elements of Document Image. At the firt time, the document image is loaded.
  * All components are extracted but they are still unknown elements called component element.
  *
  */
namespace Doc
{
class FRAMEWORK_EXPORT DocComponentBlock : public NodeOfNodes<DocZone>, public Block
{
public:
    explicit DocComponentBlock(Document* document);
    DocComponentBlock(Document* document, int w, int h, int x, int y);

    virtual DocComponentBlock* clone() override;

    virtual DocComponentBlock* getSelection() override;

    void add(DocComponent* e);
    void add(const QList<DocComponent*> &l);
    virtual void add(DocZone *e) override;
    virtual void add(const QList<DocZone*> &l) override;

    DocZone* currentZone();
    QList<DocZone*> getZones();

    virtual QString content() const override;

protected:
    virtual void actionWhenElementIsEmpty(DocZone* empty) override;
    virtual void removeBeforeAndCurrentAtBeginning() override;
    virtual void removeAfterAndCurrentAtEnd() override;

private:

};
}
#endif // DOCCOMPONENTBLOCK_H
