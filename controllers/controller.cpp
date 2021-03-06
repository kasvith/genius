#include "controller.h"

Controller::Controller(QObject *parent) : QObject(parent)
{

  _history=new ClipboardHistory();
  _clipboard=QApplication::clipboard();
  _managerOpened=false;
  _settingsWindowOpened=false;
  _selectorOpen=false;
  _paused=false;
  _holtCollection=false;
  GSettings::initialize();
}

Controller::~Controller()
{
  Resources::tempFolder.remove();
  deleteVariables();
  deleteHotkeys();
}

void Controller::itemSelected(int reference)
{
 selectItem(reference);
}


void Controller::clipboardChanged(QClipboard::Mode mode)
{
  if(mode==QClipboard::Clipboard && !_paused )
  {
    if(!_holtCollection && !isClipboardEmpty())
      addClipboardContentToHistory();
    else return;
  }
  else return;
}


void Controller::history_itemAdded(ClipboardEntity *entity, int index)
{
  addItem(entity,index);
}

void Controller::history_removed(int reference, int index)
{
  if(index>-1)
  {
    _manager->removeItem(reference);
    _trayIcon->removeItem(reference);
  }
}

void Controller::history_cleared()
{
  _manager->clearList();
  _trayIcon->clearHistoryList();
}

void Controller::manager_hidden()
{
  _managerOpened=false;
}

void Controller::manager_shown()
{
  _managerOpened=true;
}

void Controller::settingsWindowRequested()
{
  if(!_settingsWindowOpened)
  {
    _settingsWindow->show();
    _settingsWindowOpened=true;
  }
}

void Controller::showHideManagerRequest()
{
  toggleManager();
}

void Controller::selectorClosed(int currentIndex)
{

  _selectorOpen=false;
  ClipboardEntity *entity=_history->at(currentIndex);
  if(entity)
  {
    int reference=entity->ref();
    itemSelected(reference);
    if(GSettings::pasteAutomaticlay)
    {
      QThread::msleep(WAIT_BEFORE_PASTE);
      FakeKey::simulatePaste();
    }
  }
}

void Controller::settingsWindow_hidden()
{
  enableHotkeys(true);
  if(_settingsWindowOpened)
    _settingsWindowOpened=false;
}

void Controller::turnOffRequest()
{
  _holtCollection=true;
  enableHotkeys(false);
  _trayIcon->showMessage("genius is turned off","genius is successfully turned off. now any clipboard change will not be monitored",QSystemTrayIcon::Information,500);
}

void Controller::turnOnRequest()
{
  _holtCollection=false;
  enableHotkeys(true);
  _trayIcon->showMessage("genius is turned on","genius is successfully turned on. now all clipboard changes will be monitored",QSystemTrayIcon::Information,500);
}

void Controller::exitRequested()
{
  deleteVariables();
  deleteHotkeys();
  Resources::tempFolder.remove();
  exit(0);
}

void Controller::openSelectorHKtriggered()
{

  if(!_selectorOpen && _history->isEmpty()==false)
  {
    _selector->show();
    _selectorOpen=true;
  }
}

void Controller::clearHistoryHKTrigered()
{
  if(_history->isEmpty()==false)
  {
    int length=_history->length();
    _history->clear();
    _trayIcon->showMessage("history cleared",QString("clipboard history is cleared. \n%1 items was deleted").arg(length),QSystemTrayIcon::Information,1000);
  }
}

void Controller::pasteLasteHKTrigered()
{
  int length=_history->length();
  if(length>1)
  {
    ClipboardEntity *entity=_history->at(1);
    if(entity)
    {
      itemSelected(entity->ref());
      FakeKey::simulatePaste();
    }
  }
}

void Controller::openManagerHKTriggered()
{
  if(!_managerOpened)
  {
    _manager->show();
  }
}

void Controller::openSettingsHKTriggered()
{
  if(!_settingsWindowOpened)
  {
    enableHotkeys(false);
    _settingsWindow->show();
    _settingsWindowOpened=true;
  }
}

void Controller::start()
{
  GSettings::initialize();
  createViews();
  createHotkeys();
  createConnections();
  showViews();
}

