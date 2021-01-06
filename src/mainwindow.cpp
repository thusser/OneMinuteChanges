#include <QDebug>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <algorithm>
#include <cmath>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "models.h"

MainWindow::MainWindow(QSqlDatabase *db, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), db(db)
{
    ui->setupUi(this);

    // signals/slots
    connect(ui->tableChords->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::chordPair_selected);
    connect(&timer, &QTimer::timeout, this, &MainWindow::timerUpdate);

    // initial update
    updateChordList();
    updateChordTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initDatabase()
{

}

void MainWindow::updateChordTable()
{
    // get all chords
    auto chords = Chord::list();
    QStringList chordNames;
    foreach (auto chord, chords) {
        chordNames.append(chord.name);
    }

    // get current
    auto current = selectedPair();
    auto currentSelection = QTableWidgetSelectionRange(0, 0, 0, 0);

    // set row/col counts
    ui->tableChords->setRowCount(chordNames.length() - 1);
    ui->tableChords->setColumnCount(chordNames.length() - 1);

    // set headers
    ui->tableChords->setHorizontalHeaderLabels(chordNames.mid(1));
    ui->tableChords->setVerticalHeaderLabels(chordNames);

    // loop rows and cols
    int row = 0, col = 0;
    foreach (auto rowChord, chords.mid(0, chords.length() - 1)) {
        // reset col
        col = 0;
        foreach (auto colChord, chords.mid(1, chords.length() - 1)) {
            // create item
            QTableWidgetItem *item = new QTableWidgetItem();

            // duplicate?
            if (row > col) {
                // disable it
                item->setBackground(Qt::black);
            }
            else {
                // get chord pair
                auto minMaxIds = std::minmax({rowChord.id, colChord.id});
                ChordPair pair = ChordPair::getOrCreate(minMaxIds.first, minMaxIds.second);

                // current?
                if (current.id == pair.id) {
                    currentSelection = QTableWidgetSelectionRange(row, col, row, col);
                }

                // get all counts
                auto counts = pair.counts();

                // what did we get?
                if (counts.length() > 0) {
                    // set text
                    auto count = counts.last().count;
                    item->setText(QString::number(count));

                    // color
                    if (count > 60)
                        item->setBackground(QColor::fromRgb(47, 87, 47));
                    else if (count > 40)
                        item->setBackground(QColor::fromRgb(99, 99, 59));
                    else if (count > 20)
                        item->setBackground(QColor::fromRgb(100, 70, 28));
                    else
                        item->setBackground(QColor::fromRgb(100, 41, 38));
                }

                // store chords
                item->setData(Qt::UserRole, pair.id);
            }

            // set it
            ui->tableChords->setItem(row, col, item);

            // next column
            col++;
        }

        // next row
        row++;
    }

    // adjust size
    ui->tableChords->resizeColumnsToContents();
    ui->tableChords->resizeRowsToContents();

    // select current
    ui->tableChords->clearSelection();
    ui->tableChords->setRangeSelected(currentSelection, true);
}

void MainWindow::updateChordList()
{
    // clear list
    ui->listChords->clear();

    // get all chords
    foreach (auto chord, Chord::list()) {
        ui->listChords->addItem(chord.name);
    }
}

void MainWindow::updateHistory()
{
    // clear history
    ui->tableHistory->setRowCount(0);

    // get counts
    auto pair = selectedPair();
    auto counts = pair.counts();

    // got something?
    if (counts.length() > 0) {
        // adjust table
        ui->tableHistory->setRowCount(counts.length());

        // add them
        int row = 0;
        foreach (auto count, counts) {
            // items
            ui->tableHistory->setItem(row, 0, new QTableWidgetItem(count.time.toString()));
            auto item = new QTableWidgetItem(QString::number(count.count));
            item->setData(Qt::UserRole, count.id);
            ui->tableHistory->setItem(row, 1, item);

            // next row
            row++;
        }

        // adjust size
        ui->tableHistory->resizeColumnToContents(0);
    }

    // plot
    updatePlot();
}

ChordPair MainWindow::selectedPair()
{
    // get item
    auto item = ui->tableChords->currentItem();
    if (!item)
        return ChordPair::empty();

    // get pair
    return ChordPair::getById(item->data(Qt::UserRole).toInt());
}

void MainWindow::startTimer()
{
    // get pair
    auto pair = selectedPair();
    if (pair.isEmpty())
        return;

    // start, change button text
    ui->buttonStart->setText("Stop");

    // disable GUI
    ui->frameChords->setEnabled(false);
    ui->frameHistory->setEnabled(false);

    // start timer
    timerStart = QDateTime::currentDateTime();
    timer.start(200);
}

void MainWindow::stopTimer(bool ask)
{
    // stop timer
    timer.stop();

    // stop, change button text
    ui->buttonStart->setText("Start");
    ui->labelTimer->clear();

    // enable GUI
    ui->frameChords->setEnabled(true);
    ui->frameHistory->setEnabled(true);

    // ask for adding?
    if (ask) {
        on_buttonAddHistory_clicked();
    }
}

void MainWindow::updatePlot()
{
    // get selected pair and counts
    auto pair = selectedPair();
    if (pair.isEmpty())
        return;
    auto counts = pair.counts();

    // get now and minimum x
    auto now = QDateTime::currentDateTime();
    float minX = 0, maxY = 0;

    // get data
    int n = counts.length();
    QVector<double> x(n), y(n);
    for (int i=0; i<n; ++i)
    {
        x[i] = -counts[i].time.secsTo(now) / 86400.;
        if (x[i] < minX)
            minX = x[i];
        y[i] = counts[i].count;
        if (y[i] > maxY)
            maxY = y[i];
    }

    // create graph and assign data to it:
    ui->plotHistory->addGraph();
    ui->plotHistory->graph(0)->setData(x, y);
    ui->plotHistory->graph(0)->setLineStyle(QCPGraph::lsNone);
    ui->plotHistory->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::blue, Qt::blue, 5));

    // give the axes some labels:
    ui->plotHistory->xAxis->setLabel("Days from now");
    ui->plotHistory->yAxis->setLabel("Count");

    // set axes ranges, so we see all data:
    ui->plotHistory->xAxis->setRange(minX * 1.1, 0);
    ui->plotHistory->yAxis->setRange(0, maxY * 1.1);
    ui->plotHistory->replot();
}

