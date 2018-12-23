#!/usr/bin/env ruby
# Copyright (C) 2017 Tycho Softworks
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.

['optparse', 'fileutils'].each {|mod| require mod}

EXTS = ['.dsc', '.deb', '.changes'].freeze

verbose = false
preserve = true
header = 'Copying'
banner = 'Usage: deb-archive [options] [debian-files...] target-directory'
force = false
move = false

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

  opts.on('-m', '--move', 'move rather than copy') do
    header = 'Moving'
    move = true
  end

  opts.on('-np', '--no-preserve', 'Do not preserve file stats') do
    preserve = false
  end

  opts.on('-v', '--[no-]verbose', 'Offer verbose output') do |v|
    verbose = v
  end
end.parse!

# positional arguments and verification
*files, target = *ARGV
fileopts = {force: force}

abort(banner) if ARGV.size < 2
abort("*** deb-archive: #{target}: not a directory") unless File.directory?(target)

# process file list
files.each do |file; ext, basename, origin|
  ext = File.extname(file)
  basename = File.basename(file)
  to_dest = target + '/' + basename

  abort("*** deb-archive: #{basename}: not found") unless File.file?(file)
  abort("*** deb-archive: #{basename}: not debian source control") unless EXTS.include?(ext)
  abort("*** deb-archive: #{basename}: target exists") if File.file?(to_dest) && !force
  origin = File.dirname(file)
  print "#{header} #{basename}...\n" if verbose == true
  FileUtils.copy_file(file, dest, preserve) if move == false
  File.open(file, 'r') do |dsc; line, source, dest|
    while (line = dsc.gets)
      next unless line=~/^Files/...line=~/^$/
      next if line =~/^$/ || line.strip! =~ /^Files/

      source = line.split(' ')[-1].prepend(origin + '/')
      dest = target + '/' + File.basename(source)
      next unless File.file?(source)

      print "#{header} #{File.basename(source)}...\n" if verbose == true
      FileUtils.copy_file(source, dest, preserve) if move == false
      FileUtils.move(source, dest, fileopts) if move == true
    end
  end unless ext === '.deb'
  FileUtils.move(file, dest, fileopts) if move == true
end
