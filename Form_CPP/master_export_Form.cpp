#include "master_export_Form.h"
#include "ui_master_export_form.h"
#include "popup.h"

#include <QLineEdit>

Master_export_Form::Master_export_Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::master_export_Form)
{
    ui->setupUi(this);
    ui->le_password->setEchoMode(QLineEdit::Password);
    on_cb_set_password_clicked();
        actual_size = this->size();
//        ui->le_password->setVisible(false);
//        ui->label->setVisible(false);
//        ui->cb_set_password->setVisible(false);
}

Master_export_Form::~Master_export_Form()
{
    delete ui;
}

void Master_export_Form::on_cb_zk_clicked()
{
    ui->rb_check->setVisible(true);
}
void Master_export_Form::on_cb_off_tel_clicked()
{
    ui->rb_check->setVisible(false);
}

void Master_export_Form::on_rb_check_clicked()
{
   emit rb_zk_clicked();
}

void Master_export_Form::on_rb_check_all_clicked()
{
    if(!ui->rb_check->isChecked())
            emit rb_check_all();
}

void Master_export_Form::on_pb_directory_clicked()
{
    QString filename ;
    if(file_path == nullptr)
        filename = QFileDialog::getSaveFileName(
                           this,
                           tr("Save Document"),
                           QDir::currentPath(),
                           tr("Dump database (*.db)") );
    else
        filename = QFileDialog::getSaveFileName(
                           this,
                           tr("Save Document"),
                           file_path,
                           tr("Dump database (*.db)") );

    int x = filename.lastIndexOf("/");
    file_path = filename.left(x);
    if( !filename.isNull() )
        ui->le_file_path->setText(filename);
}

void Master_export_Form::on_pb_Export_clicked()
{
    emit TESTING_export(ui->le_file_path->text(), ui->le_password->text(),
                        ui->cb_off_tel->isChecked(), ui->cb_set_password->isChecked(), ui->cb_zk->isChecked());
}

void Master_export_Form::on_cb_set_password_clicked()
{
    if(ui->cb_set_password->isChecked())
     {
        ui->label->setVisible(true);
        ui->le_password->setVisible(true);
    }
    else
    {
        ui->label->setVisible(false);
        ui->le_password->setVisible(false);
    }
}

void Master_export_Form::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
 {
  case Qt::Key::Key_Enter:
        on_pb_Export_clicked();
     return;
 case Qt::Key::Key_Escape:
        emit closeThis(this->objectName());
     return;
    case Qt::Key::Key_F1:
      PopUp::instance()->setPopupText("<h2 align=\"middle\">Навигация в окне мастера экспорта</h2>"
                                      "<p><b>\"ENTER\"</b> для начала экспорта</p>"
                                      "<p><b>\"ESC\"</b> для закрытия окна мастера экспорта</p>", rightMenu);
 }
}

void Master_export_Form::add_file_path(QString path_from_main)
{
    file_path = path_from_main;
}

void Master_export_Form::focus_on_widget()
{
    ui->pb_directory->setFocus();
    set_tab_orders();
}

void Master_export_Form::set_tab_orders()
{
    ui->hided_le->setFocusProxy(ui->pb_directory);
    setTabOrder(  ui->pb_directory, ui->cb_off_tel);
     setTabOrder(  ui->cb_off_tel, ui->cb_zk);
     setTabOrder(  ui->cb_zk, ui->rb_check);
     setTabOrder(   ui->rb_check, ui->cb_set_password);
     setTabOrder(  ui->cb_set_password, ui->le_password);
     setTabOrder(  ui->le_password, ui->pb_Export);
     setTabOrder(  ui->pb_Export,  ui->hided_le);
}
