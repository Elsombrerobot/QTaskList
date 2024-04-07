#ifndef TASK_FILTER_H
#define TASK_FILTER_H

#include<QWidget>
#include<QMap>
#include<QSet>
#include<QCheckBox>
#include<QToolButton>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QLabel>
#include<QList>
#include<QScrollArea>
#include<QPushButton>

#include "task_utils.h"

// --- TaskFieldFilter ---
class TaskFieldFilter : public QWidget
{
	Q_OBJECT

public:
	explicit TaskFieldFilter(const TaskUtils::TaskField& taskField, QWidget* parent = nullptr);
	void SetFilterValues(TaskUtils::FieldFilter& fieldFilter);
	TaskUtils::FieldFilter* GetFilterValues();
	void SetExpanded(bool expanded);
	const TaskUtils::TaskField& taskField;

private:
	QCheckBox* m_selectAll;
	QToolButton* m_hideButton;
	QLabel* m_label;
	QVBoxLayout* m_layout;
	QVBoxLayout* m_valuesLayout;
	QHBoxLayout* m_labelLayout;

	QList<QCheckBox*> m_filterValueList;

	void m_EmptyFilterValues();

signals:
	void SelectedValuesChanged(TaskUtils::FieldFilter*);

private slots:
	void m_HandleFilterChangedByUser();
};

// --- TaskFilter ---
class TaskFilter : public QWidget
{
	Q_OBJECT

public:
	explicit TaskFilter(QWidget* parent = nullptr);
	static TaskUtils::FilterMap DeduceFieldFilterMapFromTaskList(TaskUtils::TaskList* taskList);
	TaskUtils::FilterMap currentFilter;

private:
	QVBoxLayout* m_layout;
	QVBoxLayout* m_filtersLayout;
	QHBoxLayout* m_buttonsLayout;
	QScrollArea* m_scrollArea;
	QWidget* m_scrollWidget;
	QPushButton* m_collapseButton;
	QPushButton* m_expandButton;

	QList<TaskFieldFilter*> m_fieldFilterWidgetList;

signals:
	void SelectedFiltersChanged(TaskUtils::FilterMap*);

private slots:
	void m_HandleTaskListReady(TaskUtils::TaskList* taskList);
	void m_SetFilterMap(TaskUtils::FilterMap& fieldFilters);
	void m_SetCurrentFilter();
	void m_InitializeFilters();
};

#endif // TASK_FILTER_H
 