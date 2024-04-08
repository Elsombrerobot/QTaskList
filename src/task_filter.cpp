#include<QWidget>
#include<QMap>
#include<QObject>
#include<QSet>
#include<QCheckBox>
#include<QToolButton>
#include<QVBoxLayout>
#include<QHBoxLayout>
#include<QLabel>
#include<QList>
#include<QScrollArea>
#include<QPushButton>
#include<QPalette>
#include<QColor>
#include<Qt>

#include "task_utils.h"
#include "task_filter.h"

// --- TaskFieldFilter ---
// Widget to selected values to filter one TaskField.
TaskFieldFilter::TaskFieldFilter(const TaskUtils::TaskField& taskField, QWidget* parent)
	: QWidget(parent), taskField(taskField)
{
	m_layout = new QVBoxLayout(this);
	m_valuesLayout = new QVBoxLayout();
	m_labelLayout = new QHBoxLayout();

	// Hide unhide tool button
	m_hideButton = new QToolButton(this);
	m_hideButton->setCheckable(true);
	m_hideButton->setArrowType(Qt::RightArrow);

	// Label for field filter display
	m_label = new QLabel(taskField.displayName, this);
	QFont font = m_label->font();
	font.setBold(true);
	m_label->setFont(font);

	// Select all checkbox
	m_selectAll = new QCheckBox("Select all", this);
	m_selectAll->setChecked(true);
	m_selectAll->setVisible(false);

	// Layouts
	m_labelLayout->addWidget(m_hideButton);
	m_labelLayout->addWidget(m_label);
	m_valuesLayout->addWidget(m_selectAll);
	m_layout->addLayout(m_labelLayout);
	m_layout->addLayout(m_valuesLayout);

	// On hide button toggled, hide/unhide every values.
	connect(m_hideButton, &QAbstractButton::toggled, this, [this](bool visible)
	{
		// Set the checkbox visibility
		m_selectAll->setVisible(visible);
		for (QCheckBox* checkbox : m_filterValueList) {
			checkbox->setVisible(visible);
		}

		// Set the icon for the button
		Qt::ArrowType arrow = visible ? Qt::DownArrow : Qt::RightArrow;
		m_hideButton->setArrowType(arrow);
		
	});

	// On select all state change, set every value to values.
	connect(m_selectAll, SIGNAL(toggled(bool)), this, SLOT(m_HandleFilterChangedByUser()));
}

void TaskFieldFilter::m_HandleFilterChangedByUser()
{
	// Get sender checkbox
	QCheckBox* sender = qobject_cast<QCheckBox*>(QObject::sender());

	// Select all if sender is select all checkbox
	if (sender == m_selectAll)
	{
		for (QCheckBox* checkbox : m_filterValueList)
		{
			// Block sinal to avoid recursive calls
			checkbox->blockSignals(true);
			checkbox->setChecked(m_selectAll->isChecked());
			checkbox->blockSignals(false);
		}
	}

	// Emit newly selected values
	emit SelectedValuesChanged(GetFilterValues());
}

// Set given Set of values as checkboxes
void TaskFieldFilter::SetFilterValues(TaskUtils::FieldFilter& fieldFilter)
{
	// Sort the QSet
	QStringList sortedFieldFilter(fieldFilter.begin(), fieldFilter.end());
	sortedFieldFilter.sort();

	// Delete previous value checkboxes
	m_EmptyFilterValues();

	// Set the new ones
	for (const QString& value : sortedFieldFilter) {
		QCheckBox* tmp_checkbox = new QCheckBox(value, this);

		// Set it at the same state of select all checkbox.
		tmp_checkbox->setChecked(m_selectAll->isChecked());

		// Connect checked signals to filter emitting function
		connect(tmp_checkbox, SIGNAL(toggled(bool)), this, SLOT(m_HandleFilterChangedByUser()));

		// Set visibility based on hide button state.
		tmp_checkbox->setVisible(m_hideButton->isChecked());

		// Add to the widget.
		m_valuesLayout->addWidget(tmp_checkbox);

		// Keep track of them for next clean up.
		m_filterValueList.append(tmp_checkbox);
	}

	// Reset the filters te be all selected.
	m_selectAll->setChecked(true);
}

// Get checked values for that field.
TaskUtils::FieldFilter* TaskFieldFilter::GetFilterValues()
{
	TaskUtils::FieldFilter* values = new TaskUtils::FieldFilter;

	for (QCheckBox* checkbox : m_filterValueList)
	{
		if (checkbox->isChecked())
		{
			values->insert(checkbox->text());
		}
	}

	return values;
}

// Collapse / Expand function
void TaskFieldFilter::SetExpanded(bool expanded)
{
	m_hideButton->setChecked(expanded);
}

// Empty the values for this task field filter, so new one can be inserted.
void TaskFieldFilter::m_EmptyFilterValues()
{
	// hide and delete checkboxes
	for (QCheckBox* checkbox : m_filterValueList)
	{
		checkbox->hide();
		delete checkbox;
	}

	// Clear pointers to old Checkboxes
	m_filterValueList.clear();
};

