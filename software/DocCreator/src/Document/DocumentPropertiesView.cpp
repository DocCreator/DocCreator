#include "DocumentPropertiesView.hpp"

#include <cassert>

#include <QCheckBox>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>

#include "DocumentController.hpp"

DocumentPropertiesView::DocumentPropertiesView(DocumentController *controller,
                                               QWidget *parent)
  : QWidget(parent)
  , _controller(controller)
{
  assert(_controller);

  _intValidator = new QIntValidator(this);
  //B: a page of at least one pixel
  auto pageSizeValidator = new QIntValidator(1, 1000000, this);

  QLabel *pageWidthL = new QLabel(tr("&Page width:"), this);
  _pageWidth =
    new QLineEdit(QString::number(_controller->getPageWidth()), this);
  _pageWidth->setValidator(pageSizeValidator);
  pageWidthL->setBuddy(_pageWidth);
  connect(_pageWidth, SIGNAL(returnPressed()), this, SLOT(changePageWidth()));
  connect(_pageWidth, SIGNAL(editingFinished()), this, SLOT(changePageWidth()));

  QLabel *pageHeightL = new QLabel(tr("&Page height:"), this);
  _pageHeight =
    new QLineEdit(QString::number(_controller->getPageHeight()), this);
  _pageHeight->setValidator(pageSizeValidator);
  pageHeightL->setBuddy(_pageHeight);
  connect(_pageHeight, SIGNAL(returnPressed()), this, SLOT(changePageHeight()));
  connect(
    _pageHeight, SIGNAL(editingFinished()), this, SLOT(changePageHeight()));

  QLabel *lineSpacingL = new QLabel(tr("&Line spacing:"), this);
  _lineSpacingLineEdit = new QLineEdit(
    QString::number(_controller->getParagraphLineSpacing()), this);
  lineSpacingL->setBuddy(_lineSpacingLineEdit);
  connect(_lineSpacingLineEdit,
          SIGNAL(returnPressed()),
          this,
          SLOT(changeLineSpacing()));
  connect(_lineSpacingLineEdit,
          SIGNAL(editingFinished()),
          this,
          SLOT(changeLineSpacing()));

  QLabel *marginTopL = new QLabel(tr("&Margin-top:"), this);
  _marginTopLineEdit =
    new QLineEdit(QString::number(_controller->getBlockMarginTop()), this);
  _marginTopLineEdit->setValidator(_intValidator);
  marginTopL->setBuddy(_marginTopLineEdit);
  connect(
    _marginTopLineEdit, SIGNAL(returnPressed()), this, SLOT(changeMarginTop()));
  connect(_marginTopLineEdit,
          SIGNAL(editingFinished()),
          this,
          SLOT(changeMarginTop()));

  QLabel *marginBottomL = new QLabel(tr("&Margin-bottom:"), this);
  _marginBottomLineEdit =
    new QLineEdit(QString::number(_controller->getBlockMarginBottom()), this);
  _marginBottomLineEdit->setValidator(_intValidator);
  marginBottomL->setBuddy(_marginBottomLineEdit);
  connect(_marginBottomLineEdit,
          SIGNAL(returnPressed()),
          this,
          SLOT(changeMarginBottom()));
  connect(_marginBottomLineEdit,
          SIGNAL(editingFinished()),
          this,
          SLOT(changeMarginBottom()));

  QLabel *marginLeftL = new QLabel(tr("&Margin-left:"), this);
  _marginLeftLineEdit =
    new QLineEdit(QString::number(_controller->getBlockMarginLeft()), this);
  _marginLeftLineEdit->setValidator(_intValidator);
  marginLeftL->setBuddy(_marginLeftLineEdit);
  connect(_marginLeftLineEdit,
          SIGNAL(returnPressed()),
          this,
          SLOT(changeMarginLeft()));
  connect(_marginLeftLineEdit,
          SIGNAL(editingFinished()),
          this,
          SLOT(changeMarginLeft()));

  QLabel *marginRightL = new QLabel(tr("&Margin-right:"), this);
  _marginRightLineEdit =
    new QLineEdit(QString::number(_controller->getBlockMarginRight()), this);
  _marginRightLineEdit->setValidator(_intValidator);
  marginRightL->setBuddy(_marginRightLineEdit);
  connect(_marginRightLineEdit,
          SIGNAL(returnPressed()),
          this,
          SLOT(changeMarginRight()));
  connect(_marginRightLineEdit,
          SIGNAL(editingFinished()),
          this,
          SLOT(changeMarginRight()));

  QLabel *baseLineVisibleL = new QLabel(tr("&BaseLine visible:"), this);
  _baseLineVisibleCheckBox = new QCheckBox(getVisibilityString(), this);
  baseLineVisibleL->setBuddy(_baseLineVisibleCheckBox);
  connect(_baseLineVisibleCheckBox,
          SIGNAL(clicked()),
          this,
          SLOT(changeBaseLineVisibility()));

  const int minW = 100;
  _pageWidth->setMinimumWidth(minW);
  _pageHeight->setMinimumWidth(minW);
  _lineSpacingLineEdit->setMinimumWidth(minW);
  _marginTopLineEdit->setMinimumWidth(minW);
  _marginBottomLineEdit->setMinimumWidth(minW);
  _marginLeftLineEdit->setMinimumWidth(minW);
  _marginRightLineEdit->setMinimumWidth(minW);

  const int maxW = 200;
  _pageWidth->setMaximumWidth(maxW);
  _pageHeight->setMaximumWidth(maxW);
  _lineSpacingLineEdit->setMaximumWidth(maxW);
  _marginTopLineEdit->setMaximumWidth(maxW);
  _marginBottomLineEdit->setMaximumWidth(maxW);
  _marginLeftLineEdit->setMaximumWidth(maxW);
  _marginRightLineEdit->setMaximumWidth(maxW);

  auto l = new QGridLayout(this);
  l->addWidget(pageWidthL, 0, 0);
  l->addWidget(_pageWidth, 0, 1);
  l->addWidget(pageHeightL, 0, 2);
  l->addWidget(_pageHeight, 0, 3);
  l->addWidget(lineSpacingL, 1, 0);
  l->addWidget(_lineSpacingLineEdit, 1, 1);
  l->addWidget(marginTopL, 2, 0);
  l->addWidget(_marginTopLineEdit, 2, 1);
  l->addWidget(marginBottomL, 2, 2);
  l->addWidget(_marginBottomLineEdit, 2, 3);
  l->addWidget(marginLeftL, 3, 0);
  l->addWidget(_marginLeftLineEdit, 3, 1);
  l->addWidget(marginRightL, 3, 2);
  l->addWidget(_marginRightLineEdit, 3, 3);
  l->addWidget(baseLineVisibleL, 4, 0);
  l->addWidget(_baseLineVisibleCheckBox, 4, 1);

  l->setColumnStretch(4, 1);

  setLayout(l);
}

