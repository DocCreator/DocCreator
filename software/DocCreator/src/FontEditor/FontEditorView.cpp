#include "FontEditorView.hpp"

#include "CharEditCursorItem.hpp"
#include "CharEditLineItem.hpp"
#include "CharEditScene.hpp"
#include "FontEditor/CharEditView.hpp"
#include "FontEditorController.hpp"
#include "VirtualKeyboard/KeyboardController.hpp"
#include "context/fontcontext.h"
#include "iomanager/fontfilemanager.h"
#include "models/character.h"
#include "models/font.h"

FontEditorView::FontEditorView(FontEditorController *controller,
                               QWidget *parent)
  : QWidget(parent)
  , _ch(nullptr)
  , _chCenterFontName()
  , _scene(nullptr)
  , _view(nullptr)
  , _controller(controller)
  , _keyboardController(nullptr)
{
  _controller->setView(this);

  _ui.setupUi(this);

  _scene = new CharEditScene();
  _view = new CharEditView(_scene, this);

  _ui.mainLayout->addWidget(_view);

  _ui.upLineSpinBox->setValue(0);
  _ui.baseLineSpinBox->setValue(0);
  _ui.leftLineSpinBox->setValue(0);
  _ui.rightLineSpinBox->setValue(0);

  createActions();
}

/* Setters */
void
FontEditorView::setKeyboardController(KeyboardController *keyboardController)
{
  _keyboardController = keyboardController;
}

void
FontEditorView::setCenterChar(Models::Character *ch)
{
  _ch = ch;
  _chCenterFontName =
    Context::FontContext::instance()->getCurrentFont()->getName();
  _scene->setCenterChar(ch);
}

void
FontEditorView::setLeftChar(const Models::Character *ch)
{
  _scene->setLeftChar(ch);
}

void
FontEditorView::setRightChar(const Models::Character *ch)
{
  _scene->setRightChar(ch);
}

void
FontEditorView::addCharacter(Models::Character *ch)
{
  if (_scene->getCenterCharItem() == nullptr) {
    setCenterChar(ch);
  }
  else {
    switch (_scene->getCursorItem()->getCursorPosition()) {
      case RIGHT:
        setRightChar(ch);
        break;
      case LEFT:
        setLeftChar(ch);
        break;
      default:
        setCenterChar(ch);
    }
  }
}

/* Events */
void
FontEditorView::keyPressEvent(QKeyEvent *e)
{
  if (_keyboardController == nullptr) {
    return;
  }

  _keyboardController->keyPressEvent(e);
}

void
FontEditorView::keyReleaseEvent(QKeyEvent *e)
{
  if (_keyboardController == nullptr) {
    return;
  }

  _keyboardController->keyReleaseEvent(e);
}

/* Other methods */
void
FontEditorView::createActions()
{
  QObject::connect(_scene->getUpLineItem(),
                   SIGNAL(moving(int)),
                   this,
                   SLOT(updateUpLineSpinBox(int)));
  QObject::connect(_scene->getBaseLineItem(),
                   SIGNAL(moving(int)),
                   this,
                   SLOT(updateBaseLineSpinBox(int)));
  QObject::connect(_scene->getLeftLineItem(),
                   SIGNAL(moving(int)),
                   this,
                   SLOT(updateLeftLineSpinBox(int)));
  QObject::connect(_scene->getRightLineItem(),
                   SIGNAL(moving(int)),
                   this,
                   SLOT(updateRightLineSpinBox(int)));

  QObject::connect(_ui.upLineSpinBox,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(updateUpLineItem(int)));
  QObject::connect(_ui.baseLineSpinBox,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(updateBaseLineItem(int)));
  QObject::connect(_ui.leftLineSpinBox,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(updateLeftLineItem(int)));
  QObject::connect(_ui.rightLineSpinBox,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(updateRightLineItem(int)));

  //"Apply" save button for base line
  QObject::connect(
    _ui.saveButton, SIGNAL(clicked()), this, SLOT(saveCharacter()));
  //QObject::connect( _ui.cancelButton, SIGNAL(clicked()), this, SLOT(close()) );
}

