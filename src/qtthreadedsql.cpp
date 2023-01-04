#include "qtthreadedsql.h"

#include <QMapIterator>

using namespace QtThreadedSql;

DBConnection::DBConnection(QObject *parent)
    : QObject(parent)
{
    m_worker.moveToThread(&m_thread);
    connect(&m_thread, &QThread::started, &m_worker, [ this ] { start(); });
}

DBConnection::~DBConnection()
{
    disconnectFromDatabase();
}

void DBConnection::connectToDatabase()
{
    m_currentInfo = m_info;
    QTimer::singleShot(0, this, [ this ] {
        m_thread.start();
    });
}

void DBConnection::disconnectFromDatabase()
{
    if (!m_thread.isRunning())
        return;
    QTimer::singleShot(0, &m_worker, [ this ] { finish(); });
    m_thread.wait();
}

DBQuery *DBConnection::createQuery()
{
    const auto callback = [ this ](DBQuery *query) {
        QTimer::singleShot(0, this, [ this, query ] {
            QTimer::singleShot(0, &m_worker, [ this, query ] { exec(query); });
        });
    };

    auto query = new DBQuery(callback, this);
    if (m_autoDeleteQueries)
        connect(query, &DBQuery::finished, query, &QObject::deleteLater);
    return query;
}

void DBConnection::start()
{
    m_connectionName = QStringLiteral("QtThreadedSql-%1").arg(reinterpret_cast<qulonglong>(QThread::currentThread()), 0, 16);

    m_db = QSqlDatabase::addDatabase(m_currentInfo.type, m_connectionName);

    m_db.setDatabaseName(m_currentInfo.databaseName);
    m_db.setUserName(m_currentInfo.userName);
    m_db.setPassword(m_currentInfo.password);
    m_db.setHostName(m_currentInfo.hostName);
    m_db.setPort(m_currentInfo.port);
    m_db.setConnectOptions(m_currentInfo.connectOptions);
    m_db.setNumericalPrecisionPolicy(m_currentInfo.precisionPolicy);

    if (!m_db.open()) {
        emit error();
        return;
    }
    emit ready();
}

void DBConnection::exec(DBQuery *query)
{
    if (!m_db.isOpen())
        return;

    QSqlQuery sql { m_db };
    sql.prepare(query->m_query);

    QMapIterator<QString, QVariant> i { query->m_bounds };
    while (i.hasNext()) {
        i.next();
        sql.bindValue(i.key(), i.value());
    }

    sql.setForwardOnly(true);

    const bool isError = !sql.exec();
    query->m_isError = isError;
    if (isError || !sql.isSelect()) {
        emit query->finished();
        return;
    }

    const int rows = sql.size();
    const int cols = sql.record().count();

    QVector<QVector<QVariant> > data;
    if (rows > 0)
        data.reserve(rows);

    while (sql.next()) {
        QVector<QVariant> row;
        row.reserve(cols);
        for (int i = 0; i < cols; ++i)
            row << sql.value(i);

        data << row;
    }
    query->m_data = data;
    query->m_isError = sql.lastError().isValid();

    emit query->finished();
}

void DBConnection::finish()
{
    if (m_db.isOpen())
        m_db.close();

    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);

    m_thread.quit();
}