void Controller::addClipboardContentToHistory()
{
  ClipboardEntity *entity=new ClipboardEntity(_clipboard);
  if(entity)
  {
    if(sameAsLast(entity)  || entity->formats().isEmpty() || entity->size()<1)
    {
      delete entity;
      return;
    }
    _history->pushFront(entity);
  }
}


void Controller::createViews()
{
  _manager=new Manager(_history);
  _manager->initialize();
  _trayIcon=new TrayIcon(_history);
  _selector=new Selector(_history);
  _settingsWindow=new SettingsWindow();
}

void Controller::showViews()
{
  if(!_managerOpened && !GSettings::openMinimized)
    _manager->show();
  _trayIcon->show();
}

void Controller::addItem(ClipboardEntity *entity, int index)
{
  if(!entity || _history->isEmpty()) return;
  int reference=entity->ref();
  if(entity->hasImage())
  {
    const QImage *image=entity->image(false,GSettings::maximumImageWidth,GSettings::maximumImageHight);
    if(image)
    {
      QIcon icon(QPixmap::fromImage(*image));
      delete image;
      QString text("  added time : "+entity->addedTime()->toString("hh.mm.ss.zzz AP"));
      _manager->addImageItem(&text,&icon,reference,index);
      _trayIcon->addImageAction(&text,&icon,reference,index);
    }
  }
  else if(entity->hasURLs())
  {
    QList<QUrl> urls=entity->urls();
    QString text=ToolKit::URLsToPreviewText(&urls,(GSettings::limitcharLength ? GSettings::limitedCharLength : 0));
    QString tooltipText=QString("added time : %1 ").arg(entity->addedTime()->toString("hh.mm.ss.zzz AP"));
    _manager->addTextItem(&text,&tooltipText,reference,index);
    _trayIcon->addTextAction(&text,&tooltipText,reference,index);
  }
  else if(entity->hasPlainText())
  {

    QString text=entity->plainText(false,(GSettings::limitcharLength ? GSettings::limitedCharLength : -1));
    if(GSettings::showInSingleLine)
      ToolKit::removeNewLines(&text);
    QString tooltipText=QString("added time : %1 ").arg(entity->addedTime()->toString("hh.mm.ss.zzz AP"));
    _manager->addTextItem(&text,&tooltipText,reference,index);
    _trayIcon->addTextAction(&text,&tooltipText,reference,index);
  }
  else if(entity->hasHTML())
  {
    QString text=entity->HTMLText(false,(GSettings::limitcharLength ? GSettings::limitedCharLength : -1));
    if(GSettings::showInSingleLine)
      ToolKit::removeNewLines(&text);
    QString tooltipText=QString("added time : %1 ").arg(entity->addedTime()->toString("hh.mm.ss.zzz AP"));
    _manager->addTextItem(&text,&tooltipText,reference,index);
    _trayIcon->addTextAction(&text,&tooltipText,reference,index);
  }

  else
  {
    QString text=QString("formats : %1 size : %2 KB")
                 .arg(entity->formats().size())
                 .arg((double)entity->size()/1024);
    QString tooltipText=QString("added time : %1 ").arg(entity->addedTime()->toString("hh.mm.ss.zzz AP"));
    _manager->addTextItem(&text,&tooltipText,reference,index);
    _trayIcon->addTextAction(&text,&tooltipText,reference,index);
  }
}

void Controller::createHotkeys()
{
  if(GSettings::selectorEnabled)
    _openSelectorHotkey=new QHotkey(QKeySequence("Ctrl+Shift+X"),true);

  if(GSettings::clearHistoryHotKeyEnabled)
     _clearHistoryHotKey=new QHotkey(GSettings::clearHistoryHotKey,true);

  if(GSettings::pasteLastHotKeyEnabled)
     _pasteLastHotKey=new QHotkey(GSettings::pasteLastHotKey,true);

  if(GSettings::openManagerHotkeyEnabled)
     _openManagerHotKey=new QHotkey(GSettings::openManagerHotkey,true);

  if(GSettings::openSettingsHotKeyEnabled)
     _openSettingsHotKey=new QHotkey(GSettings::openSettingsHotKey,true);

  if(GSettings::historyMenuHotkeyEnabled)
    _historyMenuHotKey=new QHotkey(GSettings::historyMenuHotkey,true);
}

