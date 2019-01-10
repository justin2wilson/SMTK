//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_resource_Observer_h
#define __smtk_resource_Observer_h

#include "smtk/resource/Resource.h"

#include <map>

namespace smtk
{
namespace resource
{

/**\brief Enumerate events that the resource manager may encounter.
  *
  * Note that the resource modification event only fires when
  * a resource's clean/dirty state changes, not for each modification:
  * once a resource is dirty, further modifications are not reported.
  * If you want to monitor each modification to a resource, you should
  * observe operations via an operation manager.
  *
  * Also, the MODIFIED signal is fired when a resource is marked clean
  * not just when it is marked dirty.
  */
enum class EventType
{
  ADDED,    //!< A new resource's contents now available in memory.
  MODIFIED, //!< an existing resource's clean/dirty state has changed.
  REMOVED   //!< An existing resource's contents are being removed from memory.
};

typedef std::function<void(std::shared_ptr<Resource>, EventType)> Observer;

class SMTKCORE_EXPORT Observers
{
public:
  using Key = int;
  using ObserverInitializer = std::function<void(Manager*, Observer)>;

  Observers(Manager* rsrcMgr, ObserverInitializer initializer = nullptr);

  /// Iterate over the collection of observers and execute the observer functor.
  void operator()(std::shared_ptr<Resource>, EventType);

  /// Ask to receive notification events on all resources. The return value is a
  /// handle that can be used to unregister the observer. By default, this will
  /// immediately invoke the observer with a list of resources already available.
  Key insert(Observer observer, bool immediatelyUpdate = true);

  /// Indicate that an observer should no longer be called. Returns the number
  /// of remaining observers.
  std::size_t erase(Key);

  /// Returns the observer for the given key if one exists or nullptr otherwise.
  Observer find(Key) const;

private:
  // A map of observers. The observers are held in a map so that they can be
  // referenced (and therefore removed) at a later time using the observer's
  // associated key.
  Manager* m_parent;
  ObserverInitializer m_initializer;
  std::map<Key, Observer> m_observers;
};
}
}

#endif // __smtk_resource_Observer_h
