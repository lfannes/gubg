module vix.View;

import vix.Model;
import vix.Exit;

import gubg.graphics.Canvas;
import gubg.graphics.IMUI;
import gubg.Layout;
import gubg.Format;

import std.algorithm;
import std.regexp;

import std.stdio;

class View
{
    this(Model model, SDLCanvas canvas)
    {
        model_ = model;
        canvas_ = canvas;
        widgets_ = new Widgets;
        tabIX_ = 0;
    }

    void process()
    {
        auto box = new Box(TwoPoint(0, 0, canvas_.width, canvas_.height));
        box.split([0.03, 0.05, 0.89, 0.03], Direction.TopDown);
        auto tabs = box[0];
        auto folderBar = box[1];
        auto center = box[2];
        auto commandBar = box[3];
        //The tabs
        {
            string formatPathForTab_(Tab tab)
            {
                if (!tab.getContentPattern.empty)
                    return "(" ~ tab.getContentPattern ~ ")";
                version (Posix)
                {
                    return std.path.basename(tab.getPath);
                }
                version (Win32)
                {
                    auto re = RegExp("([A-Z]+_[A-Za-z\\._\\d]+)");
                    auto path = tab.getPath;
                    if (re.test(path))
                        return re[0];
                    return std.path.basename(path);
                }
            }
            tabs.split(model_.getTabs().length, Direction.LeftToRight);
            foreach (uint ix, ref sb; tabs)
            {
                auto tab = model_.getTabs()[ix];
                //We use _both_ the total number of tabs and the tabIX at hand as extra
                auto w = widgets_.get((model_.getTabs().length << 16) + ix);
                string label = Format.immediate("%s - %s", ix+1, formatPathForTab_(tab));
                switch (w.process)
                {
                    case WidgetState.Empty:
                        w.set(new Button(sb.area, label, Alignment.Left, canvas_));
                        break;
                    case WidgetState.Activated:
                        setCurrentTab(ix);
                        break;
                    default:
                        auto button = w.get!(Button).setLabel(label);
                        if (tab == currentTab)
                            button.setFillColor(Color.coolGreen);
                        else
                            button.resetFillColor();
                        break;
                }
            }
        }
        //The folder bar
        {
            auto w = widgets_.get();
            updateCurrentPath_();
            switch (w.process)
            {
                case WidgetState.Empty:
                    w.set(new Button(folderBar.area, currentPath_, Alignment.Left, canvas_));
                    break;
                case WidgetState.Activated:
                    //What to do here?
                    break;
                default:
                    w.get!(Button).setLabel(currentPath_);
                    break;
            }
        }
        center.split([0.02, 0.97, 0.01], Direction.LeftToRight);
        auto back = center[0];
        auto buttons = center[1];
        auto scroller = center[2];
        //The back button
        {
            auto w = widgets_.get();
            switch (w.process)
            {
                case WidgetState.Empty:
                    w.set(new Button(back.area, "", Alignment.Left, canvas_));
                    break;
                case WidgetState.Activated:
                    currentTab.moveToRoot;
                    return;
                    break;
                default:
                    break;
            }
        }
        //The file and folder buttons with scrollbar
        {
            //Get all the childs and the focusIX
            uint focusIX;
            Tab.DisplayMode displayMode;
            auto allChilds = currentTab.getChilds(focusIX, displayMode);
            //Check that topIX_ is in range
            if (topIX_ < 0)
            {
                reportError(Format.immediate("topIX_: %s is negative", topIX_));
                topIX_ = 0;
            }
            else if (topIX_ >= allChilds.length && !allChilds.empty)
            {
                reportError(Format.immediate("topIX_: %s is negative", topIX_));
                topIX_ = allChilds.length-1;
            }

            const MaxNrEntries = 40;
            //Shift topIX_ to make sure focusIX will be shown
            if (focusIX < topIX_)
                topIX_ = focusIX;
            else if (focusIX >= topIX_+MaxNrEntries)
                topIX_ = focusIX-MaxNrEntries+1;
            //Show as much entries as possible
            if (allChilds.length <= MaxNrEntries)
                topIX_ = 0;
            //Scrollbar
            {
                auto w = widgets_.get();
                switch (w.process)
                {
                    case WidgetState.Empty:
                        w.set(new Scroller(scroller.area, buttons.area, canvas_));
                        break;
                    case WidgetState.ScrollDown:
                        if (allChilds.length > MaxNrEntries)
                            topIX_ = min(topIX_+10, allChilds.length-MaxNrEntries);
                        if (focusIX < topIX_)
                        {
                            focusIX = topIX_;
                            currentTab.setFocus(focusIX);
                        }
                        break;
                    case WidgetState.ScrollUp:
                        topIX_ = max(topIX_-10, 0);
                        if (focusIX >= topIX_+MaxNrEntries)
                        {
                            focusIX = topIX_+MaxNrEntries-1;
                            currentTab.setFocus(focusIX);
                        }
                        break;
                    default:
                        auto sb = w.get!(Scroller);
                        sb.setRange(allChilds.length);
                        sb.setBar([cast(real)topIX_, min(topIX_+MaxNrEntries, allChilds.length)]);
                        break;
                }
            }
            //Draw all the folder and file entries from a subset of allChilds
            auto childs = allChilds[topIX_ .. $];
            auto relativeFocusIX = focusIX - topIX_;
            buttons.split(MaxNrEntries, Direction.TopDown);
            foreach (uint ix, ref sb; buttons)
            {
                if (ix >= childs.length)
                    break;
                FSTree child = childs[ix];
                string label;
                switch (displayMode)
                {
                    case Tab.DisplayMode.Mixed:
                        label = child.name ~ (cast(Folder)child is null ? "" : std.path.sep);
                        break;
                    case Tab.DisplayMode.Files:
                        label = child.path;
                        string baseDir = currentPath_ ~ std.path.sep;
                        if (label.length > baseDir.length && label[0 .. baseDir.length] == baseDir)
                            label = label[baseDir.length .. $];
                        break;
                }
                auto w = widgets_.get(ix);
                switch (w.process)
                {
                    case WidgetState.Empty:
                        w.set(new Button(sb.area, label, Alignment.Left, canvas_));
                        break;
                    case WidgetState.Activated:
                        currentTab.activate(child, Tab.ActivationType.Native);
                        break;
                    default:
                        auto button = w.get!(Button).setLabel(label);
                        if (ix == relativeFocusIX)
                            button.setFillColor(Color.coolGreen);
                        else
                            button.resetFillColor();
                        break;
                }
            }
        }
        //The command bar
        {
            auto w = widgets_.get();
            switch (w.process)
            {
                case WidgetState.Empty:
                    w.set(new Button(commandBar.area, model_.getCommand, Alignment.Left, canvas_));
                    break;
                case WidgetState.Activated:
                    //What to do here?
                    break;
                default:
                    w.get!(Button).setLabel(model_.getCommand);
                    break;
            }
        }
    }

    Tab currentTab(){return model_.getTabs()[tabIX_];}
    int currentTabIX(){return tabIX_;}
    bool setCurrentTab(int tabIX)
    {
        if (0 == model_.getTabs().length)
            return false;
        tabIX_ = tabIX;
        if (tabIX_ < 0)
            tabIX_ = 0;
        else if (tabIX_ >= model_.getTabs().length)
            tabIX_ = model_.getTabs().length-1;
        return true;
    }

    private:
    Model model_;
    Widgets widgets_;
    SDLCanvas canvas_;

    void updateCurrentPath_()
    {
        if (currentPath_ != currentTab.getPath)
        {
            currentPath_ = currentTab.getPath;
            topIX_ = 0;
        }
    }
    int topIX_;

    int tabIX_;
    string currentPath_;
}
