#include <qtthreadedsql.h>

#include <QCoreApplication>
#include <QTimer>

using namespace QtThreadedSql;

int main(int argc, char *argv[])
{
    QCoreApplication app { argc, argv };

    QTimer::singleShot(0, qApp, [ ] {
        auto connector = new DBConnector(qApp);
        connector->deleteLater();
    });

    return app.exec();
}
