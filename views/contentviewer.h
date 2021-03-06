#ifndef CONTENTVIEWER_H
#define CONTENTVIEWER_H

#include <QDialog>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextBrowser>
#include <QTabWidget>
#include <QColorDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QDesktopServices>
#include <models/clipboardentity.h>
#include <resources/resources.h>

namespace Ui {
class ContentViewer;
}
/**
 * @brief this dialog can show content of an ClipboardEntity .
 */
class ContentViewer : public QDialog
{
  Q_OBJECT

public:
  /**
   * @brief initialize ContentViewer user interface with provided ClipboardEntity object
   * @param ClipboardEntity for show contents
   * @param basic parent
   */
  explicit ContentViewer(ClipboardEntity *entity,QWidget *parent=0);
  ~ContentViewer();

private:
  /**
   * @brief basic UI object
   */
  Ui::ContentViewer *ui;
  /**
   * @brief the ClipboardEntity currently showing contents
   */
  ClipboardEntity *_entity;

  /**
   * @brief initialize basic non dynamic eliments of the dialog using ClipboardEntity
   */
  void initializeBasicUI();

  /**
   * @brief add plainText tab to the tabView
   */
  void addPlainTextTab();

  /**
   * @brief add HTML tab to the window
   */
  void addHTMLTab();

  /**
   * @brief add image Tab to the window
   */
  void addImageTab();

  /**
   * @brief add urls tab to the window
   */
  void addURLsTab();
  /**
   * @brief get user friendly text of an mimeType text
   * @param MimeType text
   * @return QString
   */
  QString imageMimeTypeToText(const QString &MT);

  /**
   * @brief replace the tvFormats tabview with list of formats availabel in the entity. this function should
   * use when the content of the ClipboardEntity is cannot show.
   */
  void replaceTabViewWithFormats();
};

#endif // CONTENTVIEWER_H
