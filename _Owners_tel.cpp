#include "_Owners_tel.h"

Owners_tel::Owners_tel()
{
}

Owners_tel::Owners_tel(int t_d): tel_id(t_d)
{
}

Owners_tel::Owners_tel(int zk, int id, DbState st)
{//////// Конструктор для формы добавления////////
    tel_id = id;
    parentZK_id = zk;
    state = st;
}

Owners_tel::Owners_tel(int id, QString l, QString n, QString m_n, QString b_d, QString r_c, QString r_s, QString r_h, QString r_cor, QString r_f, QString d_a, QString t_n):
    Crud(id,l, n, m_n, b_d, r_c, r_s, r_h, r_cor,r_f, d_a),
    tel_num(t_n)
{           ///Конструктор поиска

}

Owners_tel::Owners_tel(QString t_n, int t_id, int zk, DbState st):
    tel_id (t_id), tel_num(t_n), parentZK_id(zk)
{
    state = st;
}

Owners_tel::~Owners_tel()
{
  qDebug()<<"delete telephone"<<tel_id;
}

bool Owners_tel::selectAll(QList<Owners_tel *> *list)
{
    if(list==nullptr)
        return false;

    qDeleteAll(*list);
    list->clear();

    if( !db_connection::instance()->db_connect() )
        return false;

    QSqlQuery temp(db_connection::instance()->db);
    temp.prepare("SELECT \"owners_tel\".\"Telephone_num\" "
                           "FROM  \"owners_tel\"");
    if (!temp.exec())
    {
        qDebug() << temp.lastError();
        return false;
    }

    while (temp.next())
    {
        //Owners_tel *ot = new Owners_tel(temp.value(0).toString(),IsReaded);
        //list->append(ot);
    }

    for (int i = 0; i<list->size(); i++)
    {
        //QString str = QString::number(list->at(i)->contact_id);
        qDebug() << list->at(i)->tel_num;
    }

    return true;
}

