#!/usr/bin/ruby
# Copyright (C) 2017-2019 David Sugar <tychosoft@gmail.com>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.

['optparse', 'fileutils'].each {|mod| require mod}

EXTS = ['.dsc', '.deb', '.changes'].freeze

banner = 'Usage: deb-remove [options] debian-files...'
verbose = false

# parse main arguments
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

abort(banner) if ARGV.empty?

# process file list
ARGV.each do |file; ext, basename, origin|
  ext = File.extname(file)
  basename = File.basename(file)

  abort("*** deb-remove: #{basename}: not debian source control") unless EXTS.include?(ext)
  exit unless File.file?(file)

  origin = File.dirname(file)
  print "Removing... #{basename}...\n" if verbose == true
  File.open(file, 'r') do |dsc; line, entry|
    while (line = dsc.gets)
      next unless line=~/^Files/...line=~/^$/
      next if line =~ /^$/ || line.strip! =~ /^Files/

      entry = line.split(' ')[-1].prepend(origin + '/')
      next unless File.file?(entry)

      print "Removing #{File.basename(entry)}...\n" if verbose == true
      FileUtils.remove_file(entry)
    end
  end unless ext === '.deb'
  FileUtils.remove_file(file)
end
