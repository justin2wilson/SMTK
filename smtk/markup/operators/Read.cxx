//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/markup/operators/Read.h"

#include "smtk/markup/Resource.h"
#include "smtk/markup/operators/Read_xml.h"

#include "smtk/operation/Hints.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/markup/json/jsonResource.h"

#include "smtk/resource/json/Helper.h"

#include "smtk/common/Paths.h"

#include <fstream>

namespace smtk
{
namespace markup
{

bool Read::ableToOperate()
{
  if (!this->Superclass::ableToOperate())
  {
    return false;
  }

  auto filename = this->parameters()->findFile("filename")->value();
  std::ifstream file(filename);
  return file.good();
}

Read::Result Read::operateInternal()
{
  auto filename = this->parameters()->findFile("filename")->value();
  std::ifstream file(filename);
  if (!file.good())
  {
    smtkErrorMacro(this->log(), "Could not open file \"" << filename << "\" for reading.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  nlohmann::json jj;
  try
  {
    jj = nlohmann::json::parse(file);
  }
  catch (...)
  {
    smtkErrorMacro(this->log(), "File \"" << filename << "\" is not in JSON format.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  auto resource = smtk::markup::Resource::create();
  resource->setLocation(filename);

  // Deserialize resource from a set of JSON records:
  auto& helper = smtk::resource::json::Helper::pushInstance(resource);
  helper.setManagers(this->managers());

  smtk::markup::from_json(jj, resource);

  // std::string fileDirectory = smtk::common::Paths::directory(rsrc->location()) + "/";

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  result->findResource("resource")->appendValue(resource);

  std::set<smtk::resource::PersistentObject::Ptr> targets{ { resource } };
  smtk::operation::addBrowserExpandHint(result, targets);

  smtk::resource::json::Helper::popInstance();
  return result;
}

const char* Read::xmlDescription() const
{
  return Read_xml;
}

void Read::markModifiedResources(Read::Result& result)
{
  auto resourceItem = result->findResource("resource");
  for (std::size_t ii = 0; ii < resourceItem->numberOfValues(); ++ii)
  {
    if (!resourceItem->isSet(ii))
    {
      continue;
    }
    auto resource = std::dynamic_pointer_cast<smtk::markup::Resource>(resourceItem->value(ii));

    // Set the resource as unmodified from its persistent (i.e. on-disk) state
    if (resource)
    {
      resource->setClean(true);
    }
  }
}

smtk::resource::ResourcePtr read(
  const std::string& filename,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  Read::Ptr read = Read::create();
  read->setManagers(managers);
  read->parameters()->findFile("filename")->setValue(filename);
  Read::Result result = read->operate();
  if (result->findInt("outcome")->value() != static_cast<int>(Read::Outcome::SUCCEEDED))
  {
    return smtk::resource::ResourcePtr();
  }
  return result->findResource("resource")->value();
}

} // namespace markup
} // namespace smtk