bool Owners_tel::saveAll(QList<Owners_tel *> *list)
{
    if(list==0 || !db_connection::instance()->db_connect())
        return false;

    QString cname = db_connection::instance()->db.connectionName();

    bool isOk = db_connection::instance()->db.database(cname).transaction();

    for(int i=0;i<list->size();i++)
    {
        if ( !isOk )
            break;

        switch(list->at(i)->state)
        {
        case IsNewing:
            if ( ! list->at(i)->tel_num.isEmpty())
                isOk = list->at(i)->insert(false);
            else
                isOk = list->at(i)->remove();
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
        db_connection::instance()->db.database(cname).rollback();
        qDebug() << "отсюда";
        return false;
    }
    db_connection::instance()->db.database(cname).commit();

    for(int i=list->size()-1;i>=0;i--)
    {
        if( list->at(i)->state==IsRemoved)
        {
            delete list->at(i);
            list->removeAt(i);
        }
        else
            list->at(i)->state = IsReaded;
    }

    return true;
}

bool Owners_tel::selectZkTel(QList<Owners_tel *> *list, int zk)
{
    if(list==nullptr)
        return false;

    qDeleteAll(*list);
    list->clear();

    if( !db_connection::instance()->db_connect() )
        return false;

    QSqlQuery temp(db_connection::instance()->db);
    temp.prepare("SELECT \"owners_tel\".\"Telephone_num\", \"owners_tel\".\"Telephone_id\", \"owners_tel\".\"FK_Telephone_Zk\" "
                 "FROM  \"owners_tel\""
                 "WHERE \"owners_tel\".\"FK_Telephone_Zk\" = (:id)");
    temp.bindValue(":id", zk);
    if (!temp.exec())
    {
        qDebug() << temp.lastError();
        return false;
    }

    while (temp.next())
    {
        Owners_tel *ot = new Owners_tel(temp.value(0).toString(), temp.value(1).toInt(), temp.value(2).toInt(),IsReaded);
        list->append(ot);
    }
    return true;
}

//////// Вместо вызова списка, создаю пустой для формы добавления ///////////
bool Owners_tel::selectZkTelForAdd(QList<Owners_tel *> *list, int zk)
{
    if(list==nullptr)
        return false;

    qDeleteAll(*list);
    list->clear();

    if( !db_connection::instance()->db_connect() )
        return false;

    QSqlQuery temp(db_connection::instance()->db);
    temp.prepare("SELECT \"owners_tel\".\"Telephone_num\", \"owners_tel\".\"Telephone_id\", \"owners_tel\".\"FK_Telephone_Zk\" "
                 "FROM  \"owners_tel\""
                 "WHERE \"owners_tel\".\"FK_Telephone_Zk\" = (:id)");
    temp.bindValue(":id", zk);
    if (!temp.exec())
    {
        qDebug() << temp.lastError();
        return false;
    }

    while (temp.next())
    {
        Owners_tel *ot = new Owners_tel(temp.value(0).toString(), temp.value(1).toInt(), temp.value(2).toInt(),IsReaded);
        list->append(ot);
    }
    /// Если у зк нет телефонов при редактировании
    if(list->isEmpty())
    {
        temp.prepare("SELECT MAX( \"owners_tel\".\"Telephone_id\")"
                     "FROM \"owners_tel\"");
        if (!temp.exec())
        {
            qDebug() << temp.lastError();
            return false;
        }
        while (temp.next())
        {
        //////создаю модель с двумя - id и FK///////
        Owners_tel *ot = new Owners_tel(zk,temp.value(0).toInt() +1 ,IsNewing);
        list->append(ot);
       return true;
        }
    }
}

bool Owners_tel::insert(bool setState)
{
    if( !db_connection::instance()->db_connect() && tel_num.isEmpty())
        return false;

    QSqlQuery temp(db_connection::instance()->db);
    temp.prepare("INSERT INTO \"owners_tel\"( \"Telephone_num\", \"FK_Telephone_Zk\") "
                 "VALUES ( (:tel_num),(:fk_id)) RETURNING \"Telephone_id\"");
    temp.bindValue(":tel_num",tel_num);
    temp.bindValue(":fk_id",parentZK_id);

    if (!temp.exec())
    {
        db_connection::instance()->lastError = temp.lastError().text();
        db_connection::instance()->error = temp.lastError();
        db_connection::instance()->error_tel_num = tel_num;
        qDebug() << temp.lastError();
        return false;
    }
    if (temp.next())
    {
        qDebug()<<temp.executedQuery();
        tel_id = temp.value(0).toInt();
        qDebug() << "add" + QString::number(tel_id);
            if( setState )
                state = IsReaded;
            return true;
    }
        db_connection::instance()->lastError = temp.lastError().text();
        db_connection::instance()->error = temp.lastError();
        return false;
}

bool Owners_tel::update(bool setState)
{
    if( !db_connection::instance()->db_connect() )
        return false;

    QSqlQuery temp(db_connection::instance()->db);
    temp.prepare("UPDATE \"owners_tel\" SET \"Telephone_num\" = (:tel_num)"
                            " WHERE \"Telephone_id\" = (:tel_id)");
    temp.bindValue(":tel_id", tel_id);
    temp.bindValue(":tel_num", tel_num);

    if (!temp.exec())
    {
        db_connection::instance()->lastError = temp.lastError().text();
        qDebug() << temp.lastError();
        return false;
    }

    qDebug() << "update" + QString::number(tel_id);
    if(setState)
        state = IsReaded;
    return true;
}

bool Owners_tel::remove()
{
    if( !db_connection::instance()->db_connect() )
        return false;

    QSqlQuery temp(db_connection::instance()->db);
   temp.prepare("DELETE FROM \"owners_tel\" "
                "WHERE \"Telephone_id\" = (:id)");
   temp.bindValue(":id",tel_id);

    if (!temp.exec())
    {
        db_connection::instance()->lastError = temp.lastError().text();
        qDebug() << temp.lastError();
        return false;
    }

    qDebug() << "delete" + QString::number(tel_id);
    state = IsRemoved;
    return true;
}

//////////////////////////////////////////////////////////////////////


void Owners_tel::zk_search()
{
    search_is_ready = false;
    model_is_ready = false; //Зануляем флаги

    msgbx.setText("Результат поиска");
    msgbx.setGeometry(960,510, 180,210);

    QString qry;
    if(zk_id != 0)
    {
        QString temp = QString::number(zk_id);
        qry += " AND \"zk\".\"Zk_id\" = "+temp+" ";
    }

    if (!tel_num.isEmpty())
    {
       qry += "AND \"owners_tel\".\"Telephone_num\" = ('"+tel_num+"')"
               "AND \"owners_tel\".\"FK_Telephone_Zk\" = \"zk\".\"Zk_id\" ";
    }

    if (!lastname.isEmpty())
    {
        qry += " AND LOWER(\"zk\".\"Lastname\") LIKE LOWER('"+lastname+"%') ";
    }
    if (!name.isEmpty())
    {
        qry += " AND LOWER(\"zk\".\"Name\") LIKE LOWER('"+name+"%')";
    }
    if (!mid_name.isEmpty())
    {
        qry += " AND LOWER(\"zk\".\"Mid_name\") LIKE LOWER('"+mid_name+"%')";
    }

    if(birth_date.count() > 2)
    {
        if (birth_date.at(0)=="." && birth_date.at(1) == ".")
        {
            birth_date.remove(0,2); /// Убираю точки
            qry += "AND regexp_replace(\"zk\".\"Birth_date\", '\\s+$', '') LIKE ('______"+birth_date+"%')" ;
        }
            else
        {
            if (birth_date.at(0) == ".")
            {
                qry += "AND regexp_replace(\"zk\".\"Birth_date\", '\\s+$', '') LIKE ('__"+birth_date+"%')" ;
            }
            else {
                birth_date.chop(1); ///убираю одну точку
                qry += "AND regexp_replace(\"zk\".\"Birth_date\", '\\s+$', '') LIKE ('"+birth_date+"%')" ;
            }
        }
    }

    if (!reg_city.isEmpty())
    {
        qry += " AND LOWER(\"zk\".\"Reg_city\") LIKE LOWER('"+reg_city+"%')";
    }
    if (!reg_street.isEmpty())
    {
        qry += " AND LOWER(\"zk\".\"Reg_street\" ) LIKE LOWER('"+reg_street+"%')";
    }

    if (!reg_home.isEmpty())
    {
        qry += " AND \"zk\".\"Reg_home\" =('"+reg_home+"')";
    }

    if (!reg_corp.isEmpty())
    {
        qry += "AND \"zk\".\"Reg_corp\" = ('"+reg_corp+"')";
    }
    if (!reg_flat.isEmpty())
    {
        qry += " AND \"zk\".\"Reg_flat\" = ('"+reg_flat+"')";
    }

    if(date_add.count()>2)
    {
        if (date_add.at(0)=="-" && date_add.at(1) == "-")
        {
            date_add.remove(0,2); /// Убираю точки
            qry += "AND regexp_replace(\"zk\".\"Date_add\", '\\s+$', '') LIKE ('______"+date_add+"%')" ;
        }
        else
        {
            if (date_add.at(0) == "-")
            {
                qry += "AND regexp_replace(\"zk\".\"Date_add\", '\\s+$', '') LIKE ('__"+date_add+"%')" ;
            }
            else {
                date_add.chop(1); ///убираю одну точку
                qry += "AND regexp_replace(\"zk\".\"Date_add\", '\\s+$', '') LIKE ('"+date_add+"%')" ;
            }
        }
    }

    if( Date_From.isValid() || Date_To.isValid() )
    {
        QString tempp;
        querry.prepare("SELECT \"zk\".\"Date_add\", \"zk\".\"Zk_id\""
                       "FROM \"zk\""
                       "WHERE \"zk\".\"Date_add\" IS NOT NULL");

        if (!querry.exec())
            qDebug() << querry.lastError();

        while (querry.next())
        {
            QDate date;
            auto list =  querry.value(0).toString().split('-');
            date.setDate(list.at(0).toInt(),list.at(1).toInt(),list.at(2).toInt());

            if( (Date_From.isValid() && Date_To.isValid() && Date_From < Date_To) &&
                  (  date <= Date_To && date >= Date_From )) /// Если две даты
            {
                if(tempp.isEmpty())
                {
                    tempp+=" \"zk\".\"Zk_id\" = "+querry.value(1).toString();
                }
                else
                {
                  tempp+=" OR  \"zk\".\"Zk_id\" = "+querry.value(1).toString();
                }
            }
            else
            { /// Если есть дата С
                if( (Date_From.isValid() && !Date_To.isValid()) && date >= Date_From)
                {
                    if(tempp.isEmpty())
                    {
                        tempp+=" \"zk\".\"Zk_id\" = "+querry.value(1).toString();
                    }
                    else
                    {
                      tempp+=" OR  \"zk\".\"Zk_id\" = "+querry.value(1).toString();
                    }
                }
                else
                {
                    if( ( Date_To.isValid() && !Date_From.isValid() ) && date <= Date_To )
                    {/// Если есть дата ПО
                        if(tempp.isEmpty())
                        {
                            tempp+=" \"zk\".\"Zk_id\" = "+querry.value(1).toString();
                        }
                        else
                        {
                          tempp+=" OR  \"zk\".\"Zk_id\" = "+querry.value(1).toString();
                        }
                    }
//                    else /// Если мы вошли в это условие и не прошли ни по одному параметру
//                    {///месседжбокс с ошибкой
//                        msgbx.setText("<font size = '8'>Дата введена некорректно!</font>");
//                        msgbx.setStandardButtons(QMessageBox::Cancel);
//                        msgbx.setButtonText(QMessageBox::Cancel,"Закрыть");
//                        int ret = msgbx.exec();
//                        switch (ret) {
//                        case QMessageBox::Cancel:
//                            return;
//                       }
//                    }
                }
            }
        }
        if (!tempp.isEmpty())
        {
            qry+="AND ("+tempp+" ) ";
        }

    }

if (!qry.isEmpty())
  {
    querry.prepare("SELECT "
                   "COUNT(DISTINCT \"zk\".\"Zk_id\")"
                   "FROM "
                   " \"zk\",\"owners_tel\" "
                   "WHERE "
                   "\"zk\".\"Zk_id\">0 " + qry);//qry - набор данных для поиска
    if (!querry.exec())
        qDebug() << querry.lastError();
    qDebug() << querry.executedQuery();
    while (querry.next())
    {
        search_result = querry.value(0).toString(); //определил кол-во результатов
    }
    querry.clear();


    if (search_result.toInt() == 0)
    {
         msgbx.setText("<font size = '8'>Совпадений не найдено</font>");
         msgbx.setStandardButtons(QMessageBox::Cancel);
         msgbx.setButtonText(QMessageBox::Cancel,"Закрыть");
         int ret = msgbx.exec();
         switch (ret) {
         case QMessageBox::Cancel:
             break;
                 }
    }
    else {
        msgbx.setText("<font size = '8'> Найдено " + search_result + " совпадений </font>");
        msgbx.setStandardButtons(QMessageBox::Ok | QMessageBox::Open | QMessageBox::Cancel);
        msgbx.setButtonText(QMessageBox::Ok,"ПРОСМОТР");
        msgbx.setButtonText(QMessageBox::Open,"СФОРМИРОВАТЬ СПРАВКУ");
        msgbx.setButtonText(QMessageBox::Cancel,"ОТМЕНА");
    }

    int ret = msgbx.exec();

    switch (ret) {
    case QMessageBox::Ok:
        Owners_tel::zk_search_model(qry);
        break;

    case QMessageBox::Open:
        Owners_tel::zk_search_report(qry);
        break;

    case QMessageBox::Cancel:
        break;
            }
        }
        else
    {
    Crud::select_all();
}
}

void Owners_tel::zk_search_model(QString qry)
{
    querry.prepare("SELECT  DISTINCT"        //стринг для модели
                   "\"zk\".\"Zk_id\","
                   "\"zk\".\"Lastname\", "
                   "\"zk\".\"Name\", "
                   "\"zk\".\"Mid_name\", "
                   "\"zk\".\"Birth_date\","
                    "\"zk\".\"Reg_city\","
                   "\"zk\".\"Reg_street\","
                   "\"zk\".\"Reg_home\","
                   "\"zk\".\"Reg_corp\","
                   "\"zk\".\"Reg_flat\","
                   "\"zk\".\"Check_for\", "
                   "\"zk\".\"Dop_info\","
                   "\"zk\".\"Date_add\", "
                   "\"zk\".\"Time_add\""
                   "FROM "
                   "\"zk\" ,\"owners_tel\""
                   "WHERE"
                   "\"zk\".\"Zk_id\">0 " + qry + " ORDER BY \"zk\".\"Zk_id\"");
    if (!querry.exec())
        qDebug() << querry.lastError();
     model->setQuery(querry);

    if(model->rowCount()>0)
        model_is_ready = true;

    querry.clear();
}

void Owners_tel::zk_search_report(QString qry)
{
    search_res.clear();

    querry.prepare("SELECT  DISTINCT"
                   "\"zk\".\"Zk_id\","
                    "regexp_replace(\"zk\".\"Lastname\", '\\s+$', ''), "
                    "regexp_replace(\"zk\".\"Name\", '\\s+$', ''), "
                    "regexp_replace(\"zk\".\"Mid_name\", '\\s+$', ''),"
                    "regexp_replace(\"zk\".\"Birth_date\", '\\s+$', ''),"

                   "regexp_replace(\"zk\".\"Reg_city\", '\\s+$', ''),"
                   "regexp_replace(\"zk\".\"Reg_street\", '\\s+$', ''),"
                   "regexp_replace(\"zk\".\"Reg_home\", '\\s+$', ''),"
                   "regexp_replace(\"zk\".\"Reg_corp\", '\\s+$', ''),"
                   "regexp_replace(\"zk\".\"Reg_flat\", '\\s+$', ''),"

                   "regexp_replace(\"zk\".\"Check_for\", '\\s+$', ''),"
                   "regexp_replace(\"zk\".\"Dop_info\", '\\s+$', ''),"
                   "\"zk\".\"Date_add\", "
                   "\"zk\".\"Time_add\""
                   "FROM "
                   "\"zk\" ,\"owners_tel\""
                   "WHERE"
                   "\"zk\".\"Zk_id\">0 " + qry + " ORDER BY \"zk\".\"Zk_id\"");
    if (!querry.exec())
        qDebug() << querry.lastError();
    while (querry.next())
    {
        search_res+="Записная книжка № "+querry.value(0).toString()+" \r\n Владелец: "+
                querry.value(1).toString()+" "+querry.value(2).toString()+" "+querry.value(3).toString()+
                ", "+querry.value(4).toString()+", адрес регистрации: г."+
                querry.value(5).toString()+" ул. "+querry.value(6).toString()+" д. "+
                querry.value(7).toString()+" к. "+querry.value(8).toString()+" кв. "+
                querry.value(9).toString();
        temp.prepare("SELECT \"owners_tel\".\"Telephone_num\""
                      "FROM \"owners_tel\""
                      "WHERE \"owners_tel\".\"FK_Telephone_Zk\" = (:id)");
        temp.bindValue(":id", querry.value(0).toInt());
        if (!temp.exec())
            qDebug() << temp.lastError();
        else
            search_res +=", телефон:";
        while (temp.next())
        {
         search_res +=" "+temp.value(0).toString()+", ";
        }
       search_res +="окраска: ???, дополнительная информация: "+querry.value(11).toString()+
               ", записная книжка сформирована "+querry.value(12).toString()+
               " в интересах: " +querry.value(10).toString()+"\r\n \r\n";
    }

      if(!search_res.isEmpty())
    {
       search_is_ready = true;
      }
}

void Owners_tel::check_tel_num(QString result)
{
    stop = false;
    drop = false;
    querry.prepare("SELECT \"owners_tel\".\"FK_Telephone_Zk\""
                   "FROM \"owners_tel\""
                   "WHERE \"owners_tel\".\"Telephone_num\" = ('" + result+"')");
   if(!querry.exec())
       qDebug() << querry.lastError();

    qDebug() << querry.executedQuery();
   while (querry.next())
   {
        if(stop == false && drop == false)
        {
            msgbx.setGeometry(960,510, 180,210);
            msgbx.setText("Предупреждение!");
            msgbx.setText("<font size = '8'>Введенный вами телефон: "+result+" принадлежит владельцу ЗК № "+querry.value(0).toString()+" </font>");
            msgbx.setStandardButtons(QMessageBox::Ok | QMessageBox::Open | QMessageBox::Cancel);
            msgbx.setButtonText(QMessageBox::Ok,"Перейти к записной книжке № "+querry.value(0).toString());
            msgbx.setButtonText(QMessageBox::Open,"Редактировать телефонный номер");
            msgbx.setButtonText(QMessageBox::Cancel,"Закрыть карточку без сохранения");
            int ret = msgbx.exec();

            switch (ret) {
            case QMessageBox::Ok:
            zk_id = querry.value(0).toInt();
                stop = true;
                break;

            case QMessageBox::Open:
                break;

            case QMessageBox::Cancel:
                drop = true;
                break;
            }

        }
   }
}

