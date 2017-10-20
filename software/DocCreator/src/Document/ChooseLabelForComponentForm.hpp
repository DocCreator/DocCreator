#ifndef CHOOSELABELFORCOMPONENTFORM_HPP
#define CHOOSELABELFORCOMPONENTFORM_HPP

#include "models/font.h"
#include <QMap>
#include <QWidget>

class QComboBox;
class QCheckBox;
class QListWidget;
class QListWidgetItem;
class QTableWidget;

class DocumentController;
namespace Doc {
class DocComponent;
class DocComponentBlock;
}

namespace Ui {
class ChooseLabelForComponentForm;
}

class ChooseLabelForComponentForm : public QWidget
{
  Q_OBJECT

public:
  explicit ChooseLabelForComponentForm(
    DocumentController *docController,
    Models::Font *currentFont,
    const QList<Doc::DocComponentBlock *> &componentBlocks,
    QWidget *parent = 0);

  ~ChooseLabelForComponentForm();

public slots:
  void close();
  void smallItemClicked(QListWidgetItem *item);
  void mediumItemClicked(QListWidgetItem *item);
  void bigItemClicked(QListWidgetItem *item);
  void btnLabelingClicked();
  void createNewFont();
  void isBackGround();
  void isImageBlock();
  void loadItems();
  void isSelectAll();

private:
  int getExistingID(const QString &file_code, const QString &filePath) const;
  void splitListComponentBlocks();
  void initialize();
  void loadTable();
  void reset();
  void fillScanlineToImage(QImage &image,
                           const Doc::DocComponent *dc,
                           int dx = 0,
                           int dy = 0);
  /*
  QImage getCharacterDataOf_display(int &x_i, int &y_i, QListWidgetItem* main_item, const QString &display,
                                      const QMap<QListWidgetItem*, Doc::DocComponentBlock*> &map_Items_ComponentBlocks,
                                      const QMap<Doc::DocComponentBlock*, QImage> &map_Component_Imagedata,
                                      const QMap<Doc::DocComponentBlock*, bool> &map_ComponentBlock_DisplayForListWidget);
  */

  void createListItems();

private:
  // ui component
  Ui::ChooseLabelForComponentForm *ui;
  QListWidget *_listWidgetSmall;
  QListWidget *_listWidgetMedium;
  QListWidget *_listWidgetBig;
  QTableWidget *_tableOfLabeledComponents;
  QCheckBox *_chkOneLettre;
  QComboBox *_cbPositions;
  // variable local

  //B:TODO:UGLY Do we need so many maps ???

  QList<Doc::DocComponentBlock *> _componentBlocks, _smallCBlocks,
    _MediumCBlocks, _bigCBlocks;
  QMap<Doc::DocComponentBlock *, bool>
    _map_SmallComponentBlock_DisplayForListWidget;
  QMap<Doc::DocComponentBlock *, bool>
    _map_MediumComponentBlock_DisplayForListWidget;
  QMap<Doc::DocComponentBlock *, bool>
    _map_BigComponentBlock_DisplayForListWidget;
  QMap<QListWidgetItem *, Doc::DocComponentBlock *>
    _map_Items_SmallComponentBlocks;
  QMap<QListWidgetItem *, Doc::DocComponentBlock *>
    _map_Items_MediumComponentBlocks;
  QMap<QListWidgetItem *, Doc::DocComponentBlock *>
    _map_Items_BigComponentBlocks;
  QMap<Doc::DocComponentBlock *, QImage> _map_SmallComponent_Imagedata;
  QMap<Doc::DocComponentBlock *, QImage> _map_MediumComponent_Imagedata;
  QMap<Doc::DocComponentBlock *, QImage> _map_BigComponent_Imagedata;

  QMap<QString, QMap<Doc::DocComponentBlock *, QImage> *>
    _map_display_block_image;

  QImage _originalImage;
  Models::Font *_currentFont;
  Models::CharacterMap *_currentCharacterMap;
  DocumentController *_docController;
  int _imageBlockID;
  int _upLine;
  int _baseLine;
  int _leftLine;
  int _rightLine;
  QVector<int> _listBaseLines;
};

#endif // CHOOSELABELFORCOMPONENTFORM_HPP
