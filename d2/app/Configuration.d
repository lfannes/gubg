module Configuration;

import gubg.JSON;
import std.file;
import std.path;
import std.stdio;

class Configuration
{
    this(string filename, string path = null, bool stepDown = false)
    {
        //Whatever happens, make sure we set the current working directory back to its original setting
        string origPath = getcwd();
        scope(exit) chdir(origPath);

        //Make sure we start with path == current working directory
        if (path)
            chdir(path);
        else
            path = getcwd();

        //Depending on stepDown, we either scan for filename in all parent directories
        //Or, we only look in path
        if (stepDown)
        {
            string prevPath;
            while (prevPath != path)
            {
                chdir(path);
                if (exists(filename))
                {
                    filepath_ = join(path, filename);
                    writefln("I found %s", filepath_);
                    break;
                }
                prevPath = path;
                path = dirname(path);
            }
        } else if (exists(filename))
                filepath_ = join(path, filename);

        //If we found the configuration file, parse it
        if (filepath_)
            parseConfigFile_();
    }
    bool isValid()
    {
        if (!jsonIsParsed_) return false;
        return true;
    }

    bool get(string key, out string value)
    {
        if (!isValid()) return false;
        return get(json_, key, value);
    }
    bool get(string key, out string[] values)
    {
        if (!isValid()) return false;
        foreach (value; lookup(json_, key))
            values ~= value;
        return true;
    }

    private:
    string filepath_;
    JSONValue json_;
    bool jsonIsParsed_;

    void parseConfigFile_()
    {
        if (!filepath_)
            return;

        json_ = parseJSON(cast(char[])read(filepath_));
        jsonIsParsed_ = true;
    }
}

version(UnitTest)
{
    void main()
    {
        auto conf = new Configuration("gb.json", "/home/gfannes/gubg/d2/test/l1/l2");
        string executableName;
        writefln("var executableName %s", conf.get("executableName", executableName));
    }
}
