#ifndef MODELS_H
#define MODELS_H

#include <QDateTime>
#include <QList>
#include <QString>


class Chord
{
public:
    Chord(int id, const QString &name);

    static const QList<Chord> list();
    static const Chord getOrCreate(QString name);
    static const Chord getById(int id);
    static bool remove(QString name);

    static const Chord empty();

    int id;
    QString name;
};

class ChordCount
{
public:
    ChordCount(int id, int chords_id, QDateTime time, int count);

    static const QList<ChordCount> listForPair(int pair_id);
    static ChordCount create(int pair_id, int count);
    static bool remove(int id);

    int id, chords_id, count;
    QDateTime time;
};

class ChordPair
{
public:
    ChordPair(int id, int chord1_id, int chord2_id);

    inline bool isEmpty() { return id == -1; }
    const QList<ChordCount> counts();
    inline Chord chord1() { return Chord::getById(chord1_id); }
    inline Chord chord2() { return Chord::getById(chord2_id); }

    static const ChordPair empty();
    static const ChordPair getById(int id);
    static const ChordPair getOrCreate(int chord1_id, int chord2_id);

    int id, chord1_id, chord2_id;
};


#endif // MODELS_H
