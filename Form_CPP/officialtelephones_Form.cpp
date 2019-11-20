#include "officialtelephones_Form.h"
#include "ui_officialtelephones.h"
#include <QMessageBox>


OfficialTelephones::OfficialTelephones(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OfficialTelephones)
{
    ui->setupUi(this);

}

OfficialTelephones::~OfficialTelephones()
{
    delete ui;
}

void OfficialTelephones::Fill_table()
{
    of_model = new MTM_Off_Tels;
    ofTlist = new QList<Off_tels*>;

    if(Off_tels::selectOffTel(ofTlist))
           of_model->setOffTList(ofTlist);

       //ui->tableView->setModel(of_model);
       m1 = new QSortFilterProxyModel();

       m1->setSourceModel(of_model);
       ui->tableView->setModel(m1);
       ui->tableView->resizeColumnToContents(1);
       ui->tableView->resizeColumnToContents(0);
       ui->tableView->setWordWrap(false);
       ui->tableView->horizontalHeader()->setStretchLastSection(true);
}

void OfficialTelephones::on_pb_add_clicked()
{
    Off_tels *of_t = new Off_tels;
    of_t->service_name = ui->le_set_name->text();
    of_t->tel_num = ui->le_set_num->text();
    if (Off_tels::add_off_tel(of_t))
    {//или просто добавлять указатель в список
//        delete of_model;
//        of_model = nullptr;
//        delete ofTlist;
//        ofTlist = nullptr;

//        Fill_table();

        ofTlist->append(of_t);
         of_model->setOffTList(ofTlist);

         m1->setSourceModel(of_model);
         ui->tableView->setModel(m1);
    }
    else {
        QMessageBox::critical(this,QObject::tr("Ошибка"),QObject::tr("Не удалось добавить служебный телефон!")); ///Хвалимся
    }
}

void OfficialTelephones::on_pb_del_clicked()
{
   QModelIndex index = ui->tableView->currentIndex();
   QModelIndex index1 = m1->mapToSource(index);
   if ( index1.isValid())
   {
       Off_tels *oft = of_model->actofflist.at(index1.row());
      if (Off_tels::del_off_tel(oft))
      {
          ofTlist->removeAt(ofTlist->indexOf(oft));
           of_model->setOffTList(ofTlist);

           m1->setSourceModel(of_model);
           ui->tableView->setModel(m1);
      }
      else
          {
          QMessageBox::critical(this,QObject::tr("Ошибка"),QObject::tr("Не удалось удалить служебный телефон!")); ///Хвалимся
          };
   }
}

void OfficialTelephones::on_pushButton_clicked()
{
    QString tel_num, name;
    tel_num = ui->le_search_num->text();
    name = ui->le_search_name->text();
    //Парсинг
    tel_num.replace("*","%");
    tel_num.replace("?","_");
    name.replace("*","%");
    name.replace("?","_");

    Off_tels::search(ofTlist, tel_num, name);
        of_model->setOffTList(ofTlist);

     m1->setSourceModel(of_model);
     ui->tableView->setModel(m1);
}

void OfficialTelephones::on_pushButton_2_clicked()
{
    if(!ui->le_search_num->text().isEmpty())
        ui->le_search_num->clear();
    if(!ui->le_search_name->text().isEmpty())
        ui->le_search_name->clear();

    Off_tels::selectOffTel(ofTlist);
        of_model->setOffTList(ofTlist);

     m1->setSourceModel(of_model);
     ui->tableView->setModel(m1);

}
