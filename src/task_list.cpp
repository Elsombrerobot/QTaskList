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
#include <QMenu>
#include <QIcon>
#include <QContextMenuEvent>
#include <QModelindex>
#include <QList>

#include "task_utils.h"
#include "task_list.h"

// --- Task Table Model ---
// The model that will be used by a QTableView to get data for the correct roles and display.
TaskTableModel::TaskTableModel(TaskUtils::TaskList* taskList, QObject* parent)
    : QAbstractTableModel(parent), taskList(taskList) {}

 // Return the number of available tasks.
int TaskTableModel::rowCount(const QModelIndex& parent) const
{
    return taskList->size();
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
    switch (role)
    {
    case Qt::DisplayRole:
        return field.displayName;

    case Qt::ToolTipRole:
        return field.tooltip;

    default:
        return QVariant();
    }
}

// Data reading for the table.
QVariant TaskTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !taskList)
    {
        return QVariant();
    }

    // Get basic infos to use in this method
    quint8 row = index.row();        
    quint8 column = index.column(); 

    const TaskUtils::Task& task = taskList->at(row);
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
TaskTable::TaskTable(TaskUtils::TaskList* taskList, TaskUtils::FilterMap* filter, QWidget* parent)
    : QTableView(parent)
{
    // Model and proxy filter model
    taskModel = new TaskTableModel(taskList, this);
    filterProxy = new TaskTableFilterProxy(filter, this);
    filterProxy->setSourceModel(taskModel);
    setModel(filterProxy);

    // Gui setup
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setSelectionMode(QAbstractItemView::MultiSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(true);
    sortByColumn(0, Qt::DescendingOrder);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

// return the list of currently selected tasks.
TaskUtils::TaskConstRefList TaskTable::SelectedTasks()
{
    TaskUtils::TaskConstRefList taskList;

    // Get all the tasks selected
    QList<QModelIndex> selectedRows = selectionModel()->selectedRows();

    for (qsizetype i = 0; i < selectedRows.size(); i++)
    {
        const QModelIndex& index = selectedRows.at(i);

        const TaskUtils::Task& task = m_TaskAt(index);
        taskList.append(&task);
    }

    return taskList;
}

// Get a Task object given an index in the table.
const TaskUtils::Task& TaskTable::m_TaskAt(const QModelIndex& index)
{
    // Get the right index since we are using a proxy filter.
    QModelIndex sourceIndex = filterProxy->mapToSource(index);

    return taskModel->taskList->at(sourceIndex.row());
}

// Emit a custom signals that indicates task menu has been requested.
void TaskTable::contextMenuEvent(QContextMenuEvent* event)
{
    const QModelIndex proxyIndex = indexAt(event->pos());

    // Do not request menu if row under index is not selected. 
    if (!selectionModel()->isSelected(proxyIndex))
    {
        return;
    }
  
    // Get task through index
    const TaskUtils::Task& task = m_TaskAt(proxyIndex);

    // Emit requested context menu.
    emit TaskContextMenuRequested(task, SelectedTasks(), event);
}