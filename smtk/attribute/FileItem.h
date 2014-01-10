/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME FileItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_FileItem_h
#define __smtk_attribute_FileItem_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <string>
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class FileItemDefinition;
    class SMTKCORE_EXPORT FileItem : public Item
    {
    friend class FileItemDefinition;
    public:
      virtual ~FileItem();
      virtual Item::Type type() const;
      bool shouldBeRelative() const;
      bool shouldExist() const;
      std::size_t numberOfValues() const
      {return this->m_values.size();}
      bool  setNumberOfValues(std::size_t newSize);
      std::size_t numberOfRequiredValues() const;
      std::string value(int element=0) const
      {return this->m_values[element];}
      bool setValue(const std::string &val)
      {return this->setValue(0, val);}
      bool setValue(int element, const std::string &val);
      bool appendValue(const std::string &val);
      bool removeValue(int element);
      virtual void reset();
      virtual std::string valueAsString(const std::string &format="") const
      {return this->valueAsString(0, format);}
      virtual std::string valueAsString(int element, const std::string &format="") const;
      virtual bool isSet(std::size_t element=0) const
      {return this->m_isSet[element];}
      virtual void unset(std::size_t element=0)
      {this->m_isSet[element] = false;}

    protected:
      FileItem(Attribute *owningAttribute, int itemPosition);
      FileItem(Item *owningItem, int position, int subGroupPosition);
      virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);
      std::vector<std::string>m_values;
      std::vector<bool> m_isSet;
    private:
    };
  }
}


#endif /* __smtk_attribute_FileItem_h */