// --- TaskFilter ---
// Widget that holds all the TaskFieldFilter widgets, and hold a current filter for user selected options.
TaskFilter::TaskFilter(QWidget* parent)
	: QWidget(parent)
{
	// Filters scrolls area
	m_layout = new QVBoxLayout(this);
	m_scrollWidget = new QWidget(this);
	m_filtersLayout = new QVBoxLayout(m_scrollWidget);
	m_filtersLayout->setAlignment(Qt::AlignTop);
	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// Expand collpase buttons
	m_buttonsLayout = new QHBoxLayout();
	m_collapseButton = new QPushButton("Collapse", this);
	m_expandButton = new QPushButton("Expand", this);
	m_buttonsLayout->addWidget(m_collapseButton);
	m_buttonsLayout->addWidget(m_expandButton);

	m_scrollArea->setWidget(m_scrollWidget);
	m_layout->addLayout(m_buttonsLayout);
	m_layout->addWidget(m_scrollArea);
	m_InitializeFilters();

	// Connect button collpase
	connect(m_collapseButton, &QPushButton::clicked, this, [this]()
	{
		// Loop trhough initiliazed TaskFieldFilter 
		for (TaskFieldFilter* fieldFilterWidget : m_fieldFilterWidgetList)
		{
			fieldFilterWidget->SetExpanded(false);
		}
	});

	// Connect button expand
	connect(m_expandButton, &QPushButton::clicked, this, [this]()
		{
			// Loop trhough initiliazed TaskFieldFilter 
			for (TaskFieldFilter* fieldFilterWidget : m_fieldFilterWidgetList)
			{
				fieldFilterWidget->SetExpanded(true);
			}
		});
}

// Tasklist is ready, deduce filter values and initialize corresponding widgets.
void TaskFilter::m_HandleTaskListReady(TaskUtils::TaskList* taskList)
{
	TaskUtils::FilterMap filterMap = DeduceFieldFilterMapFromTaskList(taskList);
	m_SetFilterMap(filterMap);
}

// Deduce filters for each field defined in TaskFieldIndexMap.
TaskUtils::FilterMap TaskFilter::DeduceFieldFilterMapFromTaskList(TaskUtils::TaskList* taskList)
{
	// Initialize filter map
	TaskUtils::FilterMap filterMap;

	// Loop through all defined fields
	for (const TaskUtils::TaskField& taskField : TaskUtils::Fields::TaskFieldIndexMap)
	{
		// If TaskField is set to not filterable skip.
		if (!taskField.filterable)
		{
			continue;
		}

		// Initialize filter for that field
		filterMap[taskField] = {};

		// Populate field filter with every values in the tasks.
		for (const TaskUtils::Task& task : *taskList)
		{
			filterMap[taskField].insert(task.Field(taskField));
		}
	}

	return filterMap;
}

// Create filters base on the TaskFieldIndexMap available fields.
void TaskFilter::m_InitializeFilters()
{
	// Loop trhough defined TaskField, and create a TaskFieldFilter for each one. 
	for (auto i = TaskUtils::Fields::TaskFieldIndexMap.cbegin(), end = TaskUtils::Fields::TaskFieldIndexMap.cend(); i != end; ++i)
	{
		const TaskUtils::TaskField& taskField = i.value();

		// If TaskField is set to not filterable skip initialization.
		if (!taskField.filterable)
		{
			continue;
		}

		TaskFieldFilter* fieldFilterWidget = new TaskFieldFilter(taskField, this);

		// Connect filter values changed, to filter changed, so proxy filter be updated later.
		connect(fieldFilterWidget, 
			SIGNAL(SelectedValuesChanged(TaskUtils::FieldFilter*)),
			this,
			SLOT(m_SetCurrentFilter()));

		m_filtersLayout->addWidget(fieldFilterWidget);
		m_fieldFilterWidgetList.append(fieldFilterWidget);
	}
}

// Populate the widget with filter value options.
void TaskFilter::m_SetFilterMap(TaskUtils::FilterMap& fieldFilters)
{
	// Loop trhough initiliazed TaskFieldFilter 
	for (TaskFieldFilter* fieldFilterWidget : m_fieldFilterWidgetList)
	{
		// Get the list of value deduced from The TaskList, for the corresponding TaskField of the current TaskFieldFilter
		TaskUtils::FieldFilter filterValues = fieldFilters.value(fieldFilterWidget->taskField);

		// Set the new option values inside the TaskFieldFilter
		fieldFilterWidget->SetFilterValues(filterValues);
	}

	// Filter are reset to all selected, set current filter with current value, and emit filter SelectedFiltersChanged.
	m_SetCurrentFilter();
}

// Set the current filter, ie, filter selected by user.
void TaskFilter::m_SetCurrentFilter()
{
	currentFilter.clear();

	// Loop trhough initiliazed TaskFieldFilter , get every values set for each task field
	for (TaskFieldFilter* fieldFilterWidget : m_fieldFilterWidgetList)
	{
		currentFilter[fieldFilterWidget->taskField] = *fieldFilterWidget->GetFilterValues();
	}
	// Emit new filter.
	emit SelectedFiltersChanged(&currentFilter);
}