#include <QAbstractTableModel>
#include <QVector>
#include <QVariant>
#include <Qt>
#include <QWidget>
#include <QColor>
#include <QTableView>
#include <QHeaderView>
#include <QApplication>
#include <QStyleHints>

#include "task_utils.h"
#include "task_list.h"

// --- Task Table Model ---
// The model that will be used by a QTableView to get data for the correct roles and display.
TaskTableModel::TaskTableModel(TaskUtils::TaskList* taskList, QObject* parent)
    : QAbstractTableModel(parent), m_taskList(taskList) {}

 // Return the number of available tasks.
int TaskTableModel::rowCount(const QModelIndex& parent) const
{
    return m_taskList->size();
}

// Return the size of the TaskFieldMap, as this is where we define the fields available in the view and filter.
int TaskTableModel::columnCount(const QModelIndex& parent) const
{
    return TaskUtils::Fields::TaskFieldIndexMap.size();
}

// As the data is a pointer to a list, just refresh the whole table.
void TaskTableModel::RefreshModel()
{
    beginResetModel();
    endResetModel();
}

QVariant TaskTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // Skip vertical header.
    if (orientation == Qt::Vertical)
    {
        return QVariant();
    }

    // Get column corresponding task field
    const TaskUtils::TaskField& field = TaskUtils::Fields::TaskFieldIndexMap.value(section);

    // Roles
    // Display
    if (role == Qt::DisplayRole)
    {
        return field.displayName;
    }

    // Tooltip
    if (role == Qt::ToolTipRole)
    {
        return field.tooltip;
    }
    return QVariant();
}

// Data reading for the table.
QVariant TaskTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_taskList)
    {
        return QVariant();
    }

    // Get basic infos to use in this method
    quint8 row = index.row();        
    quint8 column = index.column(); 

    const TaskUtils::Task& task = m_taskList->at(row);
    const TaskUtils::TaskField& field = TaskUtils::Fields::TaskFieldIndexMap.value(column);

    // Roles
    switch (role)
    {
    case Qt::TextAlignmentRole:
        return Qt::AlignmentFlag::AlignCenter;

    case Qt::DisplayRole:
        return task.Field(field);

    case Qt::BackgroundRole:
        return GetBackgroundColor(task, field);

    default:
        return QVariant();
    }
}

// Get background color for given field from task. (Could implement a whole TaskStatus and TaskType system, since the api exposes the data)
QColor TaskTableModel::GetBackgroundColor(const TaskUtils::Task& task, const TaskUtils::TaskField& field) const
{
    // Select color based on field   
    QColor color;

    if (field == TaskUtils::Fields::TaskStatus)
    {
        color = QColor(task.Data().value("task_status_color").toString());
    }
    else if (field == TaskUtils::Fields::TaskType)
    {
        color = QColor(task.Data().value("task_type_color").toString());
    }
    else
    {
        color = Qt::transparent;
    }

    // Modify color based on system theme settings
     Qt::ColorScheme colorScheme = qApp->styleHints()->colorScheme();

    if (colorScheme == Qt::ColorScheme::Dark)
    {
        return color.darker();
    }

    else if (colorScheme == Qt::ColorScheme::Light)
    {
        return color.lighter();
    }

    return Qt::transparent;
}

// --- Task Table Filter Proxy ---
// The filter proxy to sort, and filter the TableView display thourhg the TaskModel, based on user selected filters from TaskFiler.
TaskTableFilterProxy::TaskTableFilterProxy(TaskUtils::FilterMap* userFilter, QObject* parent)
    : QSortFilterProxyModel(parent), m_userFilter(userFilter) {}

// Test value against FieldFilter values from TaskFilter::m_currentFilter
bool TaskTableFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    if (!sourceModel())
        return false;

    for (auto it = m_userFilter->constBegin(); it != m_userFilter->constEnd(); ++it)
    {
        const TaskUtils::TaskField& taskField = it.key();

        // If TaskField is set to not filterable skip.
        if (!taskField.filterable)
        {
            continue;
        }

        // Get index 
        QModelIndex index = sourceModel()->index(sourceRow, taskField.index, sourceParent);

        // it.value() returns the TaskUtils::FieldFilter
        if (!index.isValid() || !m_checkCriterion(index, it.value()))
            return false;
    }
    return true;
}

// Verify that the cell value is in user selected filter.
bool TaskTableFilterProxy::m_checkCriterion(const QModelIndex& index, const TaskUtils::FieldFilter& values) const
{
    QString value = index.data(Qt::DisplayRole).toString();
    return values.contains(value);
}

// To update, just invalidate filter, as the pointer to the filter is already a member.
void TaskTableFilterProxy::RefreshFilter()
{
    invalidateFilter();
}

// --- Task Table ---
TaskTable::TaskTable(QWidget* parent): QTableView(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setSelectionMode(QAbstractItemView::MultiSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(true);
    sortByColumn(0, Qt::DescendingOrder);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}
