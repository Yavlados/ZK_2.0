#include "_MTM_Event.h"

MTM_Event::MTM_Event(QObject *parent):
    QAbstractTableModel(parent)
{
    this->eventList = 0  ;
    this->showing_count = Settings_connection::instance()->showing_count;
    this->columnsCount = 9;
}

void MTM_Event::setEventList(QList<Event *> *eventList)
{
    beginResetModel();

    this->eventList = eventList;
    this->actEventList.clear();

    if(this->eventList!=0)
    {
        for(int i=0;( i < showing_count) && (i < this->eventList->size()); i++)
            if( this->eventList->at(i)->state!=IsRemoved )
                this->actEventList.append(this->eventList->at(i));
    }

    endResetModel();
}

QVariant MTM_Event::data(const QModelIndex &index, int role) const
{
    if( !index.isValid() || this->eventList == 0)
        return QVariant();

    int row = index.row();      ///целочисленные указатели на строку
    int col = index.column();   /// и столбец
    if (role == Qt::DisplayRole)
    {
        switch (col) {
            case 1:
                return this->actEventList.at(row)->id;
            case 2:
                return this->actEventList.at(row)->category;
            case 3:
                return this->actEventList.at(row)->detention_date;
            case 4:
                return this->actEventList.at(row)->detention_time;
            case 5:
                return this->actEventList.at(row)->detention_reason;
            case 6:
                return this->actEventList.at(row)->detention_by;
            case 7:
                return this->actEventList.at(row)->keeping_place;
            case 8:
                return this->actEventList.at(row)->additional;
        }
    }
    if (role == Qt::CheckStateRole && col == 0)  // this shows the checkbox
            {
                bool aBool = actEventList.at(row)->checkState_;
                if (aBool)
                        return Qt::Checked;
                else
                        return Qt::Unchecked;
            }

    return QVariant();
}

QVariant MTM_Event::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::CheckStateRole)
            return QVariant();

    if (role == Qt::DisplayRole)
         if (orientation == Qt::Horizontal)
             switch (section)
           {
             case 0:
                 return "Все";
             case 1:
                 return "ID";
             case 2:
                 return "Категория";
             case 3:
                 return "Дата";
             case 4:
                 return "Время";
             case 5:
                 return "Причина";
             case 6:
                 return "Задержан";
             case 7:
                 return "Место";
             case 8:
                 return "Доп. инфо";
             }
     return QVariant(); /// вот сюда внимание в случае краша
}

int MTM_Event::columnCount(const QModelIndex &parent) const
{
    (void)parent;
    if(this->eventList==0)
        return 0;
    else return this->columnsCount;
//            Settings_connection::instance()->columns_count;
}

int MTM_Event::rowCount(const QModelIndex &parent) const
{
    (void)parent;
    if (this->eventList == 0)
        return 0;
    else return this->actEventList.size();
}

Qt::ItemFlags MTM_Event::flags(const QModelIndex &index) const
{
    if( !index.isValid() || this->eventList==0 )
        return Qt::NoItemFlags;
    else
        return  Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

bool MTM_Event::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( !index.isValid() || this->eventList == 0 )
        return false;

    int row = index.row();      ///целочисленные указатели на строку
    int col = index.column();   /// и столбец

    if( row>this->actEventList.size() || row<0 )
            return false;

    if (role == Qt::CheckStateRole)
     {
      if (this->actEventList.at(row)->checkState_ == Unchecked_)
           this->actEventList.at(row)->checkState_ = Checked_;
       else
        if (this->actEventList.at(row)->checkState_ == Checked_)
              this->actEventList.at(row)->checkState_= Unchecked_;

        emit dataChanged(index,index);
          return true;
      }
         return false;

}

void MTM_Event::next_page()
{
    beginResetModel();
    int a;
    a = this->eventList->indexOf(this->actEventList.at(this->actEventList.size()-1)); //индекс последнего элемента
    int b = a+1; //индекс добавления
    this->actEventList.clear();

    if(this->eventList!=0)
    {
        while (b < a+this->showing_count+1 && b < this->eventList->size())
        {
            if( this->eventList->at(b)->state!=IsRemoved )
            {
                this->actEventList.append(this->eventList->at(b));
                b++;
            }
        }
    }

    endResetModel();
}

void MTM_Event::previous_page()
{
    beginResetModel();
    int a;
    a = this->eventList->indexOf(this->actEventList.at(0)); //индекс последнего элемента
    int b = a-this->showing_count; //индекс добавления

    this->actEventList.clear();

    if(this->eventList!=0)
    {
        while (b < a)
        {
          if( this->eventList->at(b)->state!=IsRemoved )
          {
              this->actEventList.append(this->eventList->at(b));
              b++;
          }
        }
    }
    endResetModel();
}

void MTM_Event::changeCheckedStateEventList(bool checkState)
{
    beginResetModel();

    this->actEventList.clear();
    int iterator = 0;

    if(this->eventList!=0)
    {
        for(int i=0; i < this->eventList->size(); i++)
            if( this->eventList->at(i)->state!=IsRemoved )
            {
                this->eventList->at(i)->checkState_ = checkState ? Checked_ : Unchecked_;
                if (iterator < showing_count && iterator < this->eventList->size())
                {
                    this->actEventList.append(this->eventList->at(iterator));
                    iterator++;
                }
            }
    }

    endResetModel();
}
