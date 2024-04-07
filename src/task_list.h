#ifndef TASK_LIST_H
#define TASK_LIST_H

#include <QAbstractTableModel>
#include <QVector>
#include <Qt>
#include <QColor>
#include <QTableView>
#include <QSortFilterProxyModel>

#include "task_utils.h"

// --- Task Table Model ---
class TaskTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TaskTableModel(TaskUtils::TaskList* taskList, QObject* parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QColor GetBackgroundColor(const TaskUtils::Task& task, const TaskUtils::TaskField& field) const;

public slots:
    void RefreshModel();

private:
    TaskUtils::TaskList* m_taskList;
};

// --- Task Table Filter Proxy ---
class TaskTableFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    TaskTableFilterProxy(TaskUtils::FilterMap* userFilter, QObject* parent = nullptr);

public slots:
    void RefreshFilter();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    bool m_checkCriterion(const QModelIndex& index, const TaskUtils::FieldFilter& values) const;

    TaskUtils::FilterMap* m_userFilter;
};

// --- Task table ---
class TaskTable : public QTableView
{
    Q_OBJECT

public:
    TaskTable(QWidget* parent = nullptr);
};

#endif // TASK_LIST_H