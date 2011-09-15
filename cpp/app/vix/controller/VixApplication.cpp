#include "controller/VixApplication.hpp"
#include "controller/Commander.hpp"
#define GUBG_MODULE "VixApplication"
#define LOG_LEVEL Debug
#include "logging/Log.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <iostream>
#include "boost/filesystem.hpp"
using namespace vix;
using namespace std;

VixApplication::VixApplication(int argc, char **argv):
    QApplication(argc, argv),
    commander_(selectionModels_)
{
    LOG_S_(Debug, ctor);
    QWidget *centralWidget = new QWidget(&mainWindow_); 
    QVBoxLayout *vbox = new QVBoxLayout(centralWidget);
    mainWindow_.setCentralWidget(centralWidget);
    selectionView_.setModel(&stringListModel_);
    tabBar_.setFocusPolicy(Qt::NoFocus);
    vbox->addWidget(&tabBar_);
    vbox->addWidget(&pathLabel_);
    {
        QHBoxLayout *hbox = new QHBoxLayout(centralWidget);
        hbox->addWidget(&selectionView_);
        textEdit_.setReadOnly(true);
        textDocument_.setHtml(QString("<br/><br/><center><font color=\"#ff0000\">ViX</font></center><br/><center>The vi-inspired file explorer</center><br/>Geert Fannes"));
        textEdit_.setDocument(&textDocument_);
        hbox->addWidget(&textEdit_);
        vbox->addLayout(hbox);
    }
    vbox->addWidget(&filter_);
    vbox->addWidget(&content_);
    vbox->addWidget(&command_);
    //mainWindow_.resize(1024, 640);
    mainWindow_.showMaximized();
    mainWindow_.setVisible(true);

    LOG_M_(Debug, "Connecting signals");
    connect(&selectionView_, SIGNAL(readableKeyPressed(QChar)), this, SLOT(process4Commandline(QChar)));
    connect(&selectionView_, SIGNAL(keycodePressed(int, int)), this, SLOT(process4Commandline(int, int)));
    connect(&tabBar_, SIGNAL(currentChanged(int)), this, SLOT(changeCurrent(int)));
    connect(this, SIGNAL(updateSelectionSignal(vix::model::Selection*)), this, SLOT(updateSelectionSlot(vix::model::Selection*)));
    selectionModelsUpdatedConnection_ = selectionModels_.connect(boost::bind(&VixApplication::updateSelection_, this, _1));
    commanderUpdatedConnection_ = commander_.connect(boost::bind(&VixApplication::updateCommander_, this, _1, _2));

#ifdef __linux
    const string path("/home/gfannes");
#else
    const string path("h:/");
#endif
    LOG_M_(Debug, "Adding the first selection (" << path << ")");
    selectionModels_.addSelection(path);
}

void VixApplication::process4Commandline(QChar ch)
{
    LOG_S_(Debug, process4Commandline_QChar);
    int i = (int)ch.toAscii();
    LOG_M_(Debug, "Process key " << i);
    {
        switch (i)
        {
            case 27://Escape
                commander_.clear();
                break;
            case 13://Enter
                commander_.activate(Commander::Key::Enter);
                break;
            default:
                commander_.add(ch.toAscii());
                break;
        }
    }
}
enum class KeyCode: int
{
    Left = 0x1000012,
    Right = 0x1000014,
    Up = 0x1000013,
    Down = 0x1000015,
    PgUp = 0x1000016,
    PgDown = 0x1000017
};
enum class Modifiers: int
{
    None = 0x00000000,
    Shift = 0x02000000,
    Control = 0x04000000,
    Alt = 0x08000000
};
void VixApplication::process4Commandline(int keycode, int modifiers)
{
    LOG_SM_(Debug, process4Commandline_keycode, "Process keycode " << hex << keycode << ", modifiers " << modifiers << dec);
    if (selectionModels_.empty())
        return;

    model::Selection *selection = selectionModels_.current();
    {
        switch (keycode)
        {
            case (int)KeyCode::Up:
                selection->move(model::Selection::Direction::Up);
                return;
                break;
            case (int)KeyCode::Down:
                selection->move(model::Selection::Direction::Down);
                return;
                break;
            case (int)KeyCode::Left:
                {
                    auto parent = selection->path()->location();
                    if (parent)
                    {
                        commander_.clear();
                        //boost::signals2::shared_connection_block block(selectionModelsUpdatedConnection_);
                        selection->setPath(parent);
                    }
                }
                break;
            case (int)KeyCode::Right:
                {
                    //boost::signals2::shared_connection_block block(selectionModelsUpdatedConnection_);
                    commander_.activate(Commander::Key::Arrow);
                }
                break;
            case (int)KeyCode::PgUp:
                {
                    if ((int)Modifiers::Control == modifiers)
                        commander_.changeTab(selectionModels_.currentIX()-1);
                    else
                    {
                    }
                }
                break;
            case (int)KeyCode::PgDown:
                {
                    if ((int)Modifiers::Control == modifiers)
                        commander_.changeTab(selectionModels_.currentIX()+1);
                    else
                    {
                    }
                }
                break;
            default:
                LOG_M_(Debug, "Doing nothing");
                return;
                break;
        }
    }
}

