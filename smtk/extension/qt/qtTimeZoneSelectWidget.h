//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtTimeZoneSelectWidget - ui panel for selecting timezone
// .SECTION Description
// .SECTION Caveats

#ifndef __smtk_extension_qtTimeZoneSelectWidget_h
#define __smtk_extension_qtTimeZoneSelectWidget_h

#include "smtk/extension/qt/Exports.h"
#include <QItemSelection>
#include <QModelIndex>
#include <QWidget>

class Ui_qtTimeZoneSelectWidget;

namespace smtk {
  namespace extension {

class SMTKQTEXT_EXPORT qtTimeZoneSelectWidget : public QWidget
{
  Q_OBJECT

 public:
  qtTimeZoneSelectWidget(QWidget* parent = NULL);
  ~qtTimeZoneSelectWidget();

  void setContinent(const QModelIndex index);

 public slots:

 signals:

 protected slots:
  void onContinentChanged(
    const QItemSelection& selected, const QItemSelection& deselected);
  void onRegionChanged(
    const QItemSelection& selected, const QItemSelection& deselected);

 protected:
  Ui_qtTimeZoneSelectWidget *UI;

 private:
  class qtTimeZoneSelectWidgetInternal;
  qtTimeZoneSelectWidgetInternal *Internal;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_extension_qtTimeZoneSelectWidget_h
