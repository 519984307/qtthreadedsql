#ifndef QTTHREADEDSQL_H
#define QTTHREADEDSQL_H

#include <QObject>
#include <QThread>

#include <QtSql>
#include <QSqlDatabase>

#include <functional>

namespace QtThreadedSql {

class DBRequest;
class DBQuery;

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

class DBConnection : public QObject
{
    friend class DBConnector;
    Q_OBJECT
    Q_DISABLE_COPY(DBConnection)
public:
    explicit DBConnection(QObject *parent = nullptr);
    virtual ~DBConnection();

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

    void connectToDatabase();
    void disconnectFromDatabase();

    DBQuery *createQuery();
signals:
    void ready();
    void error();
private:
    void start();
    void exec(DBQuery *);
    void finish();
private:
    ConnectInfo m_info;
    ConnectInfo m_currentInfo;
    QThread m_thread;
    QObject m_worker;
    QSqlDatabase m_db;
    QString m_connectionName;
};

class DBRequest : public QObject
{
    friend class DBConnection;
    Q_OBJECT
    Q_DISABLE_COPY(DBRequest)
protected:
    bool m_isError = false;
    explicit DBRequest(QObject *parent = nullptr)
        : QObject(parent)
    {

    }
public:
    bool isError() const { return m_isError; }
signals:
    void finished();
};

class DBQuery : public DBRequest
{
    friend class DBConnection;
    Q_OBJECT
    Q_DISABLE_COPY(DBQuery)
    explicit DBQuery(std::function<void(DBQuery *)> callback, QObject *parent = nullptr)
        : DBRequest(parent)
        , m_callback(callback)
    {

    }
public:
    virtual ~DBQuery() { }
    void prepare(const QString &query) { m_query = query; }
    void bindValue(const QString &placeholder, const QVariant &val) { m_bounds[placeholder] = val; }
    void exec() { m_callback(this); }
    const QVector<QVector<QVariant> > &data() const { return m_data; }
private:
    std::function<void(DBQuery *)> m_callback;
    QString m_query;
    QVariantMap m_bounds;
    QVector<QVector<QVariant> > m_data;
};

}

#endif // QTTHREADEDSQL_H
