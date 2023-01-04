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
        auto connection = new DBConnection(qApp);
        qApp->connect(connection, &QObject::destroyed, qApp, &QCoreApplication::quit);

        connection->setType(QStringLiteral("QSQLITE"));
        connection->setDatabaseName(QStringLiteral("qtthreadedsql.db"));
        connection->connectToDatabase();
        qApp->connect(connection, &DBConnection::ready, qApp, [ connection ] {
            auto create = connection->createQuery();
            create->prepare(QStringLiteral("CREATE TABLE IF NOT EXISTS numbers (number INTEGER)"));
            create->exec();
            qApp->connect(create, &DBQuery::finished, qApp, [ connection, create ] {
                if (create->isError())
                    return connection->deleteLater();

                auto insert = connection->createQuery();
                insert->prepare(QStringLiteral("INSERT INTO numbers VALUES (:number)"));
                insert->bindValue(QStringLiteral(":number"), std::rand());
                insert->exec();
                qApp->connect(insert, &DBQuery::finished, qApp, [ connection, insert ] {
                    if (insert->isError())
                        return connection->deleteLater();

                    auto select = connection->createQuery();
                    select->prepare(QStringLiteral("SELECT * FROM numbers"));
                    select->exec();
                    qApp->connect(select, &DBQuery::finished, qApp, [ connection, select ] {
                        if (select->isError())
                            return connection->deleteLater();
                        qDebug() << select->data();
                        connection->deleteLater();
                    });
                });
            });
        });
    });

    return app.exec();
}
