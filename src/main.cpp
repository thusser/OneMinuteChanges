#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QSqlDatabase>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string>

#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>


int main(int argc, char *argv[])
{
    // create app
    QApplication app(argc, argv);

    // get home directory
    QDir dir = QDir::home();

    // get or create config directory
    if (!dir.exists(".oneminutechanges")) {
        if (!dir.mkdir(".oneminutechanges")) {
            QMessageBox::critical(NULL, "Error", "Could not create config directory.");
            return 1;
        }
    }

    // move into directory
    if (!dir.cd(".oneminutechanges")) {
        QMessageBox::critical(NULL, "Error", "Could not enter config directory.");
        return 1;
    }

    // get filename for db
    QString filename = dir.absoluteFilePath("database.sqlite");

    // check database
    const QString DRIVER("QSQLITE");
    if (!QSqlDatabase::isDriverAvailable(DRIVER)) {
        QMessageBox::critical(NULL, "Error", "No sqlite driver available.");
        return 1;
    }

    // create database
    QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);
    db.setDatabaseName(filename);

    // open it
    if (!db.open()) {
        QMessageBox::critical(NULL, "Error", "Could not open database.");
        return 1;
    }

    // create tables
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS chord ("
               "  id INTEGER NOT NULL, "
               "  name VARCHAR(20) NOT NULL, "
               "  PRIMARY KEY (id)"
               ");");
    query.exec("CREATE TABLE chordcount ("
               "  id INTEGER NOT NULL,"
               "  chords_id INTEGER, "
               "  time DATETIME NOT NULL, "
               "  count INTEGER, "
               "  PRIMARY KEY (id), "
               "  FOREIGN KEY(chords_id) REFERENCES chordpair (id)"
               ");");
    query.exec("CREATE TABLE chordpair ("
               "  id INTEGER NOT NULL, "
               "  chord1_id INTEGER, "
               "  chord2_id INTEGER, "
               "  PRIMARY KEY (id), "
               "  FOREIGN KEY(chord1_id) REFERENCES chord (id),"
               "  FOREIGN KEY(chord2_id) REFERENCES chord (id)"
               ");");

    // create and show window
    MainWindow wnd(&db);
    wnd.show();
    return app.exec();

}
