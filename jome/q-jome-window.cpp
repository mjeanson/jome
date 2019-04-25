/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QListWidget>
#include <QLabel>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <boost/algorithm/string.hpp>

#include "q-jome-window.hpp"
#include "q-cat-list-widget-item.hpp"

namespace jome {

QSearchBoxEventFilter::QSearchBoxEventFilter(QObject * const parent) :
    QObject {parent}
{
}

bool QSearchBoxEventFilter::eventFilter(QObject * const obj,
                                        QEvent * const event)
{
    if (event->type() != QEvent::KeyPress) {
        return QObject::eventFilter(obj, event);
    }

    auto keyEvent = static_cast<const QKeyEvent *>(event);

    switch (keyEvent->key()) {
    case Qt::Key_Up:
        emit this->upKeyPressed();
        break;

    case Qt::Key_Right:
        emit this->rightKeyPressed();
        break;

    case Qt::Key_Down:
        emit this->downKeyPressed();
        break;

    case Qt::Key_Left:
        emit this->leftKeyPressed();
        break;

    case Qt::Key_F1:
        emit this->f1KeyPressed();
        break;

    case Qt::Key_F2:
        emit this->f2KeyPressed();
        break;

    case Qt::Key_F3:
        emit this->f3KeyPressed();
        break;

    case Qt::Key_F4:
        emit this->f4KeyPressed();
        break;

    case Qt::Key_F5:
        emit this->f5KeyPressed();
        break;

    case Qt::Key_PageUp:
        emit this->pgUpKeyPressed();
        break;

    case Qt::Key_PageDown:
        emit this->pgDownKeyPressed();
        break;

    case Qt::Key_Home:
        emit this->homeKeyPressed();
        break;

    case Qt::Key_End:
        emit this->endKeyPressed();
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        emit this->enterKeyPressed();
        break;

    default:
        return QObject::eventFilter(obj, event);
    }

    return true;
}

QJomeWindow::QJomeWindow(const EmojiDb& emojiDb,
                         const bool isEternal,
                         const EmojiChosenFunc& emojiChosenFunc) :
    QDialog {},
    _emojiDb {&emojiDb},
    _emojiChosenFunc {emojiChosenFunc},
    _isEternal {isEternal}
{
    this->setWindowTitle("jome");
    this->setFixedSize(800, 600);
    this->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    this->_setMainStyleSheet();
    this->_buildUi();
}

void QJomeWindow::_setMainStyleSheet()
{
    static const char * const styleSheet =
        "* {"
        "  font-family: 'Hack', 'DejaVu Sans Mono', monospace;"
        "  font-size: 12px;"
        "  border: none;"
        "}"
        "QDialog {"
        "  background-color: #333;"
        "}"
        "QLineEdit {"
        "  background-color: rgba(0, 0, 0, 0.2);"
        "  color: #f0f0f0;"
        "  font-weight: bold;"
        "  font-size: 14px;"
        "  border-bottom: 2px solid #ff3366;"
        "  padding: 4px;"
        "}"
        "QListWidget {"
        "  background-color: transparent;"
        "  color: #e0e0e0;"
        "}"
        "QScrollBar:vertical {"
        "  border: none;"
        "  background-color: #666;"
        "  width: 8px;"
        "  margin: 0;"
        "}"
        "QScrollBar::handle:vertical {"
        "  border: none;"
        "  background-color: #999;"
        "  min-height: 16px;"
        "}"
        "QScrollBar::add-line:vertical,"
        "QScrollBar::sub-line:vertical {"
        "  height: 0;"
        "}"
        "QLabel {"
        "  color: #ff3366;"
        "}";

    this->setStyleSheet(styleSheet);
}

QListWidget *QJomeWindow::_createCatListWidget()
{
    auto listWidget = new QListWidget;

    for (const auto& cat : _emojiDb->cats()) {
        listWidget->addItem(new QCatListWidgetItem {*cat});
    }

    listWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    listWidget->setFixedWidth(220);
    QObject::connect(listWidget, SIGNAL(itemSelectionChanged()),
                     this, SLOT(_catListItemSelectionChanged()));
    QObject::connect(listWidget, SIGNAL(itemClicked(QListWidgetItem *)),
                     this, SLOT(_catListItemClicked(QListWidgetItem *)));
    return listWidget;
}

void QJomeWindow::_buildUi()
{
    _wSearchBox = new QLineEdit;
    QObject::connect(_wSearchBox, SIGNAL(textChanged(const QString&)),
                     this, SLOT(_searchTextChanged(const QString&)));

    auto eventFilter = new QSearchBoxEventFilter {this};

    _wSearchBox->installEventFilter(eventFilter);
    QObject::connect(eventFilter, SIGNAL(upKeyPressed()),
                     this, SLOT(_searchBoxUpKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(rightKeyPressed()),
                     this, SLOT(_searchBoxRightKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(downKeyPressed()),
                     this, SLOT(_searchBoxDownKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(leftKeyPressed()),
                     this, SLOT(_searchBoxLeftKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(enterKeyPressed()),
                     this, SLOT(_searchBoxEnterKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(f1KeyPressed()),
                     this, SLOT(_searchBoxF1KeyPressed()));
    QObject::connect(eventFilter, SIGNAL(f2KeyPressed()),
                     this, SLOT(_searchBoxF2KeyPressed()));
    QObject::connect(eventFilter, SIGNAL(f3KeyPressed()),
                     this, SLOT(_searchBoxF3KeyPressed()));
    QObject::connect(eventFilter, SIGNAL(f4KeyPressed()),
                     this, SLOT(_searchBoxF4KeyPressed()));
    QObject::connect(eventFilter, SIGNAL(f5KeyPressed()),
                     this, SLOT(_searchBoxF5KeyPressed()));
    QObject::connect(eventFilter, SIGNAL(pgUpKeyPressed()),
                     this, SLOT(_searchBoxPgUpKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(pgDownKeyPressed()),
                     this, SLOT(_searchBoxPgDownKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(homeKeyPressed()),
                     this, SLOT(_searchBoxHomeKeyPressed()));
    QObject::connect(eventFilter, SIGNAL(endKeyPressed()),
                     this, SLOT(_searchBoxEndKeyPressed()));

    auto mainVbox = new QVBoxLayout;

    mainVbox->setMargin(8);
    mainVbox->setSpacing(8);
    mainVbox->addWidget(_wSearchBox);
    _wEmojis = new QEmojisWidget {nullptr, *_emojiDb};
    QObject::connect(_wEmojis, SIGNAL(selectionChanged(const Emoji *)),
                     this, SLOT(_emojiSelectionChanged(const Emoji *)));
    QObject::connect(_wEmojis, SIGNAL(emojiClicked(const Emoji&)),
                     this, SLOT(_emojiClicked(const Emoji&)));
    QObject::connect(_wEmojis, SIGNAL(emojiHoverEntered(const Emoji&)),
                     this, SLOT(_emojiHoverEntered(const Emoji&)));
    QObject::connect(_wEmojis, SIGNAL(emojiHoverLeaved(const Emoji&)),
                     this, SLOT(_emojiHoverLeaved(const Emoji&)));
    _wCatList = this->_createCatListWidget();
    this->_wCatList->setCurrentRow(0);

    auto emojisHbox = new QHBoxLayout;

    emojisHbox->setMargin(0);
    emojisHbox->setSpacing(8);
    emojisHbox->addWidget(_wEmojis);
    emojisHbox->addWidget(_wCatList);
    mainVbox->addLayout(emojisHbox);
    this->setLayout(mainVbox);

    _wInfoLabel = new QLabel {""};
    mainVbox->addWidget(_wInfoLabel);
}

void QJomeWindow::reject()
{
    if (_isEternal) {
        this->hide();
    } else {
        QDialog::reject();
    }
}

void QJomeWindow::accept()
{
    if (_isEternal) {
        this->hide();
    } else {
        QDialog::accept();
    }
}

void QJomeWindow::showEvent(QShowEvent * const event)
{
    QDialog::showEvent(event);

    if (!_emojisWidgetBuilt) {
        _wEmojis->rebuild();
        _emojisWidgetBuilt = true;
    }

    _wEmojis->showAllEmojis();
    _wSearchBox->setFocus();
}

void QJomeWindow::closeEvent(QCloseEvent * const event)
{
    if (_isEternal) {
        event->ignore();
        this->hide();
    } else {
        QDialog::closeEvent(event);
    }
}

void QJomeWindow::_findEmojis(const std::string& cat,
                              const std::string& needles)
{
    std::vector<const Emoji *> results;

    _emojiDb->findEmojis(cat, needles, results);
    _wEmojis->showFindResults(results);
}

void QJomeWindow::_searchTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        _wEmojis->showAllEmojis();
        return;
    }

    std::vector<std::string> parts;
    const std::string textStr {text.toUtf8().constData()};

    boost::split(parts, textStr, boost::is_any_of("/"));

    if (parts.size() != 2) {
        this->_findEmojis("", textStr);
        return;
    }

    this->_findEmojis(parts[0], parts[1]);
}

void QJomeWindow::_catListItemSelectionChanged()
{
    if (!_wEmojis->showingAllEmojis()) {
        return;
    }

    auto selectedItems = _wCatList->selectedItems();

    if (selectedItems.isEmpty()) {
        return;
    }

    const auto& item = static_cast<const QCatListWidgetItem&>(*selectedItems[0]);

    _wEmojis->scrollToCat(item.cat());
}

void QJomeWindow::_catListItemClicked(QListWidgetItem * const item)
{
    this->_catListItemSelectionChanged();
}

void QJomeWindow::_searchBoxUpKeyPressed()
{
    _wEmojis->selectPreviousRow();
}

void QJomeWindow::_searchBoxRightKeyPressed()
{
    _wEmojis->selectNext();
}

void QJomeWindow::_searchBoxDownKeyPressed()
{
    _wEmojis->selectNextRow();
}

void QJomeWindow::_searchBoxLeftKeyPressed()
{
    _wEmojis->selectPrevious();
}

void QJomeWindow::_searchBoxPgUpKeyPressed()
{
    _wEmojis->selectPreviousRow(10);
}

void QJomeWindow::_searchBoxPgDownKeyPressed()
{
    _wEmojis->selectNextRow(10);
}

void QJomeWindow::_searchBoxHomeKeyPressed()
{
    _wEmojis->selectFirst();
}

void QJomeWindow::_searchBoxEndKeyPressed()
{
    _wEmojis->selectLast();
}

void QJomeWindow::_searchBoxEnterKeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::NONE);
}

void QJomeWindow::_searchBoxF1KeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::LIGHT);
}

void QJomeWindow::_searchBoxF2KeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::MEDIUM_LIGHT);
}

