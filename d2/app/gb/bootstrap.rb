#!/usr/bin/env ruby
require("gubg/utils")

localSources = %w[gb Collection Configuration MetaInfo Commands Exit Runtime]
gubgSources = %w[Tree FSTree JSON Build Format FileCache OnlyOnce OptionParser parsing/D parsing/Cpp]

objectExtension = (%w[Linux Linux64].include?(operatingSystem) ? "o" : "obj")

objectFiles = []
[localSources, gubgSources].zip(["", "../../gubg/"]).each do |sources, dir|
    sources.each do |source|
        objectFiles << "objects/#{source}.#{objectExtension}"
        cmd = "dmd -c -of#{objectFiles.last} #{dir}#{source}.d -I../.."
        puts cmd
        system(cmd)
    end
end

cmd = "dmd -ofgb-bootstrap -L--export-dynamic #{objectFiles.join(' ')}"
puts cmd
system(cmd)
