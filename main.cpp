#include <qtthreadedsql.h>

#include <QCoreApplication>
#include <QTimer>

#include <QDebug>

#include <cstdlib>

using namespace QtThreadedSql;

int main(int argc, char *argv[])
{
    std::srand(std::time(nullptr));

    QCoreApplication app { argc, argv };

    QTimer::singleShot(0, qApp, [ ] {
        auto connector = new DBConnector(qApp);
        connector->setType(QStringLiteral("QSQLITE"));
        connector->setDatabaseName(QStringLiteral("qtthreadedsql.db"));
        auto connection = connector->createConnection(qApp);
        qApp->connect(connection, &DBConnection::ready, qApp, [ connection ] {
            auto create = connection->createQuery();
            create->prepare(QStringLiteral("CREATE TABLE IF NOT EXISTS numbers (number INTEGER)"));
            qApp->connect(create, &DBQuery::finished, qApp, [ connection, create ] {
                create->deleteLater();
                if (create->isError())
                    return;

                auto insert = connection->createQuery();
                insert->prepare(QStringLiteral("INSERT INTO numbers VALUES (:number)"));
                insert->bindValue(QStringLiteral(":number"), std::rand());
                qApp->connect(insert, &DBQuery::finished, qApp, [ insert ] {
                    insert->deleteLater();
                    if (insert->isError())
                        return;
                });
                insert->exec();
            });
            create->exec();
        });
    });

    return app.exec();
}