void Controller::createConnections()
{
  connect(_clipboard,SIGNAL(changed(QClipboard::Mode)),this,SLOT(clipboardChanged(QClipboard::Mode)));

  connect(_history,SIGNAL(added(ClipboardEntity*,int)),this,SLOT(history_itemAdded(ClipboardEntity*,int)));
  connect(_history,SIGNAL(removed(int,int)),this,SLOT(history_removed(int,int)));
  connect(_history,SIGNAL(cleared()),this,SLOT(history_cleared()));
  connect(_history,SIGNAL(locationExchanged(int,int)),this,SLOT(locationExchanged(int,int)));

  connect(_manager,SIGNAL(shown()),this,SLOT(manager_shown()));
  connect(_manager,SIGNAL(hidden()),this,SLOT(manager_hidden()));
  connect(_manager,SIGNAL(hidden()),_trayIcon,SLOT(managerHidden()));
  connect(_manager,SIGNAL(shown()),_trayIcon,SLOT(managerShown()));
  connect(_manager,SIGNAL(settingsDialogRequested()),this,SLOT(settingsWindowRequested()));
  connect(_manager,SIGNAL(itemSelected(int)),this,SLOT(itemSelected(int)));
  connect(_manager,SIGNAL(showContentRequested(ClipboardEntity*)),this,SLOT(showContent(ClipboardEntity*)));
  connect(_manager,SIGNAL(locationExchangeRequested(int,int)),this,SLOT(locationExchangeRequested(int,int)));

  connect(_trayIcon,SIGNAL(showHideManagerTriggerd()),this,SLOT(showHideManagerRequest()));
  connect(_trayIcon,SIGNAL(itemSelected(int)),this,SLOT(itemSelected(int)));
  connect(_trayIcon,SIGNAL(settingsDialogRequested()),this,SLOT(settingsWindowRequested()));
  connect(_trayIcon,SIGNAL(turnOffGenius()),this,SLOT(turnOffRequest()));
  connect(_trayIcon,SIGNAL(turnOnGenius()),this,SLOT(turnOnRequest()));
  connect(_trayIcon,SIGNAL(exitRequested()),this,SLOT(exitRequested()));
  connect(_trayIcon,SIGNAL(pause()),this,SLOT(pauseRequested()));
  connect(_trayIcon,SIGNAL(resume()),this,SLOT(resumeRequested()));

  connect(_selector,SIGNAL(closing(int)),this,SLOT(selectorClosed(int)));
  connect(_settingsWindow,SIGNAL(hiding()),this,SLOT(settingsWindow_hidden()));

  if(_openSelectorHotkey)
    connect(_openSelectorHotkey,SIGNAL(activated()),this,SLOT(openSelectorHKtriggered()));

  if(_clearHistoryHotKey)
    connect(_clearHistoryHotKey,SIGNAL(activated()),this,SLOT(clearHistoryHKTrigered()));

  if(_pasteLastHotKey)
    connect(_pasteLastHotKey,SIGNAL(activated()),this,SLOT(pasteLasteHKTrigered()));

  if(_openManagerHotKey)
    connect(_openManagerHotKey,SIGNAL(activated()),this,SLOT(openManagerHKTriggered()));

  if(_openSettingsHotKey)
    connect(_openSettingsHotKey,SIGNAL(activated()),this,SLOT(openSettingsHKTriggered()));

  if(_historyMenuHotKey)
    connect(_historyMenuHotKey,SIGNAL(activated()),this,SLOT(historyMenuHotkeyActivated()));
}

void Controller::selectItem(int reference)
{
  if(reference>-1)
  {
    if(!_history->isEmpty())
    {
      ClipboardEntity *entity=_history->get(reference);
      if(entity)
      {
        QMimeData *MD=entity->data();

        if(MD)
        {
          int index=_history->indexOf(reference);
          if(index!=0)
          {
            _clipboard->setMimeData(MD);
            if(!_paused)
                _history->remove(reference);
            return;
          }
          else
          {
              if(_paused)
              {
                  _clipboard->setMimeData(MD);
                  return;
              }
          }
        }
        delete MD;
      }
    }
  }
}

