#include <qtthreadedsql.h>

#include <QCoreApplication>
#include <QTimer>

#include <QDebug>

using namespace QtThreadedSql;

int main(int argc, char *argv[])
{
    QCoreApplication app { argc, argv };

    QTimer::singleShot(0, qApp, [ ] {
        auto connector = new DBConnector(qApp);
        connector->setType(QStringLiteral("QSQLITE"));
        connector->setDatabaseName(QStringLiteral("qtthreadedsql.db"));
        auto connection = connector->createConnection(qApp);
        qApp->connect(connection, &DBConnection::ready, qApp, [ ] { qDebug() << "ready"; });
        qApp->connect(connection, &DBConnection::error, qApp, [ ] { qDebug() << "error"; });
    });

    return app.exec();
}
