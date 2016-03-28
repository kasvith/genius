#include "manager.h"
#include "ui_manager.h"

Manager::Manager(ClipboardHistory *history, QWidget *parent, bool hidden) :
  QMainWindow(parent),
  ui(new Ui::Manager)
{
  ui->setupUi(this);
  _starthidden=hidden;
  if(history)
    _history=history;

}

Manager::~Manager()
{
  delete ui;
}

void Manager::addTextItem(QString *text, QString *tooltipText, int reference, int index)
{
  QListWidgetItem *qlwi=new QListWidgetItem();
  qlwi->setText(*text);
  qlwi->setToolTip(*tooltipText);
  qlwi->setData(50,reference);
  ui->list->insertItem(index,qlwi);
}

void Manager::addImageItem(QString *text, QIcon *icon,int reference,int index)
{
  QListWidgetItem *qlwi=new QListWidgetItem();
  qlwi->setIcon(*icon);
  qlwi->setText(*text);
  qlwi->setData(50,reference);
  ui->list->insertItem(index,qlwi);
}

void Manager::removeItem(int reference)
{
  int count=ui->list->count();
  if(count>0)
  {
    QListWidgetItem *item;
    for(int i=0;i<count;i++)
    {
      item=ui->list->item(i);
      if(item)
      {
        int ref=item->data(50).toInt();
        if(ref==reference)
        {
          delete item;
          return;
        }
      }
    }
  }
}

void Manager::clearList()
{
  int length=ui->list->count();
  QListWidgetItem *item;
  for(int i=0;i<length;i++)
  {
    item=ui->list->item(i);
    if(item)
    {
      delete item;
    }
  }
  ui->list->clear();
}



void Manager::updateTextItem(QString *text, QString *tooltipText, int reference)
{
  int length=ui->list->count();
  if(length>0)
  {
   QListWidgetItem *item;
   int ref=0;
   for(int i=0;i<length;i++)
   {
     item=ui->list->item(i);
     if(item)
     {
       ref=item->data(50).toInt();
       if(ref==reference)
       {
          item->setText(*text);
          item->setToolTip(*tooltipText);
          return;
       }
     }
   }
  }
}

void Manager::updateImageItem(QString *text, QIcon *icon, int reference)
{
  int length=ui->list->count();
  if(length>0)
  {
   QListWidgetItem *item;
   int ref=0;
   for(int i=0;i<length;i++)
   {
     item=ui->list->item(i);
     if(item)
     {
       ref=item->data(50).toInt();
       if(ref==reference)
       {
          item->setIcon(*icon);
          item->setText(*text);
          return;
       }
     }
   }
  }
}

void Manager::initialize()
{
  initializeUI();
}

void Manager::initializeUI()
{
  ui->centralwidget->setLayout(ui->rootLayout);
  setCentralWidget(ui->centralwidget);
}


void Manager::on_deleteButton_clicked()
{
  int currentRow=ui->list->currentRow();
  if(_history->isEmpty()==false && currentRow>-1)
  {
    QListWidgetItem *item=ui->list->item(currentRow);
    if(item)
    {
      _history->remove(item->data(50).toInt());
    }
  }
}

void Manager::on_settingsButton_clicked()
{
    emit settingsDialogRequested();
}

void Manager::on_clearButton_clicked()
{
  if(_history && !_history->isEmpty())
  {
    _history->clear();
  }
}

void Manager::on_list_itemDoubleClicked(QListWidgetItem *item)
{
  if(item)
  {
    int ref=item->data(50).toInt();
    emit itemSelected(ref);
  }
}

//void Manager::on_editButton_clicked()
//{
//  if(_history->isEmpty()==false)
//  {
//    int length=ui->list->count();
//    if(length>0)
//    {
//      int currentIndex=ui->list->currentRow();
//      if(currentIndex>-1)
//      {
//        QListWidgetItem *item=ui->list->item(currentIndex);
//        if(item)
//        {
//          int ref=item->data(50).toInt();
//          ClipboardEntity *Clenty=_history->get(ref);
//          if(Clenty)
//          {
//            emit editRequested(Clenty);
//            return;
//          }
//        }
//      }
//    }

//  }
//}


void Manager::on_list_customContextMenuRequested(const QPoint &pos)
{

  QListWidgetItem* item=ui->list->itemAt(pos);

  if(!item)
     return;
   int reference=item->data(50).toInt();
   ClipboardEntity* cpEnt=_history->get(reference);
   if(cpEnt)
   {
     QMenu contextMenu(this);

     QAction edit(&contextMenu);

     edit.setIcon(QIcon(Resources::show16));
     edit.setData("S");
     edit.setIconText("show content");

     QAction del(&contextMenu);
     del.setIcon(QIcon(Resources::dustBin16));
     del.setIconText("delete");
     del.setData("D");

     contextMenu.addAction(&edit);
     contextMenu.addAction(&del);

     connect(&contextMenu,SIGNAL(triggered(QAction*)),this,SLOT(itemContextMenuTriggered(QAction*)));
     contextMenu.exec(ui->list->mapToGlobal(pos));
   }
}

void Manager::itemContextMenuTriggered(QAction *action)
{
  int length=ui->list->count();
  if(length>0)
  {
    int selected=ui->list->currentRow();
    if(selected>-1)
    {
      QListWidgetItem *qlwi=ui->list->item(selected);
      if(qlwi)
      {
        int reference=qlwi->data(50).toInt();
        ClipboardEntity *entity=_history->get(reference);
        if(entity)
        {
          QString com=action->data().toString();
          if(com=="S")
          {
            emit showContentRequested(entity);
          }
          else if(com=="D")
          {
            _history->remove(reference);
          }
          else return;
        }
      }
    }
  }

}

void Manager::on_showContentButton_clicked()
{
  int length=ui->list->count();
  if(length>0)
  {
    int currentIndex=ui->list->currentRow();
    if(currentIndex>-1)
    {
      QListWidgetItem *item=ui->list->item(currentIndex);
      if(item)
      {
        int ref=item->data(50).toInt();
        ClipboardEntity *Clenty=_history->get(ref);
        if(Clenty)
        {
          emit showContentRequested(Clenty);
          return;
        }
      }
    }
  }
}


void Manager::showEvent(QShowEvent *event)
{
  if(event)
  {
    if(_starthidden)
    {
      hide();
    }
    emit shown();
  }
}

void Manager::hideEvent(QHideEvent *event)
{
  if(event)
  {
    emit hidden();
  }
}

void Manager::closeEvent(QCloseEvent *event)
{
  event->ignore();
  hide();
}