bool Controller::isClipboardEmpty()
{
  const QMimeData *mimeData=_clipboard->mimeData(QClipboard::Clipboard);
  if(!mimeData)
    return true;
  else return false;
}

void Controller::deleteHotkeys()
{

  if(_openSelectorHotkey)
  {
     _openSelectorHotkey->setRegistered(false);
     delete _openSelectorHotkey;
     _openSelectorHotkey=NULL;
  }

  if(_clearHistoryHotKey)
  {
     _clearHistoryHotKey->setRegistered(false);
     delete _clearHistoryHotKey ;
     _clearHistoryHotKey=NULL;
  }

  if(_pasteLastHotKey)
  {
     _pasteLastHotKey->setRegistered(false);
     delete _pasteLastHotKey;
     _pasteLastHotKey=NULL;
  }

  if(_openManagerHotKey)
  {
     _openManagerHotKey->setRegistered(false);
     delete _openManagerHotKey;
     _openManagerHotKey=NULL;
  }

  if(_openSettingsHotKey)
  {
     _openSettingsHotKey->setRegistered(false);
     delete _openSettingsHotKey;
     _openSettingsHotKey=NULL;
  }

  if(_historyMenuHotKey)
  {
    _historyMenuHotKey->setRegistered(false);
    delete _historyMenuHotKey;
    _historyMenuHotKey=NULL;
  }
}

void Controller::deleteVariables()
{
  if(_manager)
  {
    delete _manager;
    _manager=NULL;
  }

  if(_history)
  {
    delete _history;
    _history=NULL;
  }

  if(_trayIcon)
  {
    delete _trayIcon;
    _trayIcon=NULL;
  }

  if(_selector)
  {
    delete _selector;
    _selector=NULL;
  }
}

void Controller::toggleManager()
{
  if(_managerOpened)
  {
      _manager->hide();
  }
  else
  {
      _manager->show();
  }
}


void Controller::historyMenuHotkeyActivated()
{
  _trayIcon->showHistoryMenu();
}

void Controller::enableHotkeys(bool enable)
{
  if(_openSelectorHotkey)
     _openSelectorHotkey->setRegistered(enable);


  if(_clearHistoryHotKey)
     _clearHistoryHotKey->setRegistered(enable);


  if(_pasteLastHotKey)
     _pasteLastHotKey->setRegistered(enable);

  if(_openManagerHotKey)
     _openManagerHotKey->setRegistered(enable);

  if(_openSettingsHotKey)
     _openSettingsHotKey->setRegistered(enable);

  if(_historyMenuHotKey)
    _historyMenuHotKey->setRegistered(enable);
}


bool Controller::sameAsLast(ClipboardEntity *entity)
{
  if(entity)
  {
    if(_history && !_history->isEmpty())
    {
      ClipboardEntity *first=_history->first();
      if(first)
        return first->identical(entity);
      else
        return false;
    }
    else
      return false;
  }
  else
    return false;
}

void Controller::showContent(ClipboardEntity *entity)
{
  if(entity)
  {
    ContentViewer CV(entity);
    CV.exec();
  }
}

void Controller::locationExchangeRequested(int ref1, int ref2)
{
  _history->exchangeLocation(ref1,ref2);
}

void Controller::locationExchanged(int ref1, int ref2)
{
  _manager->exchangeLocations(ref1,ref2);
  _trayIcon->exchangeLocation(ref1,ref2);
}

void Controller::pauseRequested()
{
    _paused=true;
    _trayIcon->showMessage("genius is paused","genius is now paused. new clipboard changes will not add to the history. but you can use the history.(static set of items)",QSystemTrayIcon::Information,500);

}

void Controller::resumeRequested()
{
  _paused=false;
  _trayIcon->showMessage("genius is resumed","genius is now in normal state. new clipboard changes will be add to the history",QSystemTrayIcon::Information,500);
}
