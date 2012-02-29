require("targets.rb")
require("gubg/breakdown")
require("gubg/target")
require("gubg/options")
require("gubg/filestore")

options = parseOptions(name: "Generic build utility", author: "Geert Fannes", version: "0.1") do |parser, options|
    parser.on("-e", "--executable SOURCE", "Produce executable from SOURCE"){|filename|options[:executable] = filename}
    options[:run] = false
    parser.on("-r", "--run", "Run the produced executables"){options[:run] = true}
    parser.on("-c", "--clean", "Clean the filestore, forcing a complete rebuild"){options[:clean] = true}
end
$verbose = options[:verbose]

$filestore = FileStore.new
if options[:clean]
    puts("Cleaning the filestore \"#{$filestore.base}\"")
    $filestore.clean
end

global = Breakdown::Global.new do |global|
    if options[:executable]
        exe = global.breakdown(Executable.new(options[:executable]))
        if options[:run]
            global.breakdown(Run.new(exe.executable))
        end
    end
end
global.process
