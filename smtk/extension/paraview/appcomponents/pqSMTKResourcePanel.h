//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKResourcePanel_h
#define smtk_extension_paraview_appcomponents_pqSMTKResourcePanel_h

#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QDockWidget>

class pqSMTKResourceBrowser;

/**\brief A panel that displays SMTK resources available to the application/user.
  *
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKResourcePanel : public QDockWidget
{
  Q_OBJECT
  typedef QDockWidget Superclass;

public:
  pqSMTKResourcePanel(QWidget* parent = nullptr);
  ~pqSMTKResourcePanel() override;

  /// Let the panel display a custom view config, from json or xml.
  void setView(const smtk::view::ViewPtr& view);

protected slots:
  virtual void resourceManagerAdded(pqSMTKWrapper* mgr, pqServer* server);
  virtual void resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server);

protected:
  pqSMTKResourceBrowser* m_browser;
  smtk::view::ViewPtr m_view;
  smtk::extension::qtUIManager* m_viewUIMgr;
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKResourcePanel_h
