/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtScxml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qscxmlnulldatamodel.h"
#include "qscxmlevent.h"
#include "qscxmlstatemachine_p.h"
#include "qscxmltabledata.h"

class QScxmlNullDataModel::Data
{
    struct ResolvedEvaluatorInfo {
        bool error;
        QString str;

        ResolvedEvaluatorInfo()
            : error(false)
        {}
    };

public:
    Data(QScxmlNullDataModel *dataModel)
        : q(dataModel)
    {}

    bool evalBool(QScxmlExecutableContent::EvaluatorId id, bool *ok)
    {
        Q_ASSERT(ok);

        ResolvedEvaluatorInfo info;
        Resolved::const_iterator it = resolved.find(id);
        if (it == resolved.end()) {
            info = prepare(id);
        } else {
            info = it.value();
        }

        if (info.error) {
            *ok = false;
            QScxmlStateMachinePrivate::get(q->stateMachine())->submitError(QStringLiteral("error.execution"), info.str);
            return false;
        }

        return q->stateMachine()->isActive(info.str);
    }

    ResolvedEvaluatorInfo prepare(QScxmlExecutableContent::EvaluatorId id)
    {
        auto td = q->tableData();
        const QScxmlExecutableContent::EvaluatorInfo &info = td->evaluatorInfo(id);
        QString expr = td->string(info.expr);
        for (int i = 0; i < expr.size(); ) {
            QChar ch = expr.at(i);
            if (ch.isSpace()) {
                expr.remove(i, 1);
            } else {
                ++i;
            }
        }

        ResolvedEvaluatorInfo resolved;
        if (expr.startsWith(QStringLiteral("In(")) && expr.endsWith(QLatin1Char(')'))) {
            resolved.error = false;
            resolved.str =  expr.mid(3, expr.length() - 4);
        } else {
            resolved.error = true;
            resolved.str =  QStringLiteral("%1 in %2").arg(expr, td->string(info.context));
        }
        return qMove(resolved);
    }

private:
    QScxmlNullDataModel *q;
    typedef QHash<QScxmlExecutableContent::EvaluatorId, ResolvedEvaluatorInfo> Resolved;
    Resolved resolved;
};

/*!
 * \class QScxmlNullDataModel
 * \brief The QScxmlNullDataModel class is the null data model for a Qt SCXML
 * stateMachine
 * \since 5.7
 * \inmodule QtScxml
 *
 * This class implements the null data model as described in the
 * \l {SCXML Specification - B.1 The Null Data Model}. Using the value \c "null"
 * for the \e datamodel attribute of the \c <scxml> element means that there is
 * no underlying data model.
 *
 * \sa QScxmlStateMachine QScxmlDataModel
 */

/*!
 * Creates a new Qt SCXML data model for the state machine \a stateMachine.
 */
QScxmlNullDataModel::QScxmlNullDataModel(QScxmlStateMachine *stateMachine)
    : QScxmlDataModel(stateMachine)
    , d(new Data(this))
{}

/*!
 * Destroys a Qt SCXML data model.
 */
QScxmlNullDataModel::~QScxmlNullDataModel()
{
    delete d;
}

/*!
  \reimp
 */
bool QScxmlNullDataModel::setup(const QVariantMap &initialDataValues)
{
    Q_UNUSED(initialDataValues);

    return true;
}

QString QScxmlNullDataModel::evaluateToString(QScxmlExecutableContent::EvaluatorId id, bool *ok)
{
    // We do implement this, because <log> is allowed in the Null data model,
    // and <log> has an expr attribute that needs "evaluation" for it to generate the log message.
    *ok = true;
    auto td = tableData();
    const QScxmlExecutableContent::EvaluatorInfo &info = td->evaluatorInfo(id);
    return td->string(info.expr);
}

bool QScxmlNullDataModel::evaluateToBool(QScxmlExecutableContent::EvaluatorId id, bool *ok)
{
    return d->evalBool(id, ok);
}

QVariant QScxmlNullDataModel::evaluateToVariant(QScxmlExecutableContent::EvaluatorId id, bool *ok)
{
    Q_UNUSED(id);
    Q_UNUSED(ok);
    Q_UNREACHABLE();
    return QVariant();
}

void QScxmlNullDataModel::evaluateToVoid(QScxmlExecutableContent::EvaluatorId id, bool *ok)
{
    Q_UNUSED(id);
    Q_UNUSED(ok);
    Q_UNREACHABLE();
}

void QScxmlNullDataModel::evaluateAssignment(QScxmlExecutableContent::EvaluatorId id, bool *ok)
{
    Q_UNUSED(id);
    Q_UNUSED(ok);
    Q_UNREACHABLE();
}

void QScxmlNullDataModel::evaluateInitialization(QScxmlExecutableContent::EvaluatorId id, bool *ok)
{
    Q_UNUSED(id);
    Q_UNUSED(ok);
    Q_UNREACHABLE();
}

bool QScxmlNullDataModel::evaluateForeach(QScxmlExecutableContent::EvaluatorId id, bool *ok, ForeachLoopBody *body)
{
    Q_UNUSED(id);
    Q_UNUSED(ok);
    Q_UNUSED(body);
    Q_UNREACHABLE();
    return false;
}

/*!
 * \reimp
 */
void QScxmlNullDataModel::setEvent(const QScxmlEvent &event)
{
    Q_UNUSED(event);
}

/*!
 * \reimp
 * Returns an invalid variant, because the null data model does not support
 * properties.
 */
QVariant QScxmlNullDataModel::property(const QString &name) const
{
    Q_UNUSED(name);
    return QVariant();
}

/*!
 * \reimp
 * Returns \c false, because the null data model does not support properties.
 */
bool QScxmlNullDataModel::hasProperty(const QString &name) const
{
    Q_UNUSED(name);
    return false;
}

/*!
 * \reimp
 * Returns \c false, because the null data model does not support properties.
 */
bool QScxmlNullDataModel::setProperty(const QString &name, const QVariant &value, const QString &context)
{
    Q_UNUSED(name);
    Q_UNUSED(value);
    Q_UNUSED(context);
    return false;
}
