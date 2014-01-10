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


#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"

using namespace smtk::attribute; 

//----------------------------------------------------------------------------
ValueItem::ValueItem(Attribute *owningAttribute, 
                     int itemPosition): 
  Item(owningAttribute, itemPosition)
{
}

//----------------------------------------------------------------------------
ValueItem::ValueItem(Item *inOwningItem,
                     int itemPosition,
                     int mySubGroupPosition): 
  Item(inOwningItem, itemPosition, mySubGroupPosition)
{
}
//----------------------------------------------------------------------------
bool ValueItem::setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef)
{
   // Note that we do a dynamic cast here since we don't
  // know if the proper definition is being passed
  const ValueItemDefinition *def = 
    dynamic_cast<const ValueItemDefinition *>(vdef.get());
  // Call the parent's set definition - similar to constructor calls
  // we call from base to derived
  if (!def || (!Item::setDefinition(vdef)))
    {
    return false;
    }

  // Build the item's children
  def->buildChildrenItems(this);

  // Find out how many values this item is suppose to have
  // if the size is 0 then its unbounded
  std::size_t n = def->numberOfRequiredValues();
  if (n)
    {
    if (def->hasDefault())
      {
      this->m_isSet.resize(n, true);
      }
    else
      {
      this->m_isSet.resize(n, false);
      }
    if (def->isDiscrete())
      {
      this->m_discreteIndices.resize(n, def->defaultDiscreteIndex());
      this->updateActiveChildrenItems();
      }
    if (def->allowsExpressions())
      {
      int i;
      this->m_expressions.resize(n);
      for (i = 0; i < static_cast<int>(n); i++)
        {
        def->buildExpressionItem(this, i);
        }
      }
    }

  return true;
}
//----------------------------------------------------------------------------
ValueItem::~ValueItem()
{
  // we need to detach all items that are owned by this. i.e. the expression items
  std::size_t i, n = m_expressions.size();
  for (i = 0; i < n; i++)
    {
    this->m_expressions[i]->detachOwningItem();
    }
}
//----------------------------------------------------------------------------
std::size_t ValueItem::numberOfRequiredValues() const
{
  const ValueItemDefinition *def = 
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return 0;
    }
  return def->numberOfRequiredValues();
}
//----------------------------------------------------------------------------
bool ValueItem::allowsExpressions() const
{
  const ValueItemDefinition *def = 
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (!def)
    {
    return false;
    }
  return def->allowsExpressions();
}
//----------------------------------------------------------------------------
smtk::attribute::AttributePtr ValueItem::expression(int element) const
{
  const ValueItemDefinition *def = 
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (def->allowsExpressions())
    {
    return this->m_expressions[element]->value();
    }
  return smtk::attribute::AttributePtr();
}
//----------------------------------------------------------------------------
bool ValueItem::setExpression(int element, smtk::attribute::AttributePtr exp)
{
  const ValueItemDefinition *def = 
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (def->allowsExpressions())
    {
    if (!exp)
      {
      if (this->m_expressions[element]->value())
        {
        this->m_isSet[element] = false;
        this->m_expressions[element]->unset();
        }
      return true;
      }
    if (def->isValidExpression(exp))
      {
      this->m_isSet[element] = true;
      this->m_expressions[element]->setValue(exp);
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool ValueItem::appendExpression(smtk::attribute::AttributePtr exp)
{
  const ValueItemDefinition *def = 
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (!def->allowsExpressions())
    {
    return false;
    }
  if (def->numberOfRequiredValues() != 0)
    {
    return false; // The number of values is fixed
    }
  if (!def->isValidExpression(exp))
    {
    return false; // Attribute is of the proper type
    }
  int n = static_cast<int>(m_expressions.size());
  this->m_expressions.resize(n+1);
  def->buildExpressionItem(this, n);
  this->m_expressions[n]->setValue(exp);
  this->m_isSet.push_back(true);
  return true;
}
//----------------------------------------------------------------------------
bool ValueItem::isDiscrete() const
{
  return static_cast<const ValueItemDefinition*>(this->m_definition.get())->
    isDiscrete();
}
//----------------------------------------------------------------------------
void ValueItem::reset() 
{
  Item::reset();
}
//----------------------------------------------------------------------------
bool ValueItem::setDiscreteIndex(int element, int index)
{
  if (!this->isDiscrete())
    {
    return false;
    }
  const ValueItemDefinition *def = 
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (def->isDiscreteIndexValid(index))
    {
    this->m_discreteIndices[element] = index;
    if (def->allowsExpressions())
      {
      this->m_expressions[element]->unset();
      }
    this->m_isSet[element] = true;
    this->updateDiscreteValue(element);
    this->updateActiveChildrenItems();
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void ValueItem::updateActiveChildrenItems()
{
  // This is only for Discrete Value Items
  if (!this->isDiscrete())
    {
    return;
    }

  // Clear the current list of active children items
  this->m_activeChildrenItems.clear();

  // Note that for the current implementation only value items with 1
  // required value is support for conditional children.
  // Check to see if the index is valid
  const ValueItemDefinition *def =
    static_cast<const ValueItemDefinition*>(this->m_definition.get());
  if (!def->isDiscreteIndexValid(this->m_discreteIndices[0]))
    {
    return;
    }

  // Get the children that should be active for the current value
  std::string v = def->discreteEnum(this->m_discreteIndices[0]);
  std::vector<std::string> citems = def->conditionalItems(v);
  std::size_t i, n = citems.size();
  for (i = 0; i < n; i++)
    {
    this->m_activeChildrenItems.push_back(this->m_childrenItems[citems[i]]);
    }
}
//----------------------------------------------------------------------------
