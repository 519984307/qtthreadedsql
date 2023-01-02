#ifndef QTTHREADEDSQL_H
#define QTTHREADEDSQL_H

#include <QObject>
#include <QThread>

#include <QtSql>
#include <QSqlDatabase>

namespace QtThreadedSql {

class DBConnection;

struct ConnectInfo
{
    QString type = QStringLiteral("QSQLITE");

    QString databaseName;
    QString userName;
    QString password;
    QString hostName;
    int port = -1;
    QString connectOptions;
    QSql::NumericalPrecisionPolicy precisionPolicy = QSql::LowPrecisionDouble;
};

class DBConnector : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DBConnector)
public:
    explicit DBConnector(QObject *parent = nullptr);
    virtual ~DBConnector();
    DBConnection *createConnection(QObject *parent = nullptr);

    QString type() const { return m_info.type; }
    QString databaseName() const { return m_info.databaseName; }
    QString userName() const { return m_info.userName; }
    QString password() const { return m_info.password; }
    QString hostName() const { return m_info.hostName; }
    int port() const { return m_info.port; }
    QString connectOptions() const { return m_info.connectOptions; }
    QSql::NumericalPrecisionPolicy numericalPrecisionPolicy() const { return m_info.precisionPolicy; }

    void setType(const QString &type) { m_info.type = type; }
    void setDatabaseName(const QString &databaseName) { m_info.databaseName = databaseName; }
    void setUserName(const QString &userName) { m_info.userName = userName; }
    void setPassword(const QString &password) { m_info.password = password; }
    void setHostName(const QString &hostName) { m_info.hostName = hostName; }
    void setPort(int port) { m_info.port = port; }
    void setConnectOptions(const QString &connectOptions) { m_info.connectOptions = connectOptions; }
    void setNumericalPrecisionPolicy(QSql::NumericalPrecisionPolicy precisionPolicy) { m_info.precisionPolicy = precisionPolicy; }

private:
    ConnectInfo m_info;
};

class DBConnection : public QObject
{
    friend class DBConnector;
    Q_OBJECT
    Q_DISABLE_COPY(DBConnection)
    explicit DBConnection(const ConnectInfo &, QObject *parent = nullptr);
public:
    virtual ~DBConnection();

signals:
    void ready();
    void error();
private:
    void start();
private:
    ConnectInfo m_info;
    QThread m_thread;
    QObject m_worker;
    QSqlDatabase m_db;
};

}

#endif // QTTHREADEDSQL_H
