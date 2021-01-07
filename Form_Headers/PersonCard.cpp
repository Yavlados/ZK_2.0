#include "PersonCard.h"
#include "ui_PersonCard.h"
#include <QMessageBox>

PersonCard::PersonCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PersonCard)
{
    ui->setupUi(this);
}

PersonCard::~PersonCard()
{
    delete ui;
}

void PersonCard::setPerson(Person *person)
{
    this->localPerson = person;
    ui->personFio->setText(this->localPerson->name + " " +
                           this->localPerson->lastname + " " +
                           this->localPerson->midname);
}

//void PersonCard::close()
//{

//}

void PersonCard::on_editButton_clicked()
{
    emit openEditWindow(this->localPerson);
}

//void PersonCard::keyPressEvent(QKeyEvent *event)
//{
//    switch(event->key())
//    {
//    case Qt::Key_Escape:
//        this->close();
//        return;
//    }
//}

void PersonCard::on_pb_button_clicked()
{
    QMessageBox msg;
    msg.setWindowTitle("Подтверждение");
    msg.setText("Вы действительно хотите удалить выбранного фигуранта?");
    msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    int a = msg.exec();

    switch (a) {
        case QMessageBox::Ok :
        this->localPerson->state = IsRemoved;
        emit resetCardsLayout();
        return;
    case QMessageBox::Cancel :

        return;
    }

}