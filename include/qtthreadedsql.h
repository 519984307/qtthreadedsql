#ifndef QTTHREADEDSQL_H
#define QTTHREADEDSQL_H

#include <QObject>

namespace QtThreadedSql {

class DBConnector : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DBConnector)
public:
    explicit DBConnector(QObject *parent = nullptr);
    virtual ~DBConnector();
};

}

#endif // QTTHREADEDSQL_H