void VixApplication::changeCurrent(int ix)
{
    LOG_S_(Debug, changeCurrent);
    commander_.changeTab(ix);
}

void VixApplication::setSelected(const QModelIndex &current, const QModelIndex &prev)
{
    LOG_S_(Debug, setSelected);
    auto selected = stringListModel_.data(current, Qt::DisplayRole).toString().toStdString();
    LOG_M_(Debug, "setSelected to " << selected);
    if (selectionModels_.empty())
        return;
    selectionModels_.current()->setSelected(selected);
}

void VixApplication::updateSelection_(vix::model::Selection *selectionModel)
{
    LOG_SM_(Debug, updateSelection_, "selectionModel: " << selectionModel);
    emit updateSelectionSignal(selectionModel);
}
void VixApplication::updateSelectionSlot(vix::model::Selection *selectionModel)
{
    LOG_SM_(Debug, updateSelectionSlot, "selectionModel: " << selectionModel);
    pathLabel_.setText(selectionModel->path()->path().c_str());

    //Expand or shrink the tab bar if necessary and populate it
    {
        LOG_SM_(Debug, tabbar, "Populating the tab bar");
        auto selections = selectionModel->selections();
        auto prev = tabBar_.blockSignals(true);
        while (tabBar_.count() != selections.size())
        {
            if (tabBar_.count() < selections.size())
                tabBar_.addTab("");
            else
                tabBar_.removeTab(0);
        }
        for (auto selection = selections.begin(); selection != selections.end(); ++selection)
        {
            auto ix = selection-selections.begin();
            std::ostringstream oss;
            oss << ix << " " << (*selection)->path()->name();
            tabBar_.setTabText(ix, oss.str().c_str());
            if (selectionModels_.current() == *selection)
                tabBar_.setCurrentIndex(ix);
        }
        tabBar_.blockSignals(prev);
    }

    //Show the files
    {
        LOG_SM_(Debug, files, "Showing the files");
        vix::model::Files files;
        int selectedIX;
        selectionModel->getFiles(files, selectedIX);
        QStringList stringList;
        for (auto it = files.begin(); it != files.end(); ++it)
        {
            auto &file = *it;
            vix::model::File::Unlock unlockedFile(file);
            if (unlockedFile->isDirectory())
                stringList << (unlockedFile->name() + "/").c_str();
            else
                stringList << unlockedFile->name().c_str();
        }
        stringListModel_.setStringList(stringList);
        LOG_M_(Debug, "selectedIX: " << selectedIX);
        auto ix = stringListModel_.index(selectedIX);
        selectionView_.selectionModel()->select(ix, QItemSelectionModel::Select);
        selectionView_.scrollTo(ix, QAbstractItemView::EnsureVisible);
    }

    //Show the content
    {
        LOG_SM_(Debug, content, "Showing the content");
        string contentAsHtml;
        if (selectionModel->getContent(contentAsHtml, model::Format::Html))
            textDocument_.setHtml(QString::fromStdString(contentAsHtml));
        else
        {
            LOG_M_(Debug, "Could not get the content");
        }
    }
}

void VixApplication::updateCommander_(int which, const string *str)
{
    LOG_SM_(Debug, updateCommander_, "which: " << which);
    QString qstr(str->c_str());
    switch (which)
    {
        case 0:
            filter_.setText(qstr);
            break;
        case 1:
            content_.setText(qstr);
            break;
        case 2:
            command_.setText(qstr);
            break;
    }
    auto cm = commander_.currentMode();
    filter_.setEnabled(0 == cm);
    content_.setEnabled(1 == cm);
    command_.setEnabled(2 == cm);
}
