//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_Collection_h
#define __smtk_mesh_Collection_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#include "smtk/mesh/CellSet.h"
#include "smtk/mesh/PointConnectivity.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/mesh/QueryTypes.h"
#include "smtk/mesh/TypeSet.h"

#include "smtk/model/EntityRef.h"

#include <vector>

namespace smtk {
namespace mesh {

//Flyweight interface around a moab database of meshes. When constructed
//becomes registered with a manager with a weak relationship.
class SMTKCORE_EXPORT Collection : public smtk::enable_shared_from_this<Collection>
{
  //default constructor generates an invalid collection
  Collection();

  //Construct a valid collection that is associated with a manager
  //but has an empty interface that can be populated
  Collection( smtk::mesh::ManagerPtr mngr );

  //Construct a valid collection that has an associated interface
  //in the future we need a better way to make collections refer
  //to different mesh interfaces
  Collection( smtk::mesh::InterfacePtr interface,
              smtk::mesh::ManagerPtr mngr);

public:
  smtkTypeMacro(Collection);
  //construct an invalid collection
  smtkCreateMacro(Collection);

  ~Collection();

  //determine if the given Collection is valid and is properly associated
  //to a manager.
  bool isValid() const;

  //get the name of a mesh collection
  const std::string& name() const;

  //fetch the entity id for this uuid
  const smtk::common::UUID entity() const;

  //re-parent the collection onto a new manager.
  bool reparent(smtk::mesh::ManagerPtr newParent);

  std::size_t numberOfMeshes() const;

  //----------------------------------------------------------------------------
  //Queries on the full Collection
  //----------------------------------------------------------------------------
  smtk::mesh::TypeSet   associatedTypes( ) const;
  smtk::mesh::MeshSet   meshes( ); //all meshes
  smtk::mesh::CellSet   cells( ); //all cells
  smtk::mesh::Points    points( ); //all points

  //todo:
  //find all cells of a given dimension that are attached to ?
  //smtk::mesh::CellSet   connectivity( smtk::mesh::DimensionType dim );

  smtk::mesh::PointConnectivity pointConnectivity( ); //all point connectivity info for all cells

  //For any mesh set that has a name we return that name. It is possible
  //that the we have un-named mesh sets.
  std::vector< std::string > meshNames();

  //Find all meshes that have at least one cell of the given type.
  //This means that you can get back meshes of mixed dimension
  //type.
  smtk::mesh::MeshSet   meshes( smtk::mesh::DimensionType dim );
  smtk::mesh::MeshSet   meshes( const std::string& name );
  smtk::mesh::MeshSet   meshes( const smtk::mesh::Material& m )
                              { return materialMeshes(m); }

  //find a cells of a given type or a collection of types
  smtk::mesh::CellSet   cells( smtk::mesh::CellType cellType );
  smtk::mesh::CellSet   cells( smtk::mesh::CellTypes cellTypes );
  smtk::mesh::CellSet   cells( smtk::mesh::DimensionType dim );

  //----------------------------------------------------------------------------
  // Queries by a model Cursor
  //----------------------------------------------------------------------------
  smtk::mesh::TypeSet   findAssociatedTypes( const smtk::model::EntityRef& eref );
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref );
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType );
  smtk::mesh::MeshSet   findAssociatedMeshes( const smtk::model::EntityRef& eref, smtk::mesh::DimensionType dim );
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref );
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref, smtk::mesh::CellType cellType );
  smtk::mesh::CellSet   findAssociatedCells( const smtk::model::EntityRef& eref, smtk::mesh::DimensionType dim );

  //----------------------------------------------------------------------------
  // Construction of new meshes
  //----------------------------------------------------------------------------
  //given a collection of existing cells make a new Mesh inside the underlying interface
  //Return that Mesh as a MeshSet with a size of 1. The CellSet could
  //be the result of appending/intersecting,difference of other CellSets.
  //Adding a CellSet that is part of a different collection will fail, and
  //we will return an empty MeshSet.
  //Asking to create a MeshSet from a CellSet that is empty will fail, and
  //we will return an empty MeshSet.
  smtk::mesh::MeshSet createMesh( const smtk::mesh::CellSet& cells );

  //----------------------------------------------------------------------------
  // Deletion of Items
  //----------------------------------------------------------------------------
  //given a collection of meshes this will delete all meshes and any cell or vert
  //that is not referenced by any other mesh
  //This will invalidate any smtk::mesh::MeshSet that contains a reference to
  //one of the meshes that has been deleted.
  bool removeMeshes( smtk::mesh::MeshSet& meshesToDelete );


  //----------------------------------------------------------------------------
  // Material Queries
  //----------------------------------------------------------------------------
  //get all the current materials
  std::vector< smtk::mesh::Material > materials();

  //get the meshes with a given material value. If no meshes have
  //this material value the result will be empty
  smtk::mesh::MeshSet materialMeshes( const smtk::mesh::Material& m );

  //Assign a given material to a collection of meshes. Overwrites
  //any existing material value
  bool setMaterialOnMeshes(const smtk::mesh::MeshSet& meshes,
                           const smtk::mesh::Material& m);

  //todo: query based on boundary and other attributes of the mesh db
  //Tag("BOUNDARY_SET"){};
  //smtk::mesh:::MeshSet materialMeshes();
  //Tag("DIRICHLET_SET"){};
  //smtk::mesh:::MeshSet dirichletMeshes();
  //Tag("NEUMANN_SET"){};
  //smtk::mesh:::MeshSet neumannMeshes();
  //todo: need to be able to extract the entire surface of the mesh
  //smtk::mesh::MeshSet generateBoundarMeshes();


  const smtk::mesh::InterfacePtr& interface() const;

private:
  Collection( const Collection& other ); //blank since we are used by shared_ptr
  Collection& operator=( const Collection& other ); //blank since we are used by shared_ptr

  friend class smtk::mesh::Manager;

  //called by the manager that manages this collection, means that somebody
  //has requested us to be removed from a collection
  void removeManagerConnection( );

  smtk::common::UUID m_entity;
  std::string m_name;

  //holds a reference to both the manager and the specific backend interface
  class InternalImpl;
  smtk::mesh::Collection::InternalImpl* m_internals;
};

}
}

#endif  //__smtk_mesh_Collection_h
