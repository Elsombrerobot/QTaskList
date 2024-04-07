#ifndef TASK_UTILS_H
#define TASK_UTILS_H

#include<QObject>
#include<QString>
#include<QJsonObject>
#include<QList>

namespace TaskUtils
{
	// Define the data to be used in mapping later, to get info for filters, and display data.
	// TaskField is not to be instanciated anywhere else than this header. Use Fields or TaskFieldIndexMap as getters.
	struct TaskField
	{
		// Default constructor for QMap
		TaskField()
			: dataName("unkown"),
			displayName("Unkown"),
			tooltip("Unknown task field."),
			index(7),
			filterable(0) {}

		TaskField(
			const QString& dataName,
			const QString& displayName,
			const QString& tooltip,
			const quint8 index,
			const bool filterable
		)
			: dataName(dataName),
			displayName(displayName),
			tooltip(tooltip),
			index(index),
			filterable(filterable) {}

		QString dataName;			// data name inside task data from server.
		QString displayName;		// Name to show in filter and model view.
		QString tooltip;			// Tooltip to show in filter and model view.
		quint8 index;				// Index to show in model view.
		bool filterable;			// Will show in TaskFilter or not.

		// Operators overloading for using TaskField as a key in a QMap.
		bool operator<(const TaskField& other) const {
			return index < other.index;
		}

		bool operator==(const TaskField& other) const {
			return index == other.index;
		}
	};

	// Fields to be used inside filter and view.
	namespace Fields
	{
		const TaskField Project(
			"project_name",
			"Project",
			"The production related to the task.",
			0,
			1
		);

		const TaskField Entity(
			"entity_type_name",
			"For",
			"The entity type related to the task, Asset or Shot.",
			1,
			1
		);

		const TaskField Episode(
			"episode_name",
			"Episode",
			"The episode related to the task.",
			2,
			1
		);

		const TaskField Sequence(
			"sequence_name",
			"Sequence",
			"The episode related to the task.",
			3,
			1
		);

		const TaskField Name(
			"entity_name",
			"Name",
			"Name of the task.",
			4,
			0
		);

		const TaskField TaskType(
			"task_type_name",
			"Task type",
			"The type of the task.",
			5,
			1
		);

		const TaskField TaskStatus(
			"task_status_name",
			"Task status",
			"The status of the task.",
			6,
			1
		);

		// TaskField map, by index, for looping and getting purposes.
		const QMap<quint8, TaskField> TaskFieldIndexMap {
			{ 0, Project },
			{ 1, Entity },
			{ 2, Episode },
			{ 3, Sequence },
			{ 4, Name },
			{ 5, TaskType },
			{ 6, TaskStatus },
		};
	}

	// Object wrapper around task data returned by the api.
	class Task
	{
	public:
		Task(QJsonObject taskData);

		// Specific field getter.
		QString Field(const TaskField& field) const;

		// Data getter
		const QJsonObject& Data() const;

	private:
		QJsonObject m_data;
	};

	// Typedef for task stack 
	typedef QList<Task> TaskList;

	// A list of values to be used with a TaskField to filter a the Task model with.
	typedef QSet<QString> FieldFilter;

	// A complete filter for every TaskField to be used by the proxy filter, to filter everything.
	typedef QMap<TaskUtils::TaskField, FieldFilter> FilterMap;
};

#endif // TASK_UTILS_H
