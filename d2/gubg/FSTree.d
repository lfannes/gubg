module gubg.FSTree;

import gubg.Tree;
import gubg.Format;
import std.file;
import std.path;

class FSTree
{
    //Insert the tree functionality
    mixin Tree!(FSTree, Folder, File);

    //Interface
    abstract string toString() const;
    string path() const
    {
        return (parent ? join(parent.path, name) : name);
    }

    //This is the single data member, the name of the folder or file.
    //It is placed at the FSTree level, allowing access from an FSTree reference
    string name;
}

class Folder: FSTree
{
    //Insert the node functionality
    mixin Node!(FSTree);

    this(string path, Folder folder = null)
    {
        name = path;
        parent = folder;
    }
    
    //FSTree interface
    string toString() const
    {
        return Format.immediate("Folder: %s containing %d childs.", name, childs.length);
    }
}

class File: FSTree
{
    //Insert the leaf functionality
    mixin Leaf!(FSTree);

    this(string lname, Folder folder = null)
    {
        name = lname;
        parent = folder;
    }

    //FSTree interface
    string toString() const
    {
        return "File: " ~ name;
    }
}

interface ICreator
{
    Folder createFolder(string path);
    File createFile(string path);
}

FSTree createFSTreeFromPath(string path, ICreator creator)
{
    if (isdir(path))
    {
        auto folder = creator.createFolder(path);
        if (folder)
        {
            foreach (string subpath; dirEntries(path, SpanMode.shallow))
            {
                FSTree tree = createFSTreeFromPath(subpath, creator);
                if (tree)
                {
                    tree.parent = folder;
                    folder.childs ~= tree;
                }
            }
        }
        return folder;
    } else if (isfile(path))
        return creator.createFile(path);

    throw new Exception("Path is neither a directory nor a file.");
    return null;
}

version (FSTree)
{
    import std.stdio;
    import std.path;
    void main()
    {
        class Creator: ICreator
        {
            Folder createFolder(string path)
            {
                return new Folder(path);
            }
            File createFile(string path)
            {
                if ("d" == getExt(path))
                    return new File(path);
                return null;
            }
        }
        Creator creator = new Creator;
        auto tree = createFSTreeFromPath("/home/gfannes/gubg", creator);
        foreach (File el; tree)
        {
            writeln(el.toString);
        }
    }
}
