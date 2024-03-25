Qt Extension System
===================

Merge Request note
------------------
Implements a widget factory registry to extend the user interface of task nodes in
the diagram editor via the styles section in the worklet JSON.

Enhanced qtSMTKUtilities to include a method to create a widget from a given widget
factory name. Register a custom widget factory with the widget factory registry.
In qtDefaultTaskNode, the widget factory registry is used to create
a widget for the task node (if one exists).

How to use
----------
A unit test is located at `smtk/extension/qt/testing/cxx/CustomFactoryWidgetTest.cxx`

To allow for a custom widget for a given task node (qtDefaultTaskNode being the default), add a "widgetFactory" key to the
styles in the worklet JSON, underneath the "task" that the widget is for. An example is shown below:

.. code-block:: json

    {
      "tasks": [
        {
          "name": "Task 1",
          "style": [ "CustomTaskStyle" ]
        }
      ],
      "styles": {
            "CustomTaskStyle":
            {
              "qtDefaultTaskNode": {
                "children": [
                  "CustomFactoryWidget"
                ]
              }
            }
          }
    }

To register a custom widget factory, utilize the `smtk/extension/qt/qtSMTKUtilities::registerTaskWidgetConstructor` method. Ensure the string
provided matches the value under children in the json above. An example is shown below:

.. code-block:: cpp

    qtSMTKUtilities::registerTaskWidgetConstructor(
        "CustomFactoryWidget", smtk::extension::CustomTaskWidget::create);

The value of the "CustomFactoryWidget" key should be a static creation function, that the factory will use to create the widget.
The widget factory must be registered with the widget factory registry (`smtk/extension/qt/qtSMTKUtilities::registerTaskWidgetFactory`).
Once registered, the widget factory will be used to create the widget for the task node, if one exists.

In `smtk/extension/qt/diagrams/qtDefaultTaskNode.cxx`, the widget factory registry is used to create a widget for the task node.
In the constructor of the task node, the widget factory registry is queried for a widget factory with the task and the taskType
(which is by default "qtDefaultTaskNode") by way of qtSMTKUtilities::findWidgetFactoryType. If a widget factory is found in the
json styles, it adds it to a vector which is then returned. Next, the vector of custom widgets is iterated over and compared
to the registered widget factories. If a match is found, the widget is created and added as a child of the taskNodeWidget.
