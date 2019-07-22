#include "_Contacts.h"
//////////////////////////////////////////////////////////////////
Contacts::Contacts()
{
    contact_id=0;
    cont_state = IsNewing;
}

/// Конструктор класса с тремя переменными
Contacts::Contacts(int cont_id, QString tel, QString mark, int ot_id, DbState st)
    :contact_id(cont_id), contact_tel_num(tel), mark(mark)
{
    parent_OT_id = ot_id;
    cont_state = st;
}

Contacts::~Contacts()
{
    qDebug()<<"delete contact"<<contact_id;
}


bool Contacts::selectAll(QList<Contacts*> *list)
{
    if(list==nullptr)
        return false;

    qDeleteAll(*list);
    list->clear();

    if( !db_connection::instance()->db_connect() )
        return false;

    QSqlQuery temp(db_connection::instance()->db());
    temp.prepare("SELECT "
                 "contacts.contact_list_id,"
                 "contacts.cl_telephone,"
                 "contacts.cl_info, "
                 "contacts.FK_Cl_telephone"
                    " FROM contacts");
    if (!temp.exec())
    {
        qDebug() << temp.lastError();
        return false;
    }

    while (temp.next())
    {
        Contacts *cnt = new Contacts(temp.value(0).toInt(), temp.value(1).toString(), temp.value(2).toString(),temp.value(3).toInt(), IsReaded);
        list->append(cnt);
    }

    return true;
}

bool Contacts::saveAll_cont(QList<Contacts*> *list, int new_tel_id)
{
    if(list==nullptr || !db_connection::instance()->db_connect())
        return false;

    QString cname = db_connection::instance()->db().connectionName();

    bool isOk = db_connection::instance()->db().database(cname).transaction();

    for(int i=0;i<list->size();i++)
    {
        if ( !isOk )
            break;

     if (!list->at(i)->contact_tel_num.isEmpty())
        switch(list->at(i)->cont_state)
        {
        case IsNewing:
                isOk = list->at(i)->insert(false, new_tel_id);
            break;
        case IsChanged:
            isOk = list->at(i)->update(false);
            break;
        case IsRemoved:
            isOk = list->at(i)->remove();
            break;
        }
    }

    if(!isOk)
    {
        db_connection::instance()->db().database(cname).rollback();
        qDebug() << "отсюда";
        return false;
    }
    db_connection::instance()->db().database(cname).commit();

    for(int i=list->size()-1;i>=0;i--)
    {
        if( list->at(i)->cont_state==IsRemoved)
        {
            delete list->at(i);
            list->removeAt(i);
        }
        else
            list->at(i)->cont_state = IsReaded;
    }

    return true;
}

bool Contacts::selectTelContacts(QList<Contacts *> *list, int tel_id)
{
    if(list==0)
        return false;

    qDeleteAll(*list);
    list->clear();

    if( !db_connection::instance()->db_connect() )
        return false;

    QSqlQuery temp(db_connection::instance()->db());

    temp.prepare("SELECT "
                 "contacts.contact_list_id,"
                 "contacts.cl_telephone,"
                 "contacts.cl_info,"
                 "contacts.FK_Cl_telephone "
                  " FROM contacts "
                  "WHERE contacts.FK_Cl_telephone = (:id) ");
    temp.bindValue(":id",tel_id);
    if (!temp.exec())
    {
        qDebug() << temp.lastError();
        return false;
    }

    while (temp.next())
    {
            Contacts *cnt = new Contacts(temp.value(0).toInt(), temp.value(1).toString(), temp.value(2).toString(), temp.value(3).toInt(), IsReaded);
            list->append(cnt);
    }
}

bool Contacts::selectContactsforEdit(QList<Contacts *> *list, int)
{
    selectAll(list);
    for (int i = 0; i<list->size(); i++)
    {
        qDebug() << list->at(i)->contact_tel_num + " " + list->at(i)->mark;
    }
}

bool Contacts::insert(bool setState, int new_tel_id)
{
    if( !db_connection::instance()->db_connect() )
        return false;

    if (contact_tel_num.count() < 16) //номер введен неполностью
        return false;
    else {
        QString temp = contact_tel_num.at(1)+contact_tel_num.mid(3,3)+
                contact_tel_num.mid(7,3)+
                contact_tel_num.mid(11,2)+contact_tel_num.mid(14,2);
        contact_tel_num = temp;
    }

    QSqlQuery temp(db_connection::instance()->db());
    temp.prepare("INSERT INTO contacts( cl_telephone, cl_info, FK_Cl_telephone) VALUES ( (:tel_num), (:mark), (:fk_id)) RETURNING Contact_list_id");
    temp.bindValue(":tel_num",contact_tel_num);
    temp.bindValue(":mark",mark);
    temp.bindValue(":fk_id", new_tel_id);

    if (!temp.exec())
    {
        db_connection::instance()->lastError = temp.lastError().text();
        qDebug() << temp.lastError();
        return false;
    }
    if (temp.next())
    {
        qDebug()<<temp.executedQuery();
        contact_id = temp.value(0).toInt();
        qDebug() << " contact add" + QString::number(contact_id) << mark;
            if( setState )
                cont_state = IsReaded;
            return true;
    }
        db_connection::instance()->lastError = temp.lastError().text();
        return false;
}

bool Contacts::update(bool setState)
{
    if( !db_connection::instance()->db_connect() )
        return false;

    if (contact_tel_num.count() < 16) //номер введен неполностью
        return false;
    else {
        QString temp = contact_tel_num.at(1)+contact_tel_num.mid(3,3)+
                contact_tel_num.mid(7,3)+
                contact_tel_num.mid(11,2)+contact_tel_num.mid(14,2);
        contact_tel_num = temp;
    }

    QSqlQuery temp(db_connection::instance()->db());
    temp.prepare("UPDATE contacts SET cl_telephone = (:cl_tel), "
                            "cl_info = (:cl_info) "
                            " WHERE "
                            " Contact_list_id = (:id)");
      temp.bindValue(":cl_tel", contact_tel_num);
      temp.bindValue(":cl_info", mark);
      temp.bindValue(":id", contact_id);
    if (!temp.exec())
    {
        db_connection::instance()->lastError = temp.lastError().text();
        qDebug() << temp.lastError();
        return false;
    }

    qDebug() << "update" + QString::number(contact_id);
    if(setState)
        cont_state = IsReaded;
    return true;
}

bool Contacts::remove()
{
    if( !db_connection::instance()->db_connect() )
        return false;

    QSqlQuery temp(db_connection::instance()->db());
   temp.prepare("DELETE FROM contacts WHERE Contact_list_id = (:id)");
   temp.bindValue(":id",contact_id);

    if (!temp.exec())
    {
        db_connection::instance()->lastError = temp.lastError().text();
        qDebug() << temp.lastError();
        return false;
    }

    qDebug() << "delete" + QString::number(contact_id);
    cont_state = IsRemoved;
    return true;
}

void Contacts::check() const
{
    qDebug() << QString::number(contact_id)+" " +contact_tel_num+ " " +mark +" " +QString::number(parent_OT_id);
}