void
DocumentPropertiesView::keyPressEvent(QKeyEvent *e)
{
  QWidget::keyPressEvent(e);
}

void
DocumentPropertiesView::keyReleaseEvent(QKeyEvent *e)
{
  QWidget::keyReleaseEvent(e);
}

Mvc::IController *
DocumentPropertiesView::getController()
{
  return _controller;
}

void
DocumentPropertiesView::changePageWidth()
{
  //B: the fields were filled before a document existed, with -1, and a
  // focus out (editingFinished() with Qt 5) set the width to -1. Only a
  // positive size that changes is set.
  const int pageWidth = _pageWidth->text().toInt();
  if (pageWidth > 0 && pageWidth != _controller->getPageWidth()) {
    _controller->setPageWidth(pageWidth);
  }
}

void
DocumentPropertiesView::changePageHeight()
{
  const int pageHeight = _pageHeight->text().toInt(); //B: cf changePageWidth()
  if (pageHeight > 0 && pageHeight != _controller->getPageHeight()) {
    _controller->setPageHeight(pageHeight);
  }
}

void
DocumentPropertiesView::changeLineSpacing()
{
  QString lineSpacingStr = _lineSpacingLineEdit->text();
  _controller->setParagraphLineSpacing(lineSpacingStr.toInt());
}

void
DocumentPropertiesView::changeMarginTop()
{
  QString marginTopStr = _marginTopLineEdit->text();
  _controller->setBlockMarginTop(marginTopStr.toInt());
}

void
DocumentPropertiesView::changeMarginBottom()
{
  QString marginBottomStr = _marginBottomLineEdit->text();
  _controller->setBlockMarginBottom(marginBottomStr.toInt());
}

void
DocumentPropertiesView::changeMarginLeft()
{
  QString marginLeftStr = _marginLeftLineEdit->text();
  _controller->setBlockMarginLeft(marginLeftStr.toInt());
}

void
DocumentPropertiesView::changeMarginRight()
{
  QString marginRightStr = _marginRightLineEdit->text();
  _controller->setBlockMarginRight(marginRightStr.toInt());
}

void
DocumentPropertiesView::changeBaseLineVisibility()
{
  _controller->setBaseLineVisibility(!_controller->baseLineVisibility());
  _baseLineVisibleCheckBox->setText(getVisibilityString());
}

QString
DocumentPropertiesView::getVisibilityString() const
{
  assert(_controller);
  return _controller->baseLineVisibility() ? tr("Enabled") : tr("Disabled");
}

void
DocumentPropertiesView::update()
{
  //B: the page size was not updated: the -1 of the construction stayed
  _pageWidth->setText(QString::number(_controller->getPageWidth()));
  _pageHeight->setText(QString::number(_controller->getPageHeight()));
  _lineSpacingLineEdit->setText(
    QString::number(_controller->getParagraphLineSpacing()));
  _marginTopLineEdit->setText(
    QString::number(_controller->getBlockMarginTop()));
  _marginBottomLineEdit->setText(
    QString::number(_controller->getBlockMarginBottom()));
  _marginLeftLineEdit->setText(
    QString::number(_controller->getBlockMarginLeft()));
  _marginRightLineEdit->setText(
    QString::number(_controller->getBlockMarginRight()));
}
