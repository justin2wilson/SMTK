//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Operation.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/operation/Operation_xml.h"

#include "nlohmann/json.hpp"

#include <memory>
#include <sstream>

namespace smtk
{
namespace operation
{

Operation::Operation()
  : m_debugLevel(0)
  , m_manager()
  , m_specification(nullptr)
  , m_parameters(nullptr)
  , m_resultDefinition(nullptr)
{
}

Operation::~Operation()
{
  // If the specification exists...
  if (m_specification != nullptr)
  {
    // ...and if the parameters have been generated, remove the parameters from
    // the specification.
    if (m_parameters != nullptr)
    {
      m_specification->removeAttribute(m_parameters);
    }

    // Similarly, remove all results from the specification that were generated
    // by this operation.
    for (auto& result : m_results)
    {
      m_specification->removeAttribute(result);
    }
  }
}

std::string Operation::uniqueName() const
{
  if (auto manager = m_manager.lock())
  {
    // If the operation's manager is set, then the operation is registered to a
    // manager. The operation metadata has a unique name for this operation
    // type, so we return this name.
    auto metadata = manager->metadata().get<IndexTag>().find(this->index());
    if (metadata != manager->metadata().get<IndexTag>().end())
    {
      return metadata->uniqueName();
    }
  }

  // Either this operation is not registered to a manager or it does not have a
  // unique name registered to it. Simply return the class name.
  return this->classname();
}

Operation::Specification Operation::specification()
{
  // Lazily create the specification.
  if (m_specification == nullptr)
  {
    if (auto manager = m_manager.lock())
    {
      auto metadata = manager->metadata().get<IndexTag>().find(this->index());

      // The only way for an operation's manager to be set is if a manager
      // created it. The only way for a manager to create an operation is if it
      // has a metadata instance for its type. Let's check anyway.
      assert(metadata != manager->metadata().get<IndexTag>().end());

      m_specification = metadata->specification();
    }
    else
    {
      m_specification = createSpecification();
    }
  }
  return m_specification;
}

bool Operation::ableToOperate()
{
  return this->parameters()->isValid();
}

Operation::Result Operation::operate()
{
  // Gather all requested resources and their permission levels.
  auto resourcesAndPermissions = extractResourcesAndPermissions(this->specification());

  // Lock the resources.
  for (auto& resourceAndPermission : resourcesAndPermissions)
  {
    auto& resource = resourceAndPermission.first;
    auto& permission = resourceAndPermission.second;
    resource->lock({}).lock(permission);
  }

  // Remember where the log was so we only serialize messages for this
  // operation:
  std::size_t logStart = this->log().numberOfRecords();

  Result result;

  // If an operation manager is associated with the operation, call its pre- and
  // post-operation observers. Note that all observers will be called even if
  // one requests the operation be canceled. This is useful since all
  // DID_OPERATE observers are called whether the operation was canceled or not
  // -- and observers of both will expect them to be called in pairs.
  auto manager = m_manager.lock();
  bool observePostOperation = manager != nullptr;

  // First, we check that the operation is able to operate.
  if (!this->ableToOperate())
  {
    result = this->createResult(Outcome::UNABLE_TO_OPERATE);
    // If the operation cannot operate, there is no need to call any observers.
    observePostOperation = false;
  }
  // Then, we check if any observers wish to cancel this operation.
  else if (manager && manager->observers()(shared_from_this(), EventType::WILL_OPERATE, nullptr))
  {
    result = this->createResult(Outcome::CANCELED);
  }
  else
  {
    // Finally, execute the operation.

    // Set the debug level if specified as a convenience for subclasses:
    smtk::attribute::IntItem::Ptr debugItem = this->parameters()->findInt("debug level");
    this->m_debugLevel = ((debugItem && debugItem->isEnabled()) ? debugItem->value() : 0);

    // Perform the derived operation.
    result = this->operateInternal();

    // Post-process the result if the operation was successful.
    int outcome = result->findInt("outcome")->value();
    if (outcome == static_cast<int>(Outcome::SUCCEEDED))
    {
      this->postProcessResult(result);
    }
  }

  // Add a summary of the operation to the result.
  this->generateSummary(result);

  // Now grab all log messages and serialize them into the result attribute.
  {
    std::size_t logEnd = this->log().numberOfRecords();
    if (logEnd > logStart)
    {
      // Serialize relevant log records to a json-formatted string.
      nlohmann::json j = std::vector<smtk::io::Logger::Record>(
        smtk::io::Logger::instance().records().begin() + logStart,
        smtk::io::Logger::instance().records().end());
      result->findString("log")->appendValue(j.dump());
    }
  }

  // Execute post-operation observation
  if (observePostOperation)
  {
    manager->observers()(shared_from_this(), EventType::DID_OPERATE, result);
  }

  // Unlock the resources.
  for (auto& resourceAndPermission : resourcesAndPermissions)
  {
    auto& resource = resourceAndPermission.first;
    auto& permission = resourceAndPermission.second;
    resource->lock({}).unlock(permission);
  }

  return result;
}

smtk::io::Logger& Operation::log() const
{
  return smtk::io::Logger::instance();
}

Operation::Parameters Operation::parameters()
{
  // If we haven't accessed our parameters yet, ask the specification to either
  // retrieve the exisiting one or create a new one.
  if (!m_parameters)
  {
    m_parameters = extractParameters(this->specification(), this->uniqueName());
  }

  // If we still don't have our parameters, then there's not much we can do.
  if (!m_parameters)
  {
    std::stringstream s;
    s << "Could not identify parameters attribute definition for operation \"" << this->classname()
      << "\".";
    smtkErrorMacro(this->log(), s.str());
  }

  return m_parameters;
}

Operation::Result Operation::createResult(Outcome outcome)
{
  // Our result definition is located once per instance of the operation, and is
  // subsequently retrieved from cache to avoid superfluous lookups.
  if (!m_resultDefinition)
  {
    m_resultDefinition = extractResultDefinition(this->specification(), this->uniqueName());
  }

  // Now that we have our result definition, we create our result attribute.
  Result result;

  if (m_resultDefinition)
  {
    // Create a new instance of the result.
    result = this->specification()->createAttribute(m_resultDefinition);

    // Hold on to a copy of the generated result so we can remove it from our
    // specification when the operation is destroyed.
    m_results.push_back(result);
  }
  else
  {
    std::stringstream s;
    s << "Could not identify result attribute definition for operation \"" << this->classname()
      << "\".";
    smtkErrorMacro(this->log(), s.str());
  }

  if (result)
  {
    result->findInt("outcome")->setValue(0, static_cast<int>(outcome));
  }
  return result;
}

void Operation::generateSummary(Operation::Result& result)
{
  std::stringstream s;
  int outcome = result->findInt("outcome")->value();
  s << this->classname() << ": ";
  switch (outcome)
  {
    case static_cast<int>(Outcome::UNABLE_TO_OPERATE):
      s << "unable to operate";
      break;
    case static_cast<int>(Outcome::CANCELED):
      s << "operation canceled";
      break;
    case static_cast<int>(Outcome::FAILED):
      s << "operation failed";
      break;
    case static_cast<int>(Outcome::SUCCEEDED):
      s << "operation succeeded";
      break;
    case static_cast<int>(Outcome::UNKNOWN):
      s << "outcome unknown";
      break;
  }

  if (outcome == static_cast<int>(Outcome::SUCCEEDED))
  {
    smtkInfoMacro(this->log(), s.str());
  }
  else
  {
    smtkErrorMacro(this->log(), s.str());
  }
}

Operation::Specification Operation::createBaseSpecification() const
{
  Specification spec = smtk::attribute::Collection::create();
  smtk::io::AttributeReader reader;
  reader.readContents(spec, Operation_xml, this->log());
  return spec;
}

} // operation namespace
} // smtk namespace
