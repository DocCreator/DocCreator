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

#ifndef DOCZONE_H
#define DOCZONE_H

#include <QList>
#include <framework_global.h>
#include "doccomponent.h"
#include "nodeofleafs.h"

/**
  * \mainpage
  * This class header describe a list of component in document image such as WORD, LINE,TABLE
  *
  */
namespace Doc
{
class FRAMEWORK_EXPORT DocZone : public NodeOfLeafs<DocComponent>
{
public:
    explicit DocZone(Document* document);

    virtual DocZone* clone() override;

    virtual DocZone* getSelection() override;

    DocComponent* currentComponent();

    QList<DocComponent*> getComponents();

    QList<DocZone*> splitAtPosition(int position);
};
}
#endif // DOCZONE_H
