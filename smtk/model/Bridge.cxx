#include "smtk/model/Bridge.h"

#include "smtk/model/RemoteOperator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/Manager.h"

#include "smtk/util/AttributeReader.h"
#include "smtk/util/Logger.h"

using smtk::attribute::Definition;
using smtk::attribute::IntItemDefinition;
using smtk::attribute::RefItemDefinition;

namespace smtk {
  namespace model {

/// Default constructor. This assigns a random session ID to each Bridge instance.
Bridge::Bridge()
  : m_sessionId(smtk::util::UUID::random()), m_operatorMgr(NULL)
{
}

/// Destructor. We must delete the attribute manager that tracks operator definitions.
Bridge::~Bridge()
{
  delete this->m_operatorMgr;
}

/**\brief Return the name of the bridge type (i.e., the name of the modeling kernel).
  *
  * Subclasses override this method by using the smtkDeclareModelingKernel
  * and smtkImplementsModelingKernel macros.
  */
std::string Bridge::name() const
{
  return "invalid";
}

/**\brief Return the session ID for this instance of the bridge.
  *
  * Sessions are ephemeral and tied to a particular machine so
  * they should generally not be serialized. However, when using
  * JSON stringifications of operators to perform remote procedure
  * calls (RPC), the session ID specifies which Bridge on which
  * machine should actually invoke the operator.
  */
smtk::util::UUID Bridge::sessionId() const
{
  return this->m_sessionId;
}

/**\brief Transcribe an entity from a foreign modeler into an SMTK storage Manager.
  *
  * On input, the \a entity will not be valid but if transcription is
  * successful, the \a requested records in the \a entity's Manager will
  * be valid. If \a requested includes BRIDGE_ENTITY_TYPE, then
  * \a entity.isValid() should return true after this call.
  *
  * Only honor requests for entity IDs listed as dangling unless
  * \a onlyDangling is false (default is true).
  * This prevents expensive requests by Manager instances over many Bridges.
  *
  * The return value is 0 upon failure and non-zero upon success.
  * Failure occurs when any \a requested bits of information that
  * are in Bridgee::allSupportedInformation() are not transcribed,
  * or when \a requested is 0.
  */
int Bridge::transcribe(
  const Cursor& entity, BridgedInfoBits requested, bool onlyDangling)
{
  int retval = 0;
  if (requested)
    {
    // Check that the entity IDs is dangling or we are forced to continue.
    DanglingEntities::iterator it = this->m_dangling.find(entity);
    if (onlyDangling && it == this->m_dangling.end())
      { // The bridge has not been told that this UUID exists.
      return retval;;
      }
    // Ask the subclass to transcribe information.
    BridgedInfoBits actual = this->transcribeInternal(entity, requested);
    // Decide which bits of the request can possibly be honored...
    BridgedInfoBits honorable = requested & this->allSupportedInformation();
    // ... and verify that all of those have been satisfied.
    retval = (honorable & actual) == honorable;
    // If transcription is complete, then remove the UUID from the dangling
    // entity set:
    if (
      ((actual & this->allSupportedInformation()) == this->allSupportedInformation()) &&
      (it != this->m_dangling.end()))
        this->m_dangling.erase(it);
    }
  return retval;
}

/**\brief Return a bit vector describing what types of information can be transcribed.
  *
  * This is used to determine when an entity has been fully transcribed into storage
  * and is no longer "dangling."
  */
BridgedInfoBits Bridge::allSupportedInformation() const
{
  return BRIDGE_EVERYTHING;
}

/// Return a list of names of solid-model operators available.
StringList Bridge::operatorNames() const
{
  std::vector<smtk::attribute::DefinitionPtr> ops;
  this->m_operatorMgr->derivedDefinitions(
    this->m_operatorMgr->findDefinition("operator"), ops);

  StringList nameList;
  std::vector<smtk::attribute::DefinitionPtr>::iterator it;
  for (it = ops.begin(); it != ops.end(); ++it)
    nameList.push_back((*it)->type());
  return nameList;
}

OperatorPtr Bridge::op(const std::string& opName, ManagerPtr manager) const
{
  OperatorPtr oper;
  if (opName.empty())
    return oper;

  OperatorConstructor ctor = this->findOperatorConstructor(opName);
  if (!ctor)
    return oper;

  oper = ctor();
  if (!oper)
    return oper;

  oper->setBridge(const_cast<Bridge*>(this));
  oper->setManager(manager);

  RemoteOperator::Ptr remoteOp = smtk::dynamic_pointer_cast<RemoteOperator>(oper);
  if (remoteOp)
    remoteOp->setName(opName);

  return oper;
}

/// Return the map from dangling cursors to bits describing their partial transcription state.
const DanglingEntities& Bridge::danglingEntities() const
{
  return this->m_dangling;
}

/**\brief Mark an entity, \a ent, as partially transcribed.
  *
  * Subclasses should call this method when a UUID has been assigned
  * to a model entity but ent.manager() has not yet been populated with
  * all of the information about the entity. The information which *is*
  * \a present in ent.manager() should be passed but will default to
  * zero (i.e., the UUID exists in some other entity's relations but
  * has no records in manager itself).
  *
  * The entity is added to the list of dangling entities and will be
  * removed from the list when a call to \a transcribeInternal indicates
  * that Bridge::allSupportedInformation() is now present in manager.
  */
void Bridge::declareDanglingEntity(const Cursor& ent, BridgedInfoBits present)
{
  if ((present & this->allSupportedInformation()) < this->allSupportedInformation())
    this->m_dangling[ent] = present;
}

/** @name Operator Manager
  *\brief Return this bridge's internal attribute manager, used to describe operators.
  *
  * Each operator should have a definition of the same name held in this manager.
  */
///@{
smtk::attribute::Manager* Bridge::operatorManager()
{
  return this->m_operatorMgr;
}

const smtk::attribute::Manager* Bridge::operatorManager() const
{
  return this->m_operatorMgr;
}
///@}

/**\brief Transcribe information requested by \a flags into \a entity from foreign modeler.
  *
  * Subclasses must override this method.
  * This method should return a non-zero value upon success.
  * Upon success, \a flags should be modified to represent the
  * actual information transcribed (as opposed to what was requested).
  * This should always be at least the information requested but may
  * include more information.
  */
BridgedInfoBits Bridge::transcribeInternal(const Cursor& entity, BridgedInfoBits flags)
{
  (void)entity;
  (void)flags;
  // Fail to transcribe anything:
  return 0;
}

/**\brief Set the session ID.
  *
  * Do not call this unless you are preparing the bridge
  * to be a remote mirror of a modeling session (for, e.g.,
  * client-server operation).
  */
void Bridge::setSessionId(const smtk::util::UUID& sessId)
{
  this->m_sessionId = sessId;
}

/**\brief Subclasses must call this method from within their constructors.
  *
  * Each subclass has (by virtue of invoking the smtkDeclareModelOperator
  * and smtkImplementsModelOperator macros) a static map from operator
  * names to constructors and XML descriptions. That map is named
  * s_operators and should be passed to this method in the constructor
  * of the subclass (since the base class does not have access to the map).
  *
  * This method traverses the XML descriptions and imports each into
  * the bridge's attribute manager.
  */
void Bridge::initializeOperatorManager(const OperatorConstructors* opList, bool inheritSubclass)
{
  // Subclasses may already have initialized
  if (this->m_operatorMgr && !inheritSubclass)
    {
    delete this->m_operatorMgr;
    this->m_operatorMgr = NULL;
    }

  if (!this->m_operatorMgr)
    {
    this->m_operatorMgr = new smtk::attribute::Manager;

    // Create the "base" definitions that all operators and results will inherit.
    this->m_operatorMgr->createDefinition("operator");
    Definition::Ptr defn = this->m_operatorMgr->createDefinition("result");
    IntItemDefinition::Ptr outcomeDefn = IntItemDefinition::New("outcome");
    RefItemDefinition::Ptr paramsDefn = RefItemDefinition::New("validated parameters");
    outcomeDefn->setNumberOfRequiredValues(1);
    outcomeDefn->setIsOptional(false);
    paramsDefn->setNumberOfRequiredValues(1);
    paramsDefn->setIsOptional(true);
    defn->addItemDefinition(outcomeDefn);
    defn->addItemDefinition(paramsDefn);
    }

  if (!opList) return;

  smtk::util::Logger log;
  smtk::util::AttributeReader rdr;
  OperatorConstructors::const_iterator it;
  bool ok = true;
  for (it = opList->begin(); it != opList->end(); ++it)
    {
    if (it->second.first.empty())
      continue;

    ok &= !rdr.readContents(
      *this->m_operatorMgr,
      it->second.first.c_str(), it->second.first.size(),
      log);
    }
  if (!ok)
    {
    std::cerr
      << "Error. Log follows:\n---\n"
      << log.convertToString()
      << "\n---\n";
    }
}

/**\brief A convenience method used by subclass findOperatorXML methods.
  */
std::string Bridge::findOperatorXMLInternal(
  const std::string& opName,
  const OperatorConstructors* opList) const
{
  std::string xml;
  if (!opList)
    { // No operators registered.
    return xml;
    }
  smtk::model::OperatorConstructors::const_iterator it =
    opList->find(opName);
  if (it == opList->end())
    { // No matching operator.
    return xml;
    }
  return it->second.first;
}

/**\brief A convenience method used by subclass findOperatorConstructor methods.
  */
OperatorConstructor Bridge::findOperatorConstructorInternal(
  const std::string& opName,
  const OperatorConstructors* opList) const
{
  if (!opList)
    { // No operators registered.
    return smtk::model::OperatorConstructor();
    }
  smtk::model::OperatorConstructors::const_iterator it =
    opList->find(opName);
  if (it == opList->end())
    { // No matching operator.
    return smtk::model::OperatorConstructor();
    }
  return it->second.second;
}

  } // namespace model
} // namespace smtk
