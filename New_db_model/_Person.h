#ifndef PERSON_H
#define PERSON_H
#include <QString>
#include <QList>
#include "db_connection.h"
#include "_Telephone.h"

struct Adress {
    QString liv;
    QString reg;
    Adress();
    void setData(QString sqlRow);
};


class Person
{
public:
    Person();
    ~Person();
    QString lastname;
    QString name;
    QString midname;
    QString alias;
    QString id;
    DbState state;
    QList<Telephone*> *_telephones;
    Adress country;
    Adress city;
    Adress street;
    Adress house;
    Adress corp;
    Adress flat;

    QList<Telephone *> *telephones();
    static bool selectByEventId(QList<Person*> *personsList, QString eventId);
    static bool updatePerson(Person *person);
    static bool createPerson(Person *person, QString eventId);
    static bool deletePerson(Person *person);

    static bool handleTelephones(QList<Telephone*> *telephones, QString personId);

    static bool linkEventPerson(QString eventId, QString personId);
};

#endif // PERSON_H
