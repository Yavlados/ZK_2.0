#include "mainwindow_Form.h"
#include "ui_mainwindow.h"
#include "db_connection.h"
#include "table_show_delegate.h"
#include "popup.h"

#include <QGuiApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QSqlDatabase>
#include <QSettings>
#include <QKeyEvent>
#include <QDialogButtonBox>
#include <QShortcut>

#include "settings_connection.h"
/**
 * \file Mainwindow_Form.cpp
 * \brief Исполняемый файл класса MainWindow
*/


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //ui->tableView->setFocus();
    set_shortcuts();

    RefreshTab();
    auto tabbar = ui->tabWidget->tabBar();
    tabbar->tabButton(0,QTabBar::RightSide)->deleteLater();
    tabbar->setTabButton(0, QTabBar::RightSide, 0);

    auto tabbar2 = ui->tabWidget_2->tabBar();
    tabbar2->tabButton(0,QTabBar::RightSide)->deleteLater();
    tabbar2->setTabButton(0, QTabBar::RightSide, 0);
//    QDesktopWidget* widget = qApp->desktop();

    ///---Фиксирую размер окна
        showMaximized();
        int difference = this->frameGeometry().height() - this->geometry().height();
        int height = QApplication::desktop()->availableGeometry().height() - difference;
        this->setFixedSize( QApplication::desktop()->screenGeometry().width(), height);
    ///---

//    ui->tableView->selectRow(0);
//    on_tableView_clicked();
    //set_fonts();
    add_splitter_lines();
    //Settings_connection::instance();

    this->size_list = ui->splitter->sizes();
    this->m_c_s = All_unchecked;
    connect(ui->eventTable->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(header_clicked(int)));

    ui->tabWidget->setAttribute(Qt::WA_StyledBackground, true);
    ui->tabWidget_2->setAttribute(Qt::WA_StyledBackground, true);

    set_label();
    set_tab_orders();
    setTableConnections();
}
//-----------------------------------------------------------------------------------//
MainWindow::~MainWindow()
{
    delete ui;
}

