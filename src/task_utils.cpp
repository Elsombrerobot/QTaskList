#include<QObject>
#include<QString>
#include<QJsonObject>
#include<Qt>
#include<QColor>

#include "task_utils.h"

namespace TaskUtils
{
	// Base Object data for a task, that will be used to create filters, as well as displaying tasks in the model view.
	Task::Task(QJsonObject taskData)
		:m_data(taskData)
	{
		QString ep = Field(Fields::Episode);
		QString seq = Field(Fields::Sequence);
		QString entity = Field(Fields::Name);
		descriptiveName +=
			(ep.isEmpty() ? "" : ep + " / ") +
			(seq.isEmpty() ? "" : seq + " / ") +
			entity;
	};

	// Return the value for a task field, meaning a value that can be used in the model view and the filter.
	QString Task::Field(const TaskField& field) const
	{
		return GetRaw(field.dataName);
	}

	// Data getter
	const QJsonObject& Task::Data() const
	{
		return m_data;
	}

	// Specific Data getter
	QString Task::GetRaw(QString name) const
	{
		return Data()[name].toString();
	}

}
