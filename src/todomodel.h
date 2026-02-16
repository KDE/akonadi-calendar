/*
  SPDX-FileCopyrightText: 2008 Thomas Thrainer <tom_t@gmx.at>
  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "akonadi-calendar_export.h"

#include <Akonadi/ETMCalendar>
#include <Akonadi/IncidenceChanger>
#include <Akonadi/Item>

#include <Akonadi/EntityTreeModel>
#include <KCalendarCore/Todo>

#include <KExtraColumnsProxyModel>

#include <memory>

class QMimeData;

namespace Akonadi
{
class TodoModelPrivate;

/*! Expands an IncidenceTreeModel by additional columns for showing todos. */
class AKONADI_CALENDAR_EXPORT TodoModel : public KExtraColumnsProxyModel
{
    Q_OBJECT

public:
    /*! This enum defines all columns this model provides */
    enum {
        SummaryColumn = 0,
        RecurColumn,
        PriorityColumn,
        PercentColumn,
        StartDateColumn,
        DueDateColumn,
        CategoriesColumn,
        DescriptionColumn,
        CalendarColumn,
        CompletedDateColumn,
        ColumnCount // Just for iteration/column count purposes. Always keep at the end of enum.
    };

    /*! This enum defines the user defined roles of the items in this model */
    enum {
        TodoRole = Akonadi::EntityTreeModel::UserRole + 1,
        TodoPtrRole,
        IsRichTextRole,
        SummaryRole,
        RecurRole,
        PriorityRole,
        PercentRole,
        StartDateRole,
        DueDateRole,
        CategoriesRole,
        DescriptionRole,
        CalendarRole,
    };

    /*!
     * Creates a new TodoModel.
     */
    explicit TodoModel(QObject *parent = nullptr);

    /*!
     * Destroys the TodoModel.
     */
    ~TodoModel() override;

    /*!
     * Returns the number of columns for the given parent index.
     */
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /*!
     * Sets the source model.
     */
    void setSourceModel(QAbstractItemModel *sourceModel) override;

    /*!
     * Returns the data stored under the given role for the item referred to by the index.
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    /*!
     * Returns data for the extra column.
     */
    [[nodiscard]] QVariant extraColumnData(const QModelIndex &parent, int row, int extraColumn, int role = Qt::DisplayRole) const override;

    /*!
     * Sets the role data for the item at index to value.
     */
    [[nodiscard]] bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    /*!
     * Returns the data for the given role and section in the header.
     */
    [[nodiscard]] QVariant headerData(int column, Qt::Orientation orientation, int role) const override;

    /*!
     * Sets the calendar to be used.
     */
    AKONADI_CALENDAR_DECL_DEPRECATED_TEXT("Setting calendar is no longer necessary.")
    void setCalendar(const Akonadi::ETMCalendar::Ptr &calendar);

    /*!
     * Sets the incidence changer.
     */
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer);

    /*!
     * Returns the MIME data of the given indexes.
     */
    [[nodiscard]] QMimeData *mimeData(const QModelIndexList &indexes) const override;

    /*!
     * Handles the drop of MIME data.
     */
    [[nodiscard]] bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    /*!
     * Returns the MIME types supported by this model.
     */
    [[nodiscard]] QStringList mimeTypes() const override;

    /*!
     * Returns the drop actions supported by this model.
     */
    [[nodiscard]] Qt::DropActions supportedDropActions() const override;

    /*!
     * Returns the item flags for the given index.
     */
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    /*!
     * Returns the model's role names.
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    /*! Emitted when dropMimeData() rejected a drop
     *  on the same item or any of its children.
     */
    void dropOnSelfRejected();

private:
    friend class TodoModelPrivate;
    std::unique_ptr<TodoModelPrivate> const d;
};
}