//-----------------------------------------------------------------------------------//
void MainWindow::Add_pagination_buttons()
{
    while(ui->button_layout_back->count() != 0)
    {
        QLayoutItem *item = ui->button_layout_back->takeAt(0);
        delete item->widget();
        p_b_back = 0;
    }
    while(ui->button_layout_forward->count() != 0)
    {
        QLayoutItem *item = ui->button_layout_forward->takeAt(0);
        delete item->widget();
        p_b_forward = 0;
    }


     if(this->eventModel->actEventList.size() < this->eventModel->eventList->size())
    {
        if(this->eventModel->eventList->indexOf(this->eventModel->actEventList.at(0)) != 0)
        {
            p_b_back = new QPushButton;
            p_b_back->setText("<<");
            ui->button_layout_back->addWidget(p_b_back);
            connect(p_b_back,SIGNAL(clicked()),this,SLOT(previous_page()));
        }

        if(this->eventModel->eventList->indexOf(this->eventModel->actEventList.at(this->eventModel->actEventList.size()-1))
                < this->eventModel->eventList->size()-1)
        {
            p_b_forward = new QPushButton;
            p_b_forward->setText(">>");
            ui->button_layout_forward->addWidget(p_b_forward);
            connect(p_b_forward,SIGNAL(clicked()),this,SLOT(next_page()));
        }
    }
}
//-----------------------------------------------------------------------------------//
void MainWindow::ShowThisTab(int zk_id) //Открытие main окна и рефреш таблиц
{
    RefreshTab();

    if(zk_id == 0)//форма добавления
    {
        if(addlist != 0)
        {
            delete addlist->at(0);
            delete addlist;
            addlist = 0;
            return;
        }
    }else  //форма редактирования
    {
        if(updlist != 0)
        {
            for (int i=0; i<updlist->size(); i++)
            {
                if(updlist->at(i)->new_cr->zk_id == zk_id)
                {
                    delete  updlist->at(i);
                    updlist->removeAt(i);
                    if (updlist->isEmpty())
                        updlist = 0;
                    return;
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------------//
void MainWindow::RefreshTab()
{
    this->eventModel = new MTM_Event();

    QList<Event*> *list = new QList<Event*>;
    if(Event::selectAll(list)){
        eventModel->setEventList(list);
        ui->eventTable->setModel(eventModel);
    }

        ui->eventTable->setWordWrap(false);
        ui->eventTable->resizeColumnsToContents();

        this->Add_pagination_buttons();
        this->clearLabel();
        this->createLabel();
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_action_add_triggered()
{

    Event *newEvent = new Event();

    editEvent *ee = new editEvent;
    ee->setEventAndType(newEvent, addEvent);
    connect(ee, SIGNAL(addPerson(Person*, editEvent*)), this, SLOT(openAddPersonWindow(Person*, editEvent*)));
    connect(ee, SIGNAL(closeThis(editEvent*)), this, SLOT(closeEditEvent(editEvent*)));

    Util::instance()->editEventList()->append(ee);

    ui->tabWidget->insertTab( ui->tabWidget->count()+1 ,
                              Util::instance()->editEventList()->at(Util::instance()->editEventList()->size()-1),
                              "Создание нового события");
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_action_delete_triggered()
{
    if(this->eventTableIndex.isValid() && this->eventTableIndex == ui->eventTable->currentIndex())
    {
        Event *selectedEvent = this->eventModel->actEventList.at(this->eventTableIndex.row());
        msgbx.setText("Удаление");
        msgbx.setWindowTitle("Удаление");
        msgbx.setInformativeText("Вы действительно хотите удалить выбранное событие № "+selectedEvent->id);
        msgbx.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        int ret = msgbx.exec();

        switch (ret) {
        case QMessageBox::Cancel:
            break;
        case QMessageBox::Ok:
            if (this->eventTableIndex.isValid())
            {
                for(int a=0; a < Util::instance()->addEventManager()->size(); a++){
                    AddEventManager* aem =  Util::instance()->addEventManager()->at(a);
                    if(aem->parent->localEvent->id == selectedEvent->id){
                        for(int i=0; i<ui->tabWidget->count(); i++){
                            auto tab = ui->tabWidget->widget(i);
                            auto tabName = tab->objectName();

                            if(tabName=="editEvent"){
                                editEvent *localEE = dynamic_cast<editEvent*>(tab);
                                if(localEE->localEvent->id == aem->parent->localEvent->id){
                                    ui->tabWidget->widget(i)->deleteLater();

                                    for(int d=0; d < Util::instance()->editEventList()->size(); d++){
                                        if(aem->parent->localEvent->id == Util::instance()->editEventList()->at(d)->localEvent->id){
                                            Util::instance()->editEventList()->removeAt(d);
                                            break;
                                        }
                                    }

                                }
                            }
                            else if(tabName=="EditPerson"){
                                EditPerson *localEP = dynamic_cast<EditPerson*>(tab);
                                for(int c=0; c < aem->childs->size(); c++){
                                    EditPerson *personOnList = aem->childs->at(c);

                                    if(localEP->person->id == personOnList->person->id){
                                        ui->tabWidget->widget(i)->deleteLater();
                                        for(int r=0; r<Util::instance()->editPersonList()->size(); r++){
                                            if(personOnList->person->id == Util::instance()->editPersonList()->at(r)->person->id){
                                                Util::instance()->editPersonList()->removeAt(r);
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }

                }

                Event::deleteEvent(selectedEvent);
                this->RefreshTab();
            }
            break;
        }
    }
    else {
        QMessageBox::critical(this,QObject::tr("Внимание"),QObject::tr("Вы не выбрали событие для удаления!")); ///Хвалимся
    }
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_action_update_triggered()
{
    if(this->eventTableIndex.isValid() &&
       this->eventTableIndex == ui->eventTable->currentIndex()) {
        Event *localEvent = this->eventModel->actEventList.at(this->eventTableIndex.row());
        Person::selectByEventId(localEvent->persons(), localEvent->id);

        for (int i=0; i < Util::instance()->editEventList()->size(); i++)
        {
            if(Util::instance()->editEventList()->at(i)->localEvent->id == localEvent->id)
            {
                ui->tabWidget->setCurrentIndex(i+1);
                return;
            }
        }

        editEvent *ee = new editEvent;
        ee->setEventAndType(localEvent, updateEvent);
        connect(ee, SIGNAL(addPerson(Person*, editEvent*)), this, SLOT(openAddPersonWindow(Person*, editEvent*)));
        connect(ee, SIGNAL(closeThis(editEvent*)), this, SLOT(closeEditEvent(editEvent*)));

        Util::instance()->editEventList()->append(ee);

        ui->tabWidget->insertTab( ui->tabWidget->count()+1 ,
                                  Util::instance()->editEventList()->at(Util::instance()->editEventList()->size()-1),
                                  "Редактирование события №"+localEvent->id);
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
    }
    else {
            QMessageBox::critical(this,QObject::tr("Внимание"),QObject::tr("Вы не выбрали событие для изменения!")); ///Хвалимся
        }
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_action_analysis_triggered()
{
    if (an == 0)
    {
        an = new class Analysis;
        ui->tabWidget_2->insertTab( ui->tabWidget_2->count()+1 ,an,"Анализ");
        ui->tabWidget_2->setCurrentIndex(ui->tabWidget_2->count()-1);
        connect(an, SIGNAL(closeThis(QString)), this, SLOT(findIndexByNameTab2(QString)));
    }
    else
        ui->tabWidget_2->setCurrentIndex( ui->tabWidget_2->indexOf(an));

    set_normal_width(an->actual_size.width());
    an->focus_on_widget();
    an->set_tab_orders();

}
//-----------------------------------------------------------------------------------//
void MainWindow::on_action_search_triggered()
{
    if(sr == 0)
    {
         sr = new Search;
         ui->tabWidget_2->insertTab( ui->tabWidget_2->count()+1 ,sr,"Расширенный поиск");
         ui->tabWidget_2->setCurrentIndex(ui->tabWidget_2->count()-1);
         connect(sr, SIGNAL(Show_search_result(QList<Crud*>*)),this, SLOT(Search_result(QList<Crud*>*)));
         connect(sr,SIGNAL(Cancel_search()),this, SLOT(cancelSearchResults()));
         connect(sr, SIGNAL(closeThis(QString)), this, SLOT(findIndexByNameTab2(QString)));
         connect(sr, SIGNAL(showSearchedEvents(QList<QString>)), this, SLOT(searchedResults(QList<QString>)));

         sr->set_tab_orders();
    }
    else
    {
        ui->tabWidget_2->setCurrentIndex( ui->tabWidget_2->indexOf(sr));
       // sr->set_tab_orders();
    }

    set_normal_width(sr->actual_size.width());
}
//-----------------------------------------------------------------------------------//
void MainWindow::Search_result(QList<Crud*> *crudlist)
{
//    crud_model->setCrudlist(crudlist);
//    ui->tableView->setModel(crud_model);
//    Add_pagination_buttons();
//    MainWindow::add_cancel_button();

//    while(ui->hl_label_crud->count())
//    {
//        QLayoutItem *item = ui->hl_label_crud->takeAt(0);
//        delete item->widget();
//    }

//    QLabel *lb = new QLabel;
//    QString str = "Найдено записей: "+QString::number(crudlist->size());
//    lb->setText(str);
//    ui->hl_label_crud->addWidget(lb);

}
//-----------------------------------------------------------------------------------//
void MainWindow::on_pushButton_clicked()
{
//    if (ui->lineEdit->text() == QString(""))
//    {
//        RefreshTab();
//    }
//    else {
//   ///Проверяю условие: если в результате поиска, полученная книга = переданному айди
//        Crud *cr = Crud::id_zk_search(ui->lineEdit->text().toInt());
//        if (cr == 0)
//            QMessageBox::critical(this,QObject::tr("Внимание"),QObject::tr("По вашему запросу ничего не найдено!")); ///Ругаемся
//        else
//        {
//            crud_model->setOneCrud(cr);
//            ui->tableView->setModel(crud_model);
//            ui->tabWidget->setCurrentIndex(0);

//            ot_model->reset_OTModel();
//            contacts_model->reset_ContactModel();

//            while(ui->hl_label_contact->count())
//            {
//                QLayoutItem *item = ui->hl_label_contact->takeAt(0);
//                delete item->widget();
//            }

//            while(ui->hl_label_owt->count())
//            {
//                QLayoutItem *item = ui->hl_label_owt->takeAt(0);
//                delete item->widget();
//            }

//            MainWindow::add_cancel_button();
//            while(ui->hl_label_crud->count())
//            {
//                QLayoutItem *item = ui->hl_label_crud->takeAt(0);
//                delete item->widget();
//            }

//            QLabel *lb = new QLabel;
//            QString str = "Найдено записей: "+QString::number(crud_model->actcrudlist.size());
//            lb->setText(str);
//            ui->hl_label_crud->addWidget(lb);
//        }
//    }
}
//-----------------------------------------------------------------------------------//
void MainWindow::add_cancel_button()
{
    if (p_b_counter < 1)
       {
    pb_clear_search = new QPushButton;
    pb_clear_search->setText("Сброс параметров поиска");
    ui->vl_cancel_button->addWidget(pb_clear_search);

    setTabOrder( ui->pushButton, pb_clear_search);
    setTabOrder(pb_clear_search , ui->pb_refresh);

    p_b_counter++;

    connect(pb_clear_search, SIGNAL(clicked()), this ,SLOT(RefreshTab()));
    }
}
//-----------------------------------------------------------------------------------//
void MainWindow::open_upd_tab(Crud *cr)
{
//        if(updlist == 0)
//        {
//            updlist = new QList<Update*>;
//        }

//        for (int i=0; i<updlist->size(); i++)
//        {
//            if(updlist->at(i)->new_cr->zk_id == cr->zk_id)
//            {
//                ui->tabWidget->setCurrentIndex(i+1);
//                return;
//            }
//        }

//        Update *upd = new Update; //указатель на форму добавления
//        upd->frm_t = Update_form;
//        upd->imprt_t = Update_pg_data;

//        //connect(this,SIGNAL(Send_data(Crud*, int)), updlist->at(updlist->size()-1), SLOT(Recieve_data(Crud*, int)));
//        ///создание списка обусловлено работой класса листмастер
//        /// он работает только со списками
//        QList<Crud*> *crudlist = new QList<Crud*>;
//        List_master *list = new List_master(Main_window_for_Update);
//        list->set_counters();
//        //Загрузка ВСЕХ данных выбранной ЗК
//        list->fill_crud_list(crudlist,cr->zk_id, PSQLtype);
//        //emit Send_data(crudlist->at(0), updlist->size()-1);

//        if(crudlist->isEmpty())
//        {
//            QMessageBox::critical(exprt,QObject::tr("Ошибка"),QObject::tr("Возможно выбранной записи нет в базе. Попробуйте обновить таблицы")); ///Хвалимся
//            return;
//        }

//        updlist->append(upd);
//        upd->Recieve_data(crudlist->at(0));
//        upd->take_linked_zk(crudlist->at(0));
//        connect (updlist->at(updlist->size()-1), SIGNAL (open_update_tab(Crud *)), this, SLOT(open_upd_tab(Crud *)));
//        connect(updlist->at(updlist->size()-1), SIGNAL(Ready_for_update(int)), this, SLOT(ShowThisTab(int)));
//        connect(updlist->at(updlist->size()-1), SIGNAL(open_confluence_upd(Crud*,Crud*,Crud*)), this, SLOT(open_confluence_form(Crud*,Crud*,Crud*)));
//        ui->tabWidget->insertTab( ui->tabWidget->count()+1 ,updlist->at(updlist->size()-1),"Редактировать ЗК № "+QString::number(updlist->at(updlist->size()-1)->new_cr->zk_id));
//        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
//        upd->focus_on_widget();
}
//-----------------------------------------------------------------------------------//
void MainWindow::set_validators()
{
//    ui->lineEdit->setValidator(new QIntValidator(crud_model->crudlist->first()->zk_id,crud_model->crudlist->last()->zk_id));
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    ///обработка редактирования/добавления
    if ( ui->tabWidget->widget(index)->objectName() == "Update")
    {
        ui->tabWidget->widget(index)->deleteLater();
        Update *upd = dynamic_cast<Update*>(ui->tabWidget->widget(index)); //Приведение типа от виджета к классу
    if (upd->frm_t == Add_form)
    {//Работа с добавлением
        if(addlist != 0)
        {
            addlist->clear();
            delete addlist;
            addlist = 0;
            return;
        }
    } else if (upd->frm_t == Update_form)
        {
            if(updlist != 0)
            {
                int i = 0;
                int zk_id_local = upd->new_cr->zk_id;
                while (i < updlist->size())
                {
                    if(updlist->at(i)->new_cr->zk_id == zk_id_local)
                    {
                        delete  updlist->at(i);
                        //delete upd;
                        updlist->removeAt(i);
                        if (updlist->isEmpty())
                        {
                            updlist = 0;
                            return;
                        }
                    }  else
                        i++;
                }
            }
        }
    } else if ( ui->tabWidget->widget(index)->objectName() == "OfficialTelephones")
    {
        ui->tabWidget->widget(index)->deleteLater();
        delete of;
        of = 0;

    }
    ///NEW
    else if(ui->tabWidget->widget(index)->objectName() == "EditPerson"){

        if(Util::instance()->editPersonList() != 0){
            EditPerson *ep = dynamic_cast<EditPerson*>(ui->tabWidget->widget(index)); //Приведение типа от виджета к классу
            int i = 0;
            QString personIdInTab =  ep->person->id;
            while( i< Util::instance()->editPersonList()->size()){
                if(Util::instance()->editPersonList()->at(i)->person->id == personIdInTab){
                    delete  Util::instance()->editPersonList()->at(i);
                    Util::instance()->editPersonList()->removeAt(i);
                    if (Util::instance()->editPersonList()->isEmpty())
                    {
                        return;
                    }
                } else
                    i++;
            }
        }
    }
    else if(ui->tabWidget->widget(index)->objectName() == "editEvent"){
        if(Util::instance()->editEventList() != 0){
            editEvent *ee = dynamic_cast<editEvent*>(ui->tabWidget->widget(index)); //Приведение типа от виджета к классу
            int i = 0;
            QString eventIdInTab =  ee->localEvent->id;
            while( i< Util::instance()->editEventList()->size()){
                if(Util::instance()->editEventList()->at(i)->localEvent->id == eventIdInTab){
                    delete  Util::instance()->editEventList()->at(i);
                    Util::instance()->editEventList()->removeAt(i);
                    if (Util::instance()->editEventList()->isEmpty())
                    {
                        return;
                    }
                } else
                    i++;
            }
        }
    }
    this->RefreshTab();
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_tabWidget_2_tabCloseRequested(int index)
{
    ui->tabWidget_2->indexOf(an);
    if ( ui->tabWidget_2->widget(index)->objectName() == "Search")
    {
        ui->tabWidget_2->widget(index)->deleteLater();
        delete sr;
        sr = 0;
        return;
    } else
    if ( ui->tabWidget_2->widget(index)->objectName() == "Analysis")
    {
        ui->tabWidget_2->widget(index)->deleteLater();
        delete an;
        an = 0;
        return;
    } else
    if ( ui->tabWidget_2->widget(index)->objectName() == "master_export_Form")
    {
        ui->tabWidget_2->widget(index)->deleteLater();
        delete exprt;
        exprt = 0;
        return;
    } else
    if ( ui->tabWidget_2->widget(index)->objectName() == "Master_import_form")
    {
        ui->tabWidget_2->widget(index)->deleteLater();
        delete imprt;
        imprt = 0;
        return;
    }

}
//-----------------------------------------------------------------------------------//
void MainWindow::findIndexByNameTab2( QString WidgetName )
{
    int countOfTabs = ui->tabWidget_2->count();
    for(int a=0; a<countOfTabs; a++)
    {
        if(ui->tabWidget_2->widget(a)->objectName() == WidgetName)
        {
            on_tabWidget_2_tabCloseRequested(a);
            break;
        }
    }
}
//-----------------------------------------------------------------------------------//
void MainWindow::closeOF(QString WidgetName)
{
    int countOfTabs = ui->tabWidget->count();
    for(int a=0; a<countOfTabs; a++)
    {
        if(ui->tabWidget->widget(a)->objectName() == WidgetName)
        {
            on_tabWidget_tabCloseRequested(a);
            break;
        }
    }
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_action_official_tel_triggered()
{
    if(of == 0)
        {
        of = new OfficialTelephones;
        ui->tabWidget->insertTab( ui->tabWidget->count()+1 ,of,"Cлужебные телефоны");
        ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
        connect(this,SIGNAL(Fill_table_of()), of, SLOT(Fill_table()));
        connect(of, SIGNAL(closeTab(QString)), this, SLOT(closeOF(QString)));
        emit Fill_table_of();
        of->set_tab_orders();
        }
    else
    {
        ui->tabWidget->setCurrentIndex( ui->tabWidget->indexOf(of));
        of->set_tab_orders();
    }
    of->focus_on_widget();
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_actionexport_triggered()
{
    if(exprt == 0)
    {
        exprt = new Master_export_Form;
        exprt->add_file_path(file_path);
        ui->tabWidget_2->insertTab( ui->tabWidget_2->count()+1 ,exprt,"Экспорт данных");
        ui->tabWidget_2->setCurrentIndex(ui->tabWidget_2->count()-1);
        connect(exprt,SIGNAL(rb_zk_clicked()),this, SLOT(on_action_search_triggered()));
        connect(exprt, SIGNAL(prepareExport(SimpleCrypt, ExportType, QString)), this, SLOT(prepare_export(SimpleCrypt, ExportType, QString)));
//        connect(exprt, SIGNAL(TESTING_export(QString,QString, bool, bool, bool)),this,SLOT(testing_export(QString, QString, bool, bool, bool)));
        connect(exprt, SIGNAL(closeThis(QString)), this, SLOT(findIndexByNameTab2(QString)));
    }
    else
        ui->tabWidget_2->setCurrentIndex( ui->tabWidget_2->indexOf(exprt));

    set_normal_width(exprt->actual_size.width());
    exprt->focus_on_widget();

}
//-----------------------------------------------------------------------------------//
void MainWindow::testing_export(QString filename, QString password, bool cb_off_tels, bool cb_set_password, bool cb_zk)
{
//    if( for_exprt == 0)
//    {
//        for_exprt = new For_export();

//        if (filename.isEmpty())
//        {
//            QMessageBox::critical(exprt,QObject::tr("Ошибка"),QObject::tr("Вы не указали путь!")); ///Хвалимся
//            return;
//        }

//        QList<Crud*> *crud = new  QList<Crud*> ;
//        QList<Off_tels*> *offtel = new QList<Off_tels*>;
//        zk_links *links_for_export = new zk_links;
//        QList<int> *exported_id = new QList<int>;
//        for_exprt->list->set_counters();

//        if(cb_zk)
//            for (int i=0;i<crud_model->crudlist->size();i++) // пробегаюсь по отображаемому списку
//            {
//                if (crud_model->crudlist->at(i)->checkState_ == Checked)
//                {
//                    //form_exprt->exported_zk_id.append( crud_model->crudlist->at(i)->zk_id);
//                    //form_exprt->list->fill_crud_list(crud, crud_model->crudlist->at(i)->zk_id, PSQLtype);
//                    exported_id->append(crud_model->crudlist->at(i)->zk_id);
//                }
//            }
//        else if (cb_off_tels)
//            for_exprt->list->fill_off_tels(offtel,PSQLtype);

//        if(!exported_id->isEmpty())
//        {
//            for (int i=0; i<exported_id->size();i++)
//                for_exprt->list->fill_crud_list(crud, exported_id->at(i), PSQLtype);

//            links_for_export->take_links(exported_id);
//            delete exported_id;
//        }


//        if(!crud->isEmpty() || !offtel->isEmpty())
//        {
//            if( for_exprt->Do_export(filename, crud, password, cb_off_tels, cb_set_password, offtel, links_for_export))
//            {
//                if(!crud->isEmpty())
//                    QMessageBox::information(exprt,QObject::tr("Успех"),QObject::tr("Отчет по результатам экспорта и данные сохранены в файл, расположенный по пути : %1 \r\n "
//                                                                                    " ЗК экспортировано: %2 .").arg(filename).arg(QString::number(crud->size()))); ///Хвалимся
//                else
//                    if(!offtel->isEmpty())
//                    QMessageBox::information(exprt,QObject::tr("Успех"),QObject::tr("Отчет по результатам экспорта и данные сохранены в файл, расположенный по пути : %1 \r\n "
//                                                                                    " Служебных телефонов экспортировано: %2 .").arg(filename).arg(QString::number(offtel->size()))); ///Хвалимся

//            }
//            else
//                QMessageBox::critical(exprt,QObject::tr("Ошибка"),QObject::tr("Во время экспорта данных что-то пошло не так!")); ///Хвалимся
//        }
//        else {
//            QMessageBox::warning(exprt,QObject::tr("Внимание"),QObject::tr("Экспорт не был выполнен, так как вы не выбрали данные!")); ///Хвалимся
//        }
//        for_exprt =0;
//        delete offtel;
//        delete crud;
//    }
}
//-----------------------------------------------------------------------------------//
void MainWindow::prepare_export(SimpleCrypt crypt, ExportType type, QString filePath){
    if (filePath.isEmpty())
    {
        QMessageBox::critical(exprt,QObject::tr("Ошибка"),QObject::tr("Вы не указали путь!"));
        return;
    }

    QString messageStart;
    QString exportedFilePath;

    if(type == events || type == eventsPassword){
        QList<Event*> *checkedEvents = this->getSelectedEvents();

        if(checkedEvents->size() == 0){
           QMessageBox::warning(exprt,QObject::tr("Внимание"),QObject::tr("Экспорт не был выполнен, так как вы не выбрали данные!"));
           return;
        }

        exportedFilePath = this->for_exprt.exportEvents(checkedEvents, crypt, type, filePath);
        delete checkedEvents;
        switch (type){
        case events:
            messageStart = "Дела экспортированы успешно без пароля. Данные сохранены в файл: ";
            break;
        case eventsPassword:
            messageStart = "Дела экспортированы и зашифрованы паролем успешно. Данные сохранены в файл: ";
            break;
        }
    }
    else if( type == official || type == officialPassword){
        QList<Off_tels*> *offList = new QList<Off_tels*>();

        if(Off_tels::selectOffTel(offList)){
            exportedFilePath = this->for_exprt.exportOfficial(offList, crypt, type, filePath);
            delete offList;

            switch (type){
            case official:
                messageStart = "Служебные телефоны экспортированы успешно без пароля. Данные сохранены в файл: ";
                break;
            case officialPassword:
                messageStart = "Служебные телефоны экспортированы и зашифрованы паролем успешно. Данные сохранены в файл: ";
                break;
            }
        }
        else{
            QMessageBox::warning(exprt,QObject::tr("Внимание"),QObject::tr("Экспорт не был выполнен, так как во время выгрузки данных из базы возникли неполадки!"));
            return;
        }
    }
    QMessageBox::information(exprt,QObject::tr("Успех"), messageStart + exportedFilePath);

}

void MainWindow::testing_opening(QString filename, QString password, bool folder, bool oldData)
{
    auto importResults= this->for_import.openFile(filename, password, folder, oldData);

    if(importResults.state == success)
        QMessageBox::information(this,QObject::tr("Успех"), importResults.message);
    else
        QMessageBox::warning(this,QObject::tr("Ошибка"),  importResults.message);
    this->RefreshTab();
}

//-----------------------------------------------------------------------------------//
//void MainWindow::testing_opening(QString filename, QString password, bool folder, bool of_t)
//{
//    ///Класс для импорта
//    Text_handler::instance()->clear_text();

//    if(folder)
//    {
//        QDir direcotry(filename);
//        QStringList filelist = direcotry.entryList(QStringList("*"), QDir::Files);
//        Text_handler::instance()->set_mode(zk_report);
//        foreach (QString file, filelist )
//        {
//            QString filename2 = filename;
//            filename2 += "/" + file;
//            Import_Form *import_form = new Import_Form; //необходим доступ для
//            import_form->form_state = zk;
//            connect(import_form,SIGNAL(Refresh_tab()),this,SLOT(RefreshTab()));

//        Text_handler::instance()->set_zk_folder_line();
//            if (import_form->Testing_open_db( filename2,password, of_t)) //Если есть совпадение, то
//            {
//                ///Идем сравнивать выгруженный в список дамп с БД
//                /// Метод алгоритма сравнения и импорта
//                if(import_form->begin_import())
//                   {
//                    //ДИАЛОГ ДЛЯ ИМИТАЦИИ EXEC()

//                    QDialog *d = new QDialog(this);
//                    d->setWindowTitle("Мастер импорта");
//                    QVBoxLayout *l = new QVBoxLayout(d);
//                    QDialogButtonBox *dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
//                    dbb->button(QDialogButtonBox::Ok)->setVisible(false);
//                    dbb->button(QDialogButtonBox::Cancel)->setVisible(false);
//                    l->addWidget(import_form);
//                    l->addWidget(dbb);
//                    d->showMaximized();
//                    QSize sz = d->size();
//                    d->setFixedSize(sz);
//                    //передаю уровнем ниже указатель на кнопки
//                    //для имитации accept и rejected
//                    import_form->ddb = dbb;


//                    connect(dbb, SIGNAL(accepted()), d, SLOT(accept()));
//                    connect(dbb, SIGNAL(rejected()), d, SLOT(reject()));
//                    switch (d->exec())
//                    {
//                    case QDialog::Rejected:
//                        QMessageBox::critical(this,"Внимание","Импорт из папки был прерван");
//                        delete l;
//                        d->close();
//                        delete d;
//                        //delete import_form;
//                        return;
//                   case QDialog::Accepted:
//                        //QMessageBox::information(this,"Успех","Импорт прошел успешно");
//                        delete l;
//                        d->close();
//                        delete d;
//                        //delete import_form;
//                        continue;
//                    }
//               }
//            }
//        }
//        An_result::import_report(Text_handler::instance()->get_text());
//    RefreshTab();
//    }
//    else
//    {
//        Import_Form *import_form = new Import_Form; //необходим доступ для

//        if (import_form->Testing_open_db( filename,password, of_t)) //Если есть совпадение, то
//        {
//            ///Идем сравнивать выгруженный в список дамп с БД
//            /// Метод алгоритма сравнения и импорта
//           if(!of_t)
//           {
//               import_form->form_state = zk;
//               Text_handler::instance()->set_mode(zk_report);

//               if(import_form->begin_import())
//               {
//                   //ДИАЛОГ ДЛЯ ИМИТАЦИИ EXEC()

//                   QDialog *d = new QDialog(this);
//                   d->setWindowTitle("Мастер импорта");
//                   QVBoxLayout *l = new QVBoxLayout(d);
//                   QDialogButtonBox *dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
//                   dbb->button(QDialogButtonBox::Ok)->setVisible(false);
//                   dbb->button(QDialogButtonBox::Cancel)->setVisible(false);
//                   l->addWidget(import_form);
//                   l->addWidget(dbb);
//                   d->showMaximized();
//                   QSize sz = d->size();
//                   d->setFixedSize(sz);
//                   //передаю уровнем ниже указатель на кнопки
//                   //для имитации accept и rejected
//                   import_form->ddb = dbb;


//                   connect(dbb, SIGNAL(accepted()), d, SLOT(accept()));
//                   connect(dbb, SIGNAL(rejected()), d, SLOT(reject()));
//                   switch (d->exec())
//                   {
//                   case QDialog::Rejected:
//                       delete l;
//                       d->close();
//                       delete d;
//                       return;
//                   case QDialog::Accepted:
//                       delete l;
//                       d->close();
//                       delete d;
//                   }
//               }

//               An_result::import_report(Text_handler::instance()->get_text());
//               RefreshTab();

//           }
//           else {
//               import_form->form_state = official_tel;
//               Text_handler::instance()->set_mode(off_report);
//               if(import_form->begin_import_of_t())
//               {
//                   //ДИАЛОГ ДЛЯ ИМИТАЦИИ EXEC()

//                   QDialog *d = new QDialog(this);
//                   d->setWindowTitle("Мастер импорта");
//                   QVBoxLayout *l = new QVBoxLayout(d);
//                   QDialogButtonBox *dbb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
//                   dbb->button(QDialogButtonBox::Ok)->setVisible(false);
//                   dbb->button(QDialogButtonBox::Cancel)->setVisible(false);
//                   l->addWidget(import_form);
//                   l->addWidget(dbb);
//                   d->showMaximized();
//                   QSize sz = d->size();
//                   d->setFixedSize(sz);
//                   //передаю уровнем ниже указатель на кнопки
//                   //для имитации accept и rejected
//                   import_form->ddb = dbb;


//                   connect(dbb, SIGNAL(accepted()), d, SLOT(accept()));
//                   connect(dbb, SIGNAL(rejected()), d, SLOT(reject()));
//                   switch (d->exec())
//                   {
//                   case QDialog::Rejected:
//                       delete l;
//                       RefreshTab();
//                       d->close();
//                       delete d;
//                       return;
//                   case QDialog::Accepted:
//                       RefreshTab();
//                       delete l;
//                       d->close();
//                       delete d;
//                   }
//               }
//               An_result::import_report(Text_handler::instance()->get_text());
//               RefreshTab();
//           }
//        }
//    }
//}


//-----------------------------------------------------------------------------------//
void MainWindow::on_action_import_triggered()
{
    if(imprt == 0)
    {
        imprt = new Master_import_form(this);
        ui->tabWidget_2->insertTab( ui->tabWidget_2->count()+1 ,imprt,"Импорт данных");
        ui->tabWidget_2->setCurrentIndex(ui->tabWidget_2->count()-1);
        connect(imprt, SIGNAL(TESTING_open(QString,QString, bool, bool)), this, SLOT(testing_opening(QString, QString, bool, bool)));
        connect(imprt, SIGNAL(closeThis(QString)), this, SLOT(findIndexByNameTab2(QString)));

    }
    else
        ui->tabWidget_2->setCurrentIndex( ui->tabWidget_2->indexOf(imprt));

    imprt->set_tab_orders();
    imprt->focus_on_widget();

    set_normal_width(imprt->actual_size.width());
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_action_Settings_triggered()
{
        Settings_Form *settings = new Settings_Form;
        settings->show();
        connect (settings, SIGNAL(Update_main()), this, SLOT(RefreshTab()));
}
//-----------------------------------------------------------------------------------//
void MainWindow::next_page()
{
    if(ui->button_layout_forward->count() >0)
    {
        this->eventModel->next_page();
        ui->eventTable->setModel(this->eventModel);
        Add_pagination_buttons();

        this->clearLabel();
        this->createLabel();
    }
}
//-----------------------------------------------------------------------------------//
void MainWindow::previous_page()
{
   if(ui->button_layout_back->count() >0)
   {
       this->eventModel->previous_page();
       ui->eventTable->setModel(this->eventModel);
       Add_pagination_buttons();

       this->clearLabel();
       this->createLabel();

//       QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows;
//       QModelIndex tempIndex = ui->tableView->model()->index(0, 0);
//       ui->tableView->selectionModel()->select(tempIndex, flags);
//       ui->tableView->setFocus();
   }

}
//-----------------------------------------------------------------------------------//
void MainWindow::set_fonts()
{

}
//-----------------------------------------------------------------------------------//
void MainWindow::add_splitter_lines()
{
//    QSplitterHandle *handle = ui->splitter_3->handle(1);
//    QVBoxLayout *layout = new QVBoxLayout(handle);
//    layout->setSpacing(0);
//    layout->setMargin(0);

//    QFrame *line = new QFrame(handle);
//    line->setFrameShape(QFrame::VLine);
//    line->setFrameShadow(QFrame::Sunken);
//    layout->addWidget(line);

//    QSplitterHandle *handle_2 = ui->splitter_2->handle(1);
//    QVBoxLayout *layout_2 = new QVBoxLayout(handle_2);
//    layout_2->setSpacing(0);
//    layout_2->setMargin(0);

//    QFrame *line_2 = new QFrame(handle_2);
//    line_2->setFrameShape(QFrame::HLine);
//    line_2->setFrameShadow(QFrame::Sunken);
//    layout_2->addWidget(line_2);
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_tabWidget_tabBarClicked(int index)
{
}

//-----------------------------------------------------------------------------------//
void MainWindow::on_action_add_1_triggered()
{
    on_action_add_triggered();
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_action_2_upd_triggered()
{
    on_action_update_triggered();
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_action_3_del_triggered()
{
    on_action_delete_triggered();
}
//-----------------------------------------------------------------------------------//
void MainWindow::on_action_5_show_triggered()
{
       ui->tabWidget->setCurrentIndex(0);
}
//-----------------------------------------------------------------------------------//
void MainWindow::open_confluence_form(Crud *cnfl_cr, Crud *main_crud, Crud *added_cr)
{
        Update *upd = new Update;
        upd->frm_t = Confluence_form;
        upd->imprt_t = Update_pg_data;

        connect(this,SIGNAL(Send_data(Crud*)), upd, SLOT(recieve_import_data(Crud*)));
        connect(upd,SIGNAL(add_import_crud(Crud*)), this, SLOT(recieve_added_import_crud(Crud*)));
        connect(upd, SIGNAL(Ready_for_update(int)), this, SLOT(ShowThisTab(int)));
        upd->start_confluence(cnfl_cr, main_crud,added_cr);
        upd->show();
}
//-----------------------------------------------------------------------------------//
void MainWindow::set_normal_width(int size)
{
    QList<int> size_list;
    size_list.append(size);
    size_list.append(this->size().width() - size);
    ui->splitter->setSizes(size_list);
}

void MainWindow::header_clicked(int id)
{
    if(id != 0)
        return;
    else
    {
        switch (m_c_s)
        {
        case All_unchecked:
            this->eventModel->changeCheckedStateEventList(true);
            ui->eventTable->setModel(this->eventModel);
            m_c_s = All_checked;
            Add_pagination_buttons();
        return;
        case All_checked:
            this->eventModel->changeCheckedStateEventList(false);
            ui->eventTable->setModel(this->eventModel);
            m_c_s = All_unchecked;
            Add_pagination_buttons();
        return;
        }
    }
}

void MainWindow::on_action_4_analysis_triggered()
{
    on_action_analysis_triggered();
}

void MainWindow::on_action_6_triggered()
{
    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget_2->setCurrentIndex(0);
    ui->splitter->setSizes(size_list);

}

void MainWindow::on_action_8_triggered()
{
    on_action_search_triggered();
}

void MainWindow::on_action_9_triggered()
{
    on_action_official_tel_triggered();
}

void MainWindow::on_action_10_triggered()
{
    on_actionexport_triggered();
}

void MainWindow::on_action_12_triggered()
{
    on_action_import_triggered();
}

void MainWindow::on_action_13_triggered()
{
    on_action_Settings_triggered();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Enter:
            if(mainwindowFocus == FocusOnRight && this->eventTableIndex.isValid())
                on_action_update_triggered();
            else if(mainwindowFocus == FocusOnLeft)
                on_pushButton_clicked();
            return;

         case Qt::Key_PageUp:
            if(p_b_forward != 0)
                p_b_forward->click();
            return;

        case Qt::Key_PageDown:
           if(p_b_back != 0)
               p_b_back->click();
           return;

        case Qt::Key_F1:
            openPopUp();
        return;

        case Qt::Key_Escape:
            RefreshTab();
        return;
    }

}

void MainWindow::find_linked_zk()
{
//    RefreshTab();
//            while (zk_id > crud_model->actcrudlist.at(crud_model->actcrudlist.size()-1)->zk_id)
//            {
//                crud_model->next_page_crud();
//                ui->tableView->setModel(crud_model);
//                Add_pagination_buttons();
//            }

//            while (zk_id < crud_model->actcrudlist.at(0)->zk_id)
//            {
//                crud_model->previous_page_crud();
//                ui->tableView->setModel(crud_model);
//                Add_pagination_buttons();
//            }

//            for (int i = 0; i < crud_model->actcrudlist.size(); i++)
//            {
//                if(crud_model->actcrudlist.at(i)->zk_id == zk_id)
//                {
//                    zk_id = i;
//                    break;
//                }
//            }

//            index_tab1 = crud_model->index(zk_id,0);
//            ui->tableView->setCurrentIndex(index_tab1);
//            on_tableView_clicked(index_tab1, cont_num);
//            //Delete
//            if(ui->vl_for_search_contact->count())
//            {
//                QLayoutItem *item = ui->vl_for_search_contact->takeAt(0);
//                delete item->widget();
//            }
//       QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows;
//       QModelIndex tempIndex = ui->tableView->model()->index(0, 0);
//       ui->tableView->selectionModel()->select(tempIndex, flags);
//       ui->tableView->setFocus();
}

void MainWindow::on_pb_refresh_clicked()
{
    RefreshTab();
}

//void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
//{
////    index_tab1 = index;
////    on_action_update_triggered();
//}

void MainWindow::openPopUp(){
    PopUp::instance()->setPopupText("<h2 align=\"middle\">Навигация в главном окне</h2>"
                                    "<p><b>\"CTRL\"+\"Q\" и \"CTRL\"+\"W\"</b> для переключения вкладок правого окна</p>"
                                    "<p><b>\"CTRL\"+\"S\" и \"CTRL\"+\"D\"</b> для преключения вкладок левого окна</p>"
                                    "<p><b>\"ESC\"</b> для сброса результатов поиска</p>"
                                    "<h2 align=\"middle\">Навигация в таблицах</h2>"
                                    "<p>&rarr; для раскрытия телефонных номеров и контактов фокуса</p>"
                                    "<p>&uarr; и &darr; для перемещения фокуса вверх и вниз в пределах таблицы</p>"
                                    "<p>&larr; для возвращения фокуса назад</p>"
                                    "<p><b>\"Z\"</b> для перемещения фокуса налево по горизонтали</p>"
                                    "<p><b>\"X\"</b> для перемещения фокуса направо по горизонтали</p>", leftMenu);
}

void MainWindow::set_shortcuts()
{
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), this, SLOT(next_tab()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(prev_tab()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this, SLOT(next_tab_tab2()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(prev_tab_tab2()));

    new QShortcut(QKeySequence(Qt::Key_F2), this, SLOT(on_action_update_triggered()));
    new QShortcut(QKeySequence(Qt::Key_F3), this, SLOT(on_action_add_triggered()));
    new QShortcut(QKeySequence(Qt::Key_F4), this, SLOT(on_action_6_triggered()));
    //new QShortcut(QKeySequence(Qt::Key_F5), this, SLOT());
    ///???????????????????????????????????
    new QShortcut(QKeySequence(Qt::Key_F6), this, SLOT(on_action_analysis_triggered()));
    new QShortcut(QKeySequence(Qt::Key_F7), this, SLOT(on_action_search_triggered()));
    new QShortcut(QKeySequence(Qt::Key_F8), this, SLOT(on_action_delete_triggered()));
    //new QShortcut(QKeySequence(Qt::Key_F9), this, SLOT(()));
    new QShortcut(QKeySequence(Qt::Key_F10), this, SLOT(on_actionexport_triggered()));
    new QShortcut(QKeySequence(Qt::Key_F11), this, SLOT(on_action_import_triggered()));
    new QShortcut(QKeySequence(Qt::Key_F12), this, SLOT(on_action_Settings_triggered()));
}

void MainWindow::   next_tab()
{
    ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex() + 1);
    QWidget *widget = ui->tabWidget->widget(ui->tabWidget->currentIndex());
    QString widgetName = widget->objectName();
    setFocusOnTab(widgetName, widget);
}

void MainWindow::prev_tab()
{
    ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex() - 1);
    QWidget *widget = ui->tabWidget->widget(ui->tabWidget->currentIndex());
    QString widgetName = widget->objectName();
    setFocusOnTab(widgetName, widget);
}

void MainWindow::next_tab_tab2()
{
    ui->tabWidget_2->setCurrentIndex(ui->tabWidget_2->currentIndex() + 1);
    QWidget *widget = ui->tabWidget_2->widget(ui->tabWidget_2->currentIndex());
    QString widgetName = widget->objectName();
    setFocusOnTab(widgetName, widget);
}

void MainWindow::prev_tab_tab2()
{
    ui->tabWidget_2->setCurrentIndex(ui->tabWidget_2->currentIndex() - 1);
    QWidget *widget = ui->tabWidget_2->widget(ui->tabWidget_2->currentIndex());
    QString widgetName = widget->objectName();
    setFocusOnTab(widgetName, widget);
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
}

void MainWindow::setFocusOnTab(QString widgetName, QWidget *widgetOnTab)
{
    ///Ветвления от имени виджета
    if(widgetName == "OfficialTelephones")
    {
        OfficialTelephones *of = dynamic_cast<OfficialTelephones*>(widgetOnTab);
        of->focus_on_widget();
        return;
    }
    else if(widgetName =="Update")
    {
        Update *upd = dynamic_cast<Update*>(widgetOnTab);
        upd->focus_on_widget();
        return;
    }
    else if(widgetName =="tab")
    {
        mainwindowFocus = FocusOnRight;
        QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows;
//        QModelIndex tempIndex =
//                ui->tableView->model()->index( ui->tableView->currentIndex().row(),
//                                               ui->tableView->currentIndex().column());
//        ui->tableView->selectionModel()->select(tempIndex, flags);
//        ui->tableView->setFocus();
        return;
    }
    else if(widgetName =="Analysis")
    {
        class Analysis *an = dynamic_cast<class Analysis*>(widgetOnTab);
        an->focus_on_widget();
        return;
    }
    else if(widgetName =="tab_2"){
        mainwindowFocus = FocusOnLeft;
        ui->lineEdit->setFocus();
        return;
    }
    else if(widgetName =="Search"){
        Search *sr = dynamic_cast<Search*>(widgetOnTab);
        sr->set_tab_orders();
        return;
    }
    else if(widgetName =="master_export_Form"){

       Master_export_Form *exp = dynamic_cast<Master_export_Form*>(widgetOnTab);
       exp->focus_on_widget();
       return;
   }
    else if(widgetName =="Master_import_form"){
        Master_import_form *imp = dynamic_cast<Master_import_form*>(widgetOnTab);
        imp->focus_on_widget();
        return;
    }
    else if(widgetName =="EditPerson"){
        EditPerson *ep = dynamic_cast<EditPerson*>(widgetOnTab);
        ep->setFocus();
        return;
    }
}

void MainWindow::set_label()
{
//    QLabel *lb = new QLabel("<p><b>(CTRL+Q) и (CTRL+W)</b> для переключения вкладок правого окна</p>"
//                            "<p><b>(CTRL+S) и (CTRL+D)</b> для преключения вкладок левого окна</p>");
    //    ui->vl_shortcut_info->addWidget(lb);
}

void MainWindow::set_tab_orders()
{
    ui->pb_hided->setFocusProxy(ui->lineEdit);

    setTabOrder(ui->lineEdit, ui->pushButton);
    setTabOrder( ui->pushButton, ui->pb_refresh);
    setTabOrder( ui->pb_refresh, ui->pb_hided);

}

void MainWindow::focusOnOT()
{
//    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows;
//    QModelIndex tempIndex = ui->tableView_2->model()->index(0, 0);
//    ui->tableView_2->selectionModel()->select(tempIndex, flags);
//    ui->tableView_2->setFocus();
}
void MainWindow::focusOnZK()
{
//    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows;
//    ui->tableView->selectionModel()->select(this->index_tab1, flags);
//    ui->tableView->setFocus();
}


void MainWindow::setTableConnections()
{
//    connect (ui->tableView, SIGNAL(openUpdateForm(QModelIndex)), this,
//             SLOT(on_tableView_doubleClicked(const QModelIndex)));
//    connect (ui->tableView, SIGNAL(nextPage()), this, SLOT(next_page()));
//    connect (ui->tableView, SIGNAL(previousPage()), this, SLOT(previous_page()));
//    connect (ui->tableView, SIGNAL(getOt(QModelIndex)), this, SLOT(getOt(QModelIndex)));

//    connect(ui->tableView_2, SIGNAL(backOnZk()), this, SLOT(focusOnZK()));
//    connect(ui->tableView_2, SIGNAL(getCont(QModelIndex)), this, SLOT(getCont(QModelIndex)));

//    connect(ui->tableView_3, SIGNAL(backOnOt()), this, SLOT(focusOnOT()));
//    connect(ui->tableView_3, SIGNAL(getVar2Zk()), this, SLOT(find_linked_zk()));
//    connect(ui->tableView_3, SIGNAL(clickOnContact(QModelIndex)), this, SLOT(on_tableView_3_clicked(QModelIndex)));

//    connect(ui->tableView, SIGNAL( f1Pressed() ), this, SLOT( openPopUp()));
//    connect(ui->tableView_2, SIGNAL( f1Pressed() ), this, SLOT( openPopUp()));
//    connect(ui->tableView_3, SIGNAL( f1Pressed() ), this, SLOT( openPopUp()));
}

void MainWindow::getOt(QModelIndex index)
{
//    on_tableView_clicked(index, 0);
//    QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows;
//    QModelIndex tempIndex = ui->tableView_2->model()->index(0, 0);
//    ui->tableView_2->selectionModel()->select(tempIndex, flags);
//    ui->tableView_2->setFocus();
}

void MainWindow::getCont(QModelIndex index)
{
//    on_tableView_2_clicked(index);
//    ui->tableView_3->setFocus();
//     if(ui->tableView_3->currentIndex().row() != -1)
//     {
//         QItemSelectionModel::SelectionFlags flags = QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows;
//         QModelIndex tempIndex = ui->tableView_3->model()->index(0, 0);
//         ui->tableView_3->selectionModel()->select(tempIndex, flags);
//         ui->tableView_3->setFocus();
//         on_tableView_3_clicked(ui->tableView_3->currentIndex());
//     }
//     else
//         ui->tableView_2->setFocus();
}

void MainWindow::on_eventTable_clicked(const QModelIndex &index)
{
    this->eventTableIndex = index;
    Event *localEvent = this->eventModel->actEventList.at(index.row());
    Person::selectByEventId(localEvent->persons(), localEvent->id);
    Util::instance()->clearLayout(ui->cardsLayout);
    for ( int i =0; i < localEvent->persons()->size(); i++){
        PersonCard *card = new PersonCard();
        card->setPerson(localEvent->persons()->at(i));
        ui->cardsLayout->addWidget(card);
        connect(card, SIGNAL(openEditWindow(Person*)), this, SLOT(openEditPersonWindow(Person*)));

    }
}

void MainWindow::openEditPersonWindow(Person *p)
{

    for (int i=0; i < Util::instance()->editPersonList()->size(); i++)
    {
        if(Util::instance()->editPersonList()->at(i)->person->id == p->id)
        {
            ui->tabWidget->setCurrentIndex(i+1);
            return;
        }
    }

    EditPerson *ep = new EditPerson;
    ep->setType(updatePerson);
    connect(ep, SIGNAL(closeThis(EditPerson*)), this, SLOT(closePersonEdit(EditPerson*)));

    ep->setPerson(p);
    Util::instance()->editPersonList()->append(ep);

    ui->tabWidget->insertTab( ui->tabWidget->count()+1 ,
                              Util::instance()->editPersonList()->at(Util::instance()->editPersonList()->size()-1),
                              "Редактирование фигуранта "+p->name + " "+ p->lastname + " " + p->midname);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
}

void MainWindow::openAddPersonWindow(Person *p, editEvent *ee)
{
    EditPerson *ep = new EditPerson;
    ep->setType(addPerson);
    ep->setParent(ee);
    connect(ep, SIGNAL(closeThis(EditPerson*)), this, SLOT(closePersonEdit(EditPerson*)));
    connect(ep, SIGNAL(personIsAdded(EditPerson*)), this, SLOT(personIsAdded(EditPerson*)));
    ep->setPerson(p);
    Util::instance()->editPersonList()->append(ep);
    Util::instance()->linkAddEventPerson(ee,ep);

    ui->tabWidget->insertTab( ui->tabWidget->count()+1 ,
                              Util::instance()->editPersonList()->at(Util::instance()->editPersonList()->size()-1),
                              "Добавление нового фигуранта для дела "+ee->localEvent->id);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
}

void MainWindow::personIsAdded(EditPerson *ep)
{
    editEvent *ee = Util::instance()->getManagerParent(ep);
    ee->addNewPerson(ep->person);
    ee->updateCardsLayout();
    int a = ui->tabWidget->indexOf(ee);
    ui->tabWidget->setCurrentIndex(a);
    QWidget *widget = ui->tabWidget->widget(ui->tabWidget->currentIndex());
    QString widgetName = widget->objectName();
    this->setFocusOnTab(widgetName, widget);
}

void MainWindow::closePersonEdit(EditPerson *ep)
{
    if(Util::instance()->editPersonList() != 0){
        int i = 0;
        QString personIdInTab =  ep->person->id;
        while( i< Util::instance()->editPersonList()->size()){
            if(Util::instance()->editPersonList()->at(i)->person->id == personIdInTab){
                delete  Util::instance()->editPersonList()->at(i);
                Util::instance()->editPersonList()->removeAt(i);               
                return;
            } else
                i++;
        }
    }
}

void MainWindow::on_eventTable_doubleClicked(const QModelIndex &index)
{
    this->eventTableIndex = index;
    this->on_action_update_triggered();
}

void MainWindow::closeEditEvent(editEvent *ee)
{
    if(Util::instance()->editEventList() != 0){
        Util::instance()->unlinkAddEventPerson(ee);
        int i = 0;
        QString eventIdInTab =  ee->localEvent->id;
        while( i< Util::instance()->editEventList()->size()){
            if(Util::instance()->editEventList()->at(i)->localEvent->id == eventIdInTab){
                delete  Util::instance()->editEventList()->at(i);
                Util::instance()->editEventList()->removeAt(i);
                ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
                QWidget *widget = ui->tabWidget->widget(ui->tabWidget->currentIndex());
                QString widgetName = widget->objectName();
                this->setFocusOnTab(widgetName, widget);
                this->RefreshTab();
                return;
            } else
                i++;
        }
    }
}

void MainWindow::searchedResults(QList<QString> searchedIds)
{
    QLabel *lb = new QLabel;
    QString str = "Результаты поиска: "+QString::number(searchedIds.size())+" записей";
    lb->setText(str);
    QPushButton *pb = new QPushButton;
    pb->setText("Сбросить");
    connect( pb, SIGNAL(clicked()),this, SLOT(cancelSearchResults()));

    ui->searched_data_layout->addWidget(lb);
    ui->searched_data_layout->addWidget(pb);

    QList<Event*> *list = new QList<Event*>;
    if(Event::selectSearchedIds(list, searchedIds)){
        eventModel->setEventList(list);
        ui->eventTable->setModel(eventModel);
    }
}

void MainWindow::cancelSearchResults()
{
    auto size = ui->searched_data_layout->count();
    for (int a = 0; a< size; a++) {
        QLayoutItem *item = ui->searched_data_layout->takeAt(0);
        delete item->widget();
    }
    this->RefreshTab();
}

QList<Event *> *MainWindow::getSelectedEvents()
{
    QList<Event *> *events = new QList<Event *>();

    for (int a = 0; a<this->eventModel->eventList->size(); a++) {
        Event * ev = this->eventModel->eventList->at(a);
        if( ev->checkState_ == Unchecked_) continue;
        else{
            Person::selectByEventId(ev->persons(), ev->id);
            for (int b = 0; b<ev->persons()->size();b++) {
                Person *per = ev->persons()->at(b);
                Telephone::selectTelephone(per->telephones(), per->id);
                for (int c=0; c<per->telephones()->size(); c++) {
                    Telephone *tel =    per->telephones()->at(c);
                    Contact::selectContacts(tel->cont(), tel->id);
                }
            }
        }
        events->append(ev);
    }

    return events;
}

void MainWindow::clearLabel()
{
    while(ui->label_layout->count())
    {
        QLayoutItem *item = ui->label_layout->takeAt(0);
        delete item->widget();
    }
}

void MainWindow::createLabel()
{
    QLabel *lb = new QLabel;
    QString str = "Всего записей: "+QString::number(this->eventModel->eventList->size()) +" ("+this->eventModel->actEventList.at(0)->id+"..."+
            this->eventModel->actEventList.at(this->eventModel->actEventList.size()-1)->id+")";
    lb->setText(str);
    ui->label_layout->addWidget(lb);
}