void MainWindow::on_buttonAddChord_clicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "New chord", "Enter name for new chord:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        // add chord
        Chord::getOrCreate(name);

        // update gui
        updateChordList();
        updateChordTable();
    }
}

void MainWindow::on_buttonRemoveChord_clicked()
{
    // get item
    auto item = ui->listChords->currentItem();
    if (!item)
        return;

    // get chord name
    auto chordName = item->text();

    // ask
    auto r = QMessageBox::question(this, "Delete chord", QString("Really delete chord \"%1\"?").arg(chordName), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (r == QMessageBox::Yes) {
        // remove chord
        Chord::remove(chordName);

        // update gui
        updateChordList();
        updateChordTable();
    }
}

void MainWindow::chordPair_selected()
{
    // get selected pair
    auto pair = selectedPair();

    // something?
    if (pair.id == -1) {
        // no, remove label
        ui->labelChords->clear();

        // disable gui
        ui->frameHistory->setEnabled(false);
        ui->buttonStart->setEnabled(false);
    }
    else {
        // yes, set label
        ui->labelChords->setText(QString("%1 <-> %2").arg(pair.chord1().name).arg(pair.chord2().name));

        // enable gui
        ui->frameHistory->setEnabled(true);
        ui->buttonStart->setEnabled(true);
    }

    // update history
    updateHistory();
}

void MainWindow::on_buttonAddHistory_clicked()
{
    bool ok;
    int count = QInputDialog::getInt(this, "New count", "Enter number of changes:", 0, 0, 1000, 1, &ok);
    if (ok) {
        // get pair
        auto pair = selectedPair();

        // add history
        ChordCount::create(pair.id, count);

        // update gui
        updateChordTable();
        updateHistory();
    }
}

void MainWindow::on_buttonRemoveHistory_clicked()
{
    // get item
    auto item = ui->tableHistory->item(ui->tableHistory->currentRow(), 1);
    if (!item)
        return;

    // ask
    auto r = QMessageBox::question(this, "Delete count", QString("Really delete count?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (r == QMessageBox::Yes) {
        // get count id
        auto id = item->data(Qt::UserRole).toInt();

        // remove count
        ChordCount::remove(id);

        // update gui
        updateChordTable();
        updateHistory();
    }
}

void MainWindow::on_buttonStart_clicked()
{
    // start or stop
    if (ui->buttonStart->text() == "Start")
        startTimer();
    else
        stopTimer();
}

void MainWindow::timerUpdate()
{
    // diff to now
    auto diff = timerStart.msecsTo(QDateTime::currentDateTime()) / 1000.;
    auto duration = 60;

    // what do we show?
    if (diff > duration) {
        stopTimer(true);
    }
    else if (diff > 5) {
        ui->labelTimer->setText(QString::number(duration - diff, 'f', 1) + QString("s"));
    }
    else if (diff > 3) {
        ui->labelTimer->setText("GO!");
    }
    else {
        ui->labelTimer->setText(QString::number(floor(4 - diff), 'f', 0));
    }
}

