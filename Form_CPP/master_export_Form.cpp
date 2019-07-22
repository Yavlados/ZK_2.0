#include "master_export_Form.h"
#include "ui_master_export_form.h"

#include <QLineEdit>

Master_export_Form::Master_export_Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::master_export_Form)
{
    ui->setupUi(this);
    on_cb_zk_clicked();
    on_cb_set_password_clicked();
    ui->le_password->setEchoMode(QLineEdit::Password);
}

Master_export_Form::~Master_export_Form()
{
    delete ui;
}

void Master_export_Form::on_cb_zk_clicked()
{
    if(ui->cb_zk->isChecked())
     {
        ui->rb_check->setVisible(true);
        ui->rb_check_all->setVisible(true);
    }
    else
    {
        ui->rb_check->setVisible(false);
        ui->rb_check_all->setVisible(false);
    }
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
    QString filename = QFileDialog::getSaveFileName(
                       this,
                       tr("Save Document"),
                       QDir::currentPath(),
                       tr("Dump database (*.db)") );

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