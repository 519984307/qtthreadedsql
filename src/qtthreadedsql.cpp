#include "qtthreadedsql.h"

#include <QMapIterator>

using namespace QtThreadedSql;

DBConnector::DBConnector(QObject *parent)
    : QObject(parent)
{

}

DBConnector::~DBConnector()
{

}

DBConnection *DBConnector::createConnection(QObject *parent)
{
    return new DBConnection(m_info, parent);
}

DBConnection::DBConnection(const ConnectInfo &info, QObject *parent)
    : QObject(parent)
    , m_info(info)
{
    m_worker.moveToThread(&m_thread);
    connect(&m_thread, &QThread::started, &m_worker, [ this ] { start(); });
    QTimer::singleShot(0, this, [ this ] {
        m_thread.start();
    });
}

DBConnection::~DBConnection()
{
    m_thread.exit();
    m_thread.wait();
}

DBQuery *DBConnection::createQuery()
{
    const auto callback = [ this ](DBQuery *query) {
        QTimer::singleShot(0, this, [ this, query ] {
            QTimer::singleShot(0, &m_worker, [ this, query ] { exec(query); });
        });
    };
    return new DBQuery(callback, this);
}

void DBConnection::start()
{
    const auto thread = QThread::currentThread();
    const auto name = QStringLiteral("QtThreadedSql-%1").arg(reinterpret_cast<qulonglong>(thread), 0, 16);

    m_db = QSqlDatabase::addDatabase(m_info.type, name);

    m_db.setDatabaseName(m_info.databaseName);
    m_db.setUserName(m_info.userName);
    m_db.setPassword(m_info.password);
    m_db.setHostName(m_info.hostName);
    m_db.setPort(m_info.port);
    m_db.setConnectOptions(m_info.connectOptions);
    m_db.setNumericalPrecisionPolicy(m_info.precisionPolicy);

    if (!m_db.open()) {
        emit error();
        return;
    }
    emit ready();
}

void DBConnection::exec(DBQuery *query)
{
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
