#include "db_connection.h"


db_connection *db_connection::_instance = 0;

db_connection::db_connection()
{
    type =PSQLtype;
    _dbpg = QSqlDatabase::addDatabase("QPSQL","conPQ");
    ok = _dbpg.isValid();
    _dbsql = QSqlDatabase::addDatabase("QSQLITE","conql");
    _dbsqlchipher = QSqlDatabase::addDatabase("SQLITECIPHER","conqlchipher");
}

db_connection::~db_connection()
{
    if( db().isOpen())
        db().close();
}

db_connection *db_connection::instance()
{
    if( _instance == 0)
        _instance = new db_connection();

    return _instance;
}

bool db_connection::db_connect()
{
    lastError.clear();
   // bool ok;
    if( db().isOpen() )
        return true;
    switch (type)
    {
        case (PSQLtype):
        //db().setDatabaseName("zk");
        //db().setPassword("ipqo");
        //db().setPort(5433);
        _dbpg.setHostName(Settings_connection::instance()->HostName);
        _dbpg.setDatabaseName(Settings_connection::instance()->DatabaseName);
       _dbpg.setUserName(Settings_connection::instance()->User);
        _dbpg.setPassword(Settings_connection::instance()->Password);
       //_dbpg.setPassword("123");
       _dbpg.setPort(Settings_connection::instance()->Port);

       _dbpg.setConnectOptions("client_encoding='UTF8'");

        ok = _dbpg.open();
        if (ok == true)
            qDebug() << "PG: OK";
        break;
    case (SQLliteType):
        ok = _dbsql.open();
        if (ok == true)
            qDebug() <<"SQLliteType: OK";
        else
            qDebug() << "SQLliteType" << _dbsql.lastError().text();
        break;
      case (SQLlitechipher):
        ok = _dbsqlchipher.open();
        if (ok == true)
            qDebug() << "SQLlitechipher: OK";
        else
            qDebug() <<"SQLlitechipher" << _dbsqlchipher.lastError().text();
        break;
    }

    return ok;
}

QSqlDatabase &db_connection::db()
{
    switch (type)
    {
    case (PSQLtype):
         return _dbpg;
    case (SQLliteType):
        return _dbsql;
    case (SQLlitechipher):
        return _dbsqlchipher;
    }
}

void db_connection::set_Sql_type(SqlType t)
{
   type = t;
}
