//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/testing/cxx/UnitTestCustomFactoryWidget.h"

#include "smtk/common/Managers.h"
#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "smtk/io/Logger.h"
#include "smtk/plugin/Registry.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Registrar.h"
#include "smtk/task/Task.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <QWidget>
#include <QtTest/QtTest>

namespace
{

class TestCustomWidget : public QWidget
{

public:
  explicit TestCustomWidget(smtk::task::Task* task, QWidget* parent = nullptr)
    : QWidget(parent)
    , m_task(task)
  {
  }

  static QWidget* create(smtk::task::Task* task, QWidget* parent = nullptr)
  {
    return new TestCustomWidget(task, parent);
  }

  smtk::task::Task::Ptr m_task;
};

} // namespace

CustomFactoryWidgetTester::CustomFactoryWidgetTester()
  : m_window(new QMainWindow)
  , m_container(new QWidget(m_window))
  , m_layout(new QVBoxLayout)
{
  m_window->setCentralWidget(m_container);
  m_container->setLayout(m_layout);
  m_window->show();

  testCreateCustomWidget();
}

void CustomFactoryWidgetTester::testCreateCustomWidget()
{
  // Create managers
  auto managers = smtk::common::Managers::create();
  auto taskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(managers);

  auto taskManager = smtk::task::Manager::create();
  auto taskTaskRegistry = smtk::plugin::addToManagers<smtk::task::Registrar>(taskManager);
  std::string configString = R"(
{
 "tasks": [
    {
      "name" : "Task 1",
      "id": 1,
      "type": "smtk::task::Task",
      "style": [ "CustomTaskStyle" ]
    }
  ],
  "styles": {
    "CustomTaskStyle":
    {
      "qtDefaultTaskNode": {
        "children": [
          "customStyle"
        ]
      }
    }
  }
})";

  // Create a task from the configString, with the style that will be registered
  auto taskConfig = nlohmann::json::parse(configString);
  nlohmann::json roundTrip;
  bool ok = true;
  try
  {
    smtk::task::json::Helper::pushInstance(*taskManager, managers);
    smtk::task::from_json(taskConfig, *taskManager);
    smtk::task::to_json(roundTrip, *taskManager);
    smtk::task::json::Helper::popInstance();
  }
  catch (std::exception&)
  {
    ok = false;
  }
  auto task1 = taskManager->taskInstances().findByName("Task 1").begin();
  m_task = (*task1).get();
  QVERIFY(m_task != nullptr);

  QVERIFY(taskManager->taskInstances().size() == 1);

  // Register the custom widget factory method with the style type as the key
  qtSMTKUtilities::registerTaskWidgetConstructor("customStyle", TestCustomWidget::create);

  // Call qtSMTKUtilities::findWidgetFactoryType(task) and verify it returns the correct factory types
  auto factoryTypes = qtSMTKUtilities::findWidgetFactoryType(m_task);
  QVERIFY(!factoryTypes.empty());
  QVERIFY(std::find(factoryTypes.begin(), factoryTypes.end(), "customStyle") != factoryTypes.end());

  // For each factory type, call qtSMTKUtilities::getTaskWidgetConstructor(factoryType) and verify it returns a valid factory method
  for (auto& factoryType : factoryTypes)
  {
    auto factoryMethod = qtSMTKUtilities::getTaskWidgetConstructor(factoryType);
    QVERIFY(factoryMethod != nullptr);

    // Call the factory method with the task and a container as arguments and verify it returns a valid widget
    QWidget* childWidget = factoryMethod(m_task, m_container);
    QVERIFY(childWidget != nullptr);

    // In actuality, won't have access to customWidgets type, just testing it works though
    TestCustomWidget* childWidgetCasted = dynamic_cast<TestCustomWidget*>(childWidget);
    QVERIFY(childWidgetCasted != nullptr);

    // Test to see if the task was set on the widget, and verify they're the same
    QVERIFY(childWidgetCasted->m_task->id() == m_task->id());

    // Add the widget to the container's layout and verify it has been added correctly
    m_container->layout()->addWidget(childWidgetCasted);
    m_layout->addWidget(childWidgetCasted);
    QVERIFY(m_container->layout()->indexOf(childWidgetCasted) != -1);
    QVERIFY(m_layout->indexOf(childWidgetCasted) != -1);
  }
}

int UnitTestCustomFactoryWidget(int argc, char** const argv)
{
  QApplication app(argc, argv);

  // Create an instance of UnitTestCustomFactoryWidget
  CustomFactoryWidgetTester tester;

  return QTest::qExec(&tester, argc, argv);
}
