//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_doc_h
#define __smtk_doc_h

/*!\mainpage Simulation Modeling Tool Kit
 *
 * The Simulation Modeling Tool Kit (SMTK) is a library for preparing
 * one or more simulation runs from descriptions of their attributes.
 * Inputs may include
 *
 * + a description of the simulation package(s) to be used,
 * + a description of the simulation domain,
 * + attributes defined over the domain (boundary and initial conditions
 *   as well as global simulation and solver options), and optionally
 * + a solid model (from which an analysis mesh may be generated by a mesher)
 * + an analysis mesh.
 *
 * The library uses the ::smtk namespace and divides classes
 * into components for smtk::attribute definition and smtk::model definition.
 */

/**\brief The main namespace for the Simulation Modeling Tool Kit (SMTK).
  *
  */
namespace smtk
{

/**\brief Classes used throughout the toolkit.
  *
  */
namespace common
{
}

/**\brief A common base class for resources (data stored in files) and tools to manage them.
  *
  * A set of classes exist for presenting model information to users.
  * DescriptivePhrase is an abstract base class with subclasses in
  * other SMTK subsystems for presenting information about their
  * components. ComponentListPhrase presents a "folder"-like item
  * that holds an array of phrases describing components.
  * Similarly, PropertyListPhrase and PropertyValuePhrase
  * hold information about string, floating-point, and integer data
  * indexed by resource and component UUIDs.
  *
  * These instances are placed into a hierarchy
  * that describe the model in a context. Consider these examples
  * specific to SMTK's modeling subsystem:  in a functional modeling
  * context, perhaps only descriptions of models and their groups will
  * be displayed. When assigning geometry to functional groups,
  * perhaps only geometric cells of a particular dimension will be shown.
  * The SubphraseGenerator class is what determines the particular
  * hierarchy, and a subclass will generally be written for each
  * context in which resource components should be presented.
  */
namespace resource
{
}

/**\brief A common base class for operators that act on resources and a manager to track subclasses.
  *
  */
namespace operation
{
}

/**\brief Define attributes describing simulation inputs.
  *
  */
namespace attribute
{
}

/**\brief Represent geometric and topological models of simulation domains.
  *
  * The Manager class holds records defining one or more geometric-
  * and/or topological-domain decompositions;
  * it maps smtk::common::UUID values to Entity, Arrangement, and
  * Tessellation instances.
  * However, most developers will use the EntityRef classes
  * (Vertex, Edge, Face, Volume, VertexUse, EdgeUse, FaceUse, VolumeUse,
  * Chain, Loop, Shell, Group, Model, and Instance)
  * to access this information.
  * EntityRef is a base class for traversing records in Manager
  * and provides some capability for modifying the model.
  * Attributes may be defined on any record in storage by virtue of the
  * fact that all records in storage are named by their UUID.
  *
  * If built with VTK, several classes beginning with "vtk" are available
  * for rendering and interacting with model entities which have
  * tessellation information.
  *
  * If built with Qt, the QEntityItemModel, QEntityItemDelegate, and
  * QEntityItemEditor classes may be used to display model information
  * as exposed by a hierarchy of DescriptivePhrase instances.
  */
namespace model
{
}

/**\brief Mesh representation, classification, and manipulation.
  *
  */
namespace mesh
{
/// moab is an external meshing library.
namespace moab
{
}
/// json serialization.
namespace json
{
}
}

/**\brief Projects organize a user workspace.
  *
  */
namespace project
{
}

/**\brief Tools for exporting simulation input decks from attributes.
  *
  */
namespace simulation
{
}

/**\brief Classes for presenting resources and their components to users.
  *
  */
namespace view
{
}

/**\brief I/O utilities for the toolkit.
  *
  */
namespace io
{
}

/**\brief workflow managment.
  *
  */
namespace workflow
{
}

/**\brief Sessions for solid modeling kernels.
  *
  */
namespace session
{
/**\brief A session for discrete.
  *
  */
namespace discrete
{
}
/**\brief A session that imports meshes.
  *
  */
namespace mesh
{
}
/**\brief A session for multiscale.
  *
  */
namespace multiscale
{
}
/**\brief A session for gaussian oscillators in a volume.
  *
  */
namespace oscillator
{
}
/**\brief A session for planar polygonal model geometry.
  *
  */
namespace polygon
{
}
}

/**\brief Extensions to SMTK that introduce external dependencies.
  *
  * Generally, any functionality that introduces external dependencies but
  * does not expose a modeling kernel belongs in this namespace.
  */
namespace extension
{

/**\brief A simple mesh generator.
  *
  */
namespace delaunay
{
}
/**\brief matplotlib.
  *
  */
namespace matplotlib
{
}
/**\brief Expose algorithms for creating models from image data.
  *
  */
namespace opencv
{
}
/**\brief Extensions that expose SMTK data in ParaView.
  *
  */
namespace paraview
{
}
/**\brief Extensions that expose SMTK data in Qt UI components.
  *
  */
namespace qt
{
}
/**\brief Remote meshing via interprocess communication.
  *
  */
namespace remus
{
}
/**\brief Use VTK instead of ParaView.
  *
  */
namespace vtk
{
}
/**\brief Expose algorithms for creating models from image data.
  *
  */
namespace vxl
{
}
}
}

#endif // __smtk_doc_h
