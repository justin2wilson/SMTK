//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qt_CustomFactoryWidget_h
#define smtk_extension_qt_CustomFactoryWidget_h

#include <QLayout>
#include <QMainWindow>

#include "smtk/task/Task.h"

class CustomFactoryWidgetTester : public QObject
{
  Q_OBJECT;

public:
  CustomFactoryWidgetTester();

  private Q_SLOTS:
    void testCreateCustomWidget();

private:
  QMainWindow* m_window;
  QWidget* m_container;
  QLayout* m_layout;

  smtk::task::Task* m_task;
};


#endif // smtk_extension_qt_CustomFactoryWidget_h