#include <QSqlQuery>
#include <QVariant>
#include <QDebug>

#include "models.h"


Chord::Chord(int id, const QString &name) : id(id), name(name)
{

}

const QList<Chord> Chord::list()
{
    // get all chord names
    QList<Chord> chords;
    QSqlQuery query;
    if (query.exec("SELECT id, name FROM chord ORDER BY name")) {
        while(query.next()) {
            Chord chord(query.value(0).toInt(), query.value(1).toString());
            chords.append(chord);
        }
    }
    return chords;
}

const Chord Chord::getOrCreate(QString name)
{
    // try to find it
    QSqlQuery query;
    query.prepare("SELECT id FROM chord WHERE name=:name");
    query.bindValue(":name", name);
    if (query.exec() && query.first()) {
        // found it
        return Chord(query.value(0).toInt(), name);
    }

    // couldn't find it, create new one
    query.prepare("INSERT INTO chord (name) VALUES (:name)");
    query.bindValue(":name", name);
    query.exec();
    return Chord(query.lastInsertId().toInt(), name);
}

const Chord Chord::getById(int id)
{
    // try to find it
    QSqlQuery query;
    query.prepare("SELECT name FROM chord WHERE id=:id");
    query.bindValue(":id", id);
    if (query.exec() && query.first()) {
        // found it
        return Chord(id, query.value(0).toString());
    }
    return Chord::empty();
}

bool Chord::remove(QString name)
{
    // try to find it
    QSqlQuery query;
    query.prepare("DELETE FROM chord WHERE name=:name");
    query.bindValue(":name", name);
    return query.exec();
}

const Chord Chord::empty()
{
    return Chord(-1, "");
}

ChordPair::ChordPair(int id, int chord1_id, int chord2_id) : id(id), chord1_id(chord1_id), chord2_id(chord2_id)
{

}

const QList<ChordCount> ChordPair::counts()
{
    return ChordCount::listForPair(id);
}

const ChordPair ChordPair::empty()
{
    return ChordPair(-1, -1, -1);
}

const ChordPair ChordPair::getById(int id)
{
    // try to find it
    QSqlQuery query;
    query.prepare("SELECT chord1_id, chord2_id FROM chordpair WHERE id=:id");
    query.bindValue(":id", id);
    if (query.exec() && query.first()) {
        // found it
        return ChordPair(id, query.value(0).toInt(), query.value(1).toInt());
    }
    return ChordPair::empty();
}

const ChordPair ChordPair::getOrCreate(int chord1_id, int chord2_id)
{
    // try to find it
    QSqlQuery query;
    query.prepare("SELECT id FROM chordpair WHERE chord1_id=:id1 AND chord2_id=:id2");
    query.bindValue(":id1", chord1_id);
    query.bindValue(":id2", chord2_id);
    if (query.exec() && query.first()) {
        // found it
        return ChordPair(query.value(0).toInt(), chord1_id, chord2_id);
    }

    // couldn't find it, create new one
    query.prepare("INSERT INTO chordpair (chord1_id, chord2_id) VALUES (:id1, :id2)");
    query.bindValue(":id1", chord1_id);
    query.bindValue(":id2", chord2_id);
    if (query.exec()) {
        return ChordPair(query.lastInsertId().toInt(), chord1_id, chord2_id);
    }
    return ChordPair::empty();
}

ChordCount::ChordCount(int id, int chords_id, QDateTime time, int count) : id(id), chords_id(chords_id), count(count), time(time)
{

}

const QList<ChordCount> ChordCount::listForPair(int pair_id)
{
    // get all counts for pair
    QList<ChordCount> counts;
    QSqlQuery query;
    query.prepare("SELECT id, time, count FROM chordcount WHERE chords_id=:id ORDER BY time ASC");
    query.bindValue(":id", pair_id);
    if (query.exec()) {
        while(query.next()) {
            ChordCount count(query.value(0).toInt(), pair_id, query.value(1).toDateTime(), query.value(2).toInt());
            counts.append(count);
        }
    }
    return counts;
}

ChordCount ChordCount::create(int pair_id, int count)
{
    // create count
    QSqlQuery query;
    query.prepare("INSERT INTO chordcount (chords_id, time, count) VALUES (:id, :time, :count)");
    query.bindValue(":id", pair_id);
    auto time = QDateTime::currentDateTime();
    query.bindValue(":time", time);
    query.bindValue(":count", count);
    query.exec();
    return ChordCount(query.lastInsertId().toInt(), pair_id, time, count);
}

bool ChordCount::remove(int id)
{
    // try to find it
    QSqlQuery query;
    query.prepare("DELETE FROM chordcount WHERE id=:id");
    query.bindValue(":id", id);
    return query.exec();
}
