#include "master_import_Form.h"
#include "ui_master_import_form.h"
#include "popup.h"

/// REFACTORED

Master_import_form::Master_import_form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Master_import_form)
{
    ui->setupUi(this);
    ui->le_password_2->setEchoMode(QLineEdit::Password);
    this->actual_size = this->size();
    this->setTabOrder(ui->le_file_path_2, ui->le_password_2);
    this->folder = false;
    this->oldData = false;
    this->file_path = "";
}

Master_import_form::~Master_import_form()
{
    delete ui;
}
void Master_import_form::on_pushButton_clicked()
{
    emit TESTING_open(ui->le_file_path_2->text(), ui->le_password_2->text(), this->folder, this->oldData );
}

void Master_import_form::on_pb_directory_2_clicked()
{
    QMessageBox msgbx;
    msgbx.setWindowTitle("Сделайте выбор");
    msgbx.setText("Выберите, откуда будет производиться импорт");
    msgbx.setStandardButtons(QMessageBox::Ok |  QMessageBox::Save |  QMessageBox::Cancel);

    msgbx.setButtonText(QMessageBox::Save, "Импорт из старой ЗК");
    msgbx.setButtonText(QMessageBox::Ok, "Импорт ЗК/Служебных телефонов");
    msgbx.setButtonText(QMessageBox::Cancel, "Отмена");


    QString filename;
    int ret = msgbx.exec();
    
    if(ret == QMessageBox::Save)
    {
                QMessageBox temp;
                temp.setWindowTitle("Сделайте выбор");
                temp.setText("Выберите, откуда будет производиться импорт");
                temp.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                temp.setButtonText(QMessageBox::Ok, "Импорт из папки");
                temp.setButtonText(QMessageBox::Cancel, "Импорт выбранных файлов");
                int s = temp.exec();
                switch(s)
                {
                //Ипорт из ПАПКИ!!!
                    case QMessageBox::Ok :
                    this->folder = true;
                    this->oldData = true;
                    if(this->file_path == 0)
                    {
                        filename = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                                       "/home",
                                                                       QFileDialog::ShowDirsOnly
                                                                       | QFileDialog::DontResolveSymlinks);
                        this->file_path = filename;
                        if( !filename.isNull() )
                        {
                            ui->le_file_path_2->setText(filename);
                        }
                        return;
                    }
                        else
                    {
                        filename = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                                     "/home",
                                                                     QFileDialog::ShowDirsOnly
                                                                     | QFileDialog::DontResolveSymlinks);

                        this->file_path = filename;
                        if( !filename.isNull() )
                        {
                            ui->le_file_path_2->setText(filename);
                        }
                        return;
                    }
                case QMessageBox::Cancel:
                 // Импорт старых ЗК
                    this->oldData = true;
                    if(this->file_path == 0)
                    {
                        filename = QFileDialog::getOpenFileName(
                                    this,
                                    tr("Save Document"),
                                    QDir::currentPath(),
                                    tr("**"));
                        int x = filename.lastIndexOf("/");
                        this->file_path = filename.left(x);
                        this->folder = false;
                        if( !filename.isNull() )
                            ui->le_file_path_2->setText(filename);
                        return;
                    }
                        else
                    {
                        filename = QFileDialog::getOpenFileName(
                                    this,
                                    tr("Save Document"),
                                    this->file_path,
                                    tr("**") );
                        int x = filename.lastIndexOf("/");
                        this->file_path = filename.left(x);
                        this->folder = false;

                        if( !filename.isNull() )
                        {
                            ui->le_file_path_2->setText(filename);
                        }
                        return;
                    }
                default:
                    return;
                }

    }
    else
    if(ret == QMessageBox::Ok)
        {
        // ЗК/Служебные
        this->folder =false;
        this->oldData=false;
          if(this->file_path == 0)
          {
              filename = QFileDialog::getOpenFileName(
                          this,
                          tr("Save Document"),
                          QDir::currentPath(),
                          tr("*.nbd; *.nbds; *.nbod; *.nbods") );
              int x = filename.lastIndexOf("/");
              this->file_path = filename.left(x);
              this->folder = false;

              if( !filename.isNull() )
                  ui->le_file_path_2->setText(filename);

              return;
          }
          else
          {

              filename = QFileDialog::getOpenFileName(
                          this,
                          tr("Save Document"),
                          this->file_path,
                          tr("*.nbd; *.nbds; *.nbod; *.nbods") );
              int x = filename.lastIndexOf("/");
              this->file_path = filename.left(x);
              this->folder = false;

              if( !filename.isNull() )
                  ui->le_file_path_2->setText(filename);

              return;
          }
    }

}

void Master_import_form::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
 {
  case Qt::Key_Enter:
     on_pushButton_clicked();
     return;
    case Qt::Key_Escape:
        emit closeThis(this->objectName());
      return;
    case Qt::Key_F1:
      PopUp::instance()->setPopupText("<h2 align=\"middle\">Навигация в окне мастера импорта</h2>"
                                      "<p><b>\"ENTER\"</b> для начала импорта</p>"
                                      "<p><b>\"ESC\"</b> для закрытия окна мастера импорта</p>", rightMenu);
    }
}

void Master_import_form::set_tab_orders()
{
    ui->pb_directory_2->setFocus();
    ui->hided_le->setFocusProxy(ui->pb_directory_2);
    setTabOrder(ui->pb_directory_2, ui->le_password_2);
    setTabOrder(ui->le_password_2, ui->pushButton);
    setTabOrder(ui->pushButton,  ui->hided_le);
}

void Master_import_form::focus_on_widget()
{
    ui->pb_directory_2->setFocus();
}
