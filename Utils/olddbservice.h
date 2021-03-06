#ifndef OLDDBSERVICE_H
#define OLDDBSERVICE_H

#include <QString>
#include <QFile>
#include <QTextCodec>


struct OldDbAbonent /// Структура с данными абонента (zk 2.4)
{
    QString famil, imya, otchestvo, klichka;
    QString city, street, dom, korpus, kvar;
    QString tel1,tel2,tel3,tel4,tel5;
    QString dopinfo;
};


struct OldDbZk /// Структура с данными записной канижки (zk 2.4)
{
    OldDbZk();
    ~OldDbZk();

    OldDbAbonent person;    ///< владелец
    QString category;       ///< категория
    QString data_zad, povod_zad, kem_zad;   ///<
    QString keep_place;     ///< место хранения

    QList<OldDbAbonent*> contacts;    ///< контакты их ЗК
};


class OldDbService  ///Класс импорта старых Записных Книг
{
public:
    OldDbService();

   bool Open_Old_db(OldDbZk *zk, int pos, QByteArray arr);

    bool readFromDir(const QString &dirname, QList<OldDbZk*> *listdb);

    bool isOk() const;

private:
    bool _isOk;
//    bool readZkInfo();
    bool readAbonentInfo(QByteArray *arr, int *pos, OldDbAbonent *abonent);

    QString readLexem(QByteArray *arr, int *pos);
};

#endif // OLDDBSERVICE_H
