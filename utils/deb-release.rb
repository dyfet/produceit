#!/usr/bin/env ruby
# Copyright (C) 2017 Tycho Softworks
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.

['optparse', 'fileutils'].each {|mod| require mod}

banner = 'Usage: deb-release [options] archive-directory'
verbose = false

OptionParser.new do |opts|
  opts.banner = banner

  opts.on_tail('-h', '--help', 'Show this message') do
    puts opts
    exit
  end

  opts.on('-v', '--[no-]verbose', 'Offer verbose output') do |v|
    verbose = v
  end
end.parse!

abort(banner) if ARGV.size != 1
path = ARGV[0]
release = "#{path}/Release"
abort("deb-release: #{path}: not a directory") if !File.directory?(path)
abort("deb-release: #{path}: no release file") if !File.exists?(release)

relinfo = {:Archive => 'stretch'}

File.open(release, 'r') do |fp; line, key, value|
  while(line = fp.gets)
      next unless line =~ /^.*[:]/
      key, value = line.split(/\:/).each {|s| s.strip!}
      next if value == ""
      relinfo[key.to_sym] = value
  end
end
