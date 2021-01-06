#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QTimer>
#include <QDateTime>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QSqlDatabase *db, QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QTimer timer;
    QDateTime timerStart;
    QSqlDatabase *db;

    void initDatabase();
    void updateChordTable();
    void updateChordList();
    void updateHistory();
    void startTimer();
    void stopTimer(bool ask = false);
    void updatePlot();

private slots:
    void on_buttonAddChord_clicked();
    void on_buttonRemoveChord_clicked();
    void chordPair_selected();
    void on_buttonAddHistory_clicked();
    void on_buttonRemoveHistory_clicked();
    void on_buttonStart_clicked();
    void timerUpdate();
};
#endif // MAINWINDOW_H
