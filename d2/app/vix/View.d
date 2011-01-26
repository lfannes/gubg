module vix.View;

import vix.Model;

import gubg.graphics.Canvas;
import gubg.graphics.IMUI;
import gubg.Layout;

import std.algorithm;

import std.stdio;

class View
{
    this(Model model, SDLCanvas canvas)
    {
        model_ = model;
        canvas_ = canvas;
        widgets_ = new Widgets;
        topIX_ = 0;
    }

    void process()
    {
        auto box = new Box(TwoPoint(0, 0, canvas_.width, canvas_.height));
        box.split([0.05, 0.92, 0.03], Direction.TopDown);
        auto folderBar = box[0];
        auto center = box[1];
        auto commandBar = box[2];
        //The folder bar
        {
            auto w = widgets_.get();
            switch (w.process)
            {
                case WidgetState.Empty:
                    w.set(new Button(folderBar.area, model_.getCurrentPath, Alignment.Left, canvas_));
                    break;
                case WidgetState.Activated:
                    //What to do here?
                    break;
                default:
                    w.get!(Button).setLabel(model_.getCurrentPath);
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
                    model_.moveCurrentToRoot;
                    topIX_ = 0;
                    break;
                default:
                    break;
            }
        }
        //The file and folder buttons with scrollbar
        {
            //Get all the childs
            auto allChilds = model_.getCurrentChilds;
            {
                //Sort the childs using localCmp as criterion
                bool localCmp(FSTree lhs, FSTree rhs)
                {
                    //First Folders, then Files
                    if (cast(Folder)lhs && cast(gubg.FSTree.File)rhs)
                        return true;
                    if (cast(gubg.FSTree.File)lhs && cast(Folder)rhs)
                        return false;
                    //If the type results in a tie, sort alphabetically
                    return lhs.name < rhs.name;
                }
                sort!(localCmp)(allChilds);
            }
            const MaxNrEntries = 40;
            //Scrollbar
            {
                auto w = widgets_.get();
                switch (w.process)
                {
                    case WidgetState.Empty:
                        w.set(new Scroller(scroller.area, buttons.area, canvas_));
                        break;
                    case WidgetState.ScrollDown:
                        topIX_ = min(topIX_+10, allChilds.length-1);
                        break;
                    case WidgetState.ScrollUp:
                        topIX_ = max(topIX_-10, 0);
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
            buttons.split(MaxNrEntries, Direction.TopDown);
            foreach (uint ix, ref sb; buttons)
            {
                if (ix >= childs.length)
                    break;
                FSTree child = childs[ix];
                string label = child.name;
                auto w = widgets_.get(ix);
                switch (w.process)
                {
                    case WidgetState.Empty:
                        w.set(new Button(sb.area, label, Alignment.Left, canvas_));
                        break;
                    case WidgetState.Activated:
                        Folder folder = cast(gubg.FSTree.Folder)child;
                        if (folder)
                            model_.setCurrent(folder);
                        else
                        {
                            //Not yet handled
                        }
                        break;
                    default:
                        w.get!(Button).setLabel(label);
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

    private:
    Model model_;
    Widgets widgets_;
    SDLCanvas canvas_;
    int topIX_;
}