void QJomeWindow::_searchBoxF3KeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::MEDIUM);
}

void QJomeWindow::_searchBoxF4KeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::MEDIUM_DARK);
}

void QJomeWindow::_searchBoxF5KeyPressed()
{
    this->_acceptSelectedEmoji(Emoji::SkinTone::DARK);
}

void QJomeWindow::_emojiSelectionChanged(const Emoji * const emoji)
{
    _selectedEmoji = emoji;
    this->_updateInfoLabel(emoji);
}

void QJomeWindow::_emojiClicked(const Emoji& emoji)
{
    this->_acceptEmoji(emoji, Emoji::SkinTone::NONE);
}

void QJomeWindow::_emojiHoverEntered(const Emoji& emoji)
{
    this->_updateInfoLabel(&emoji);
}

void QJomeWindow::_emojiHoverLeaved(const Emoji& emoji)
{
    this->_updateInfoLabel(_selectedEmoji);
}

void QJomeWindow::_acceptSelectedEmoji(const Emoji::SkinTone skinTone)
{
    if (_selectedEmoji) {
        this->_acceptEmoji(*_selectedEmoji, skinTone);
    }
}

void QJomeWindow::_acceptEmoji(const Emoji& emoji,
                               const Emoji::SkinTone skinTone)
{
    _emojiChosenFunc(emoji, skinTone);
    this->accept();
}

void QJomeWindow::_updateInfoLabel(const Emoji * const emoji)
{
    QString text;

    if (emoji) {
        text += "<b>";
        text += emoji->name().c_str();
        text += "</b> <span style=\"color: #999\">(";

        for (const auto codepoint : emoji->codepoints()) {
            text += QString::number(codepoint, 16) + ", ";
        }

        text.truncate(text.size() - 2);
        text += ")</span>";
    }

    _wInfoLabel->setText(text);
}

} // namespace jome
