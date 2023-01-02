#include "qtthreadedsql.h"

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
    m_thread.start();
}

DBConnection::~DBConnection()
{
    m_thread.exit();
    m_thread.wait();
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