void
FontEditorView::closeEvent(QCloseEvent * /*event*/)
{
  _ui.dockWidget->close();
  //event = nullptr; //B
}

/*When the baseline is changed on the GUI*/
void
FontEditorView::saveCharacter()
{
  if (_ch != nullptr) {

    //B:integer divisions ?
    const qreal up =
      ((_scene->getUpLineItem()->y() - _scene->getCenter().y()) * 100) /
      _scene->getCenterCharItem()->pixmap().height();
    const qreal base =
      ((_scene->getBaseLineItem()->y() - _scene->getCenter().y()) * 100) /
      _scene->getCenterCharItem()->pixmap().height();
    const qreal left =
      ((_scene->getLeftLineItem()->x() - _scene->getCenter().x()) * 100) /
      _scene->getCenterCharItem()->pixmap().width();
    const qreal right =
      ((_scene->getRightLineItem()->x() - _scene->getCenter().x()) * 100) /
      _scene->getCenterCharItem()->pixmap().width();
    _ch->setAllBaseLines(up, base, left, right);
    //FontFileManager::

    //int result=
    IOManager::FontFileManager::saveBaseLineInformation(
      Context::FontContext::instance()->currentFontPath(),
      base,
      right,
      _ch->getCharacterValue());
    //TODO: handle failure

    Context::FontContext::instance()->setCurrentFont(_chCenterFontName);
  }
}

void
FontEditorView::updateUpLineSpinBox(int pos)
{
  if (_scene->getCenterCharItem() != nullptr) {
    const qreal percentage = ((pos - _scene->getCenter().y()) * 100) /
                             _scene->getCenterCharItem()->pixmap().height();
    _ui.upLineSpinBox->setValue(percentage);
  }
}

void
FontEditorView::updateBaseLineSpinBox(int pos)
{
  if (_scene->getCenterCharItem() != nullptr) {
    const qreal percentage = ((pos - _scene->getCenter().y()) * 100) /
                             _scene->getCenterCharItem()->pixmap().height();
    _ui.baseLineSpinBox->setValue(percentage);
  }
}

void
FontEditorView::updateLeftLineSpinBox(int pos)
{
  if (_scene->getCenterCharItem() != nullptr) {
    const qreal percentage = ((pos - _scene->getCenter().x()) * 100) /
                             _scene->getCenterCharItem()->pixmap().width();
    _ui.leftLineSpinBox->setValue(percentage);
  }
}

void
FontEditorView::updateRightLineSpinBox(int pos)
{
  if (_scene->getCenterCharItem() != nullptr) {
    qreal percentage = ((pos - _scene->getCenter().x()) * 100) /
                       _scene->getCenterCharItem()->pixmap().width();
    _ui.rightLineSpinBox->setValue(percentage);
  }
}

void
FontEditorView::updateUpLineItem(int value)
{
  if (_scene->getCenterCharItem() != nullptr) {
    const qreal pos =
      ((value * _scene->getCenterCharItem()->pixmap().height()) / 100) +
      _scene->getCenter().y(); //B:integer division ?
    _scene->setUpLinePos(pos, false);
  }
}

void
FontEditorView::updateBaseLineItem(int value)
{
  if (_scene->getCenterCharItem() != nullptr) {
    const qreal pos =
      ((value * _scene->getCenterCharItem()->pixmap().height()) / 100) +
      _scene->getCenter().y(); //B:integer division ?
    _scene->setBaseLinePos(pos, false);
  }
}

void
FontEditorView::updateLeftLineItem(int value)
{
  if (_scene->getCenterCharItem() != nullptr) {
    const qreal pos =
      ((value * _scene->getCenterCharItem()->pixmap().width()) / 100) +
      _scene->getCenter().x(); //B:integer division ?
    _scene->setLeftLinePos(pos, false);
  }
}

void
FontEditorView::updateRightLineItem(int value)
{
  if (_scene->getCenterCharItem() != nullptr) {
    const qreal pos =
      ((value * _scene->getCenterCharItem()->pixmap().width()) / 100) +
      _scene->getCenter().x(); //B:integer division ?
    _scene->setRightLinePos(pos, false);
  }
}
