#!/usr/bin/env ruby
# Copyright (C) 2017 Tycho Softworks
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.

['optparse', 'fileutils'].each {|mod| require mod}

EXTS = ['.dsc', '.deb', '.changes']

banner = 'Usage: dsc-remove [options] debian-files...'
verbose = false
force = false

# parse main arguments
OptionParser.new do |opts|
  opts.banner = banner

  opts.on('-f', '--[no-]force', 'force overwrite') do |f|
    force = f
  end

  opts.on_tail('-h', '--help', 'Show this message') do
    puts opts
    exit
  end

  opts.on('-v', '--[no-]verbose', 'Offer verbose output') do |v|
    verbose = v
  end
end.parse!

abort(banner) if ARGV.size < 1

# process file list
ARGV.each() do |file; ext, basename, origin|
  next if force && !File.file?(file)

  ext = File.extname(file)
  basename = File.basename(file)

  abort("*** dsc-archive: #{basename}: not found") unless File.file?(file)
  abort("*** dsc-archive: #{basename}: not debian source control") unless EXTS.include?(ext)
    
  origin = File.dirname(file)
  print "Removing... #{basename}...\n" if verbose === true
  File.open(file, 'r') do |dsc; line, entry|
    while(line = dsc.gets)
      next unless line=~/^Files/...line=~/^$/
      next if line =~/^$/ || line.strip! =~ /^Files/
      entry = line.split(' ')[-1].prepend(origin + '/')
      p entry
      next unless File.file?(entry)
      print "Removing #{File.basename(entry)}...\n" if verbose === true
      FileUtils.remove_file(entry, force)
    end
  end unless ext === '.deb'
  FileUtils.remove_file(file, force)
end
