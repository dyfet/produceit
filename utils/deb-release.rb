#!/usr/bin/ruby
# Copyright (C) 2017-2019 David Sugar <tychosoft@gmail.com>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.

['optparse', 'fileutils'].each {|mod| require mod}

META = ['Packages', 'Packages.gz', 'Packages.bz2', 'Sources', 'Sources.gz', 'Sources.bz2'].freeze
banner = 'Usage: deb-release [options] [archive-directory]'
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

abort(banner) if ARGV.size > 1
if !ARGV.empty?
  path = ARGV[0]
else
  path = '.'
end
release = "#{path}/Release"
abort("deb-release: #{path}: not a directory") unless File.directory?(path)
abort("deb-release: #{path}: no release file") unless File.exist?(release)

relinfo = {Archive: 'stretch', Codename: '', Origin: '', Label: '', Architecture: 'all'}

File.open(release, 'r') do |fp; line, key, value|
  while (line = fp.gets)
    next unless line =~ /^.*[:]/

    key, value = line.split(/\:/).each {|s| s.strip!}
    next if value.empty?

    relinfo[key.to_sym] = value
  end
end

if verbose
  p relinfo
end

Dir.chdir(path)
File.open('Release.tmp', 'w') do |tmp|
  tmp << "Archive: #{relinfo[:Archive]}\n" unless relinfo[:Archive].empty?
  tmp << "Codename: #{relinfo[:Codename]}\n" unless relinfo[:Codename].empty?
  tmp << "Origin: #{relinfo[:Origin]}\n" unless relinfo[:Origin].empty?
  tmp << "Label: #{relinfo[:Label]}\n" unless relinfo[:Label].empty?
  tmp << "Architecture: #{relinfo[:Architecture]}\n" unless relinfo[:Architecture].empty?
  tmp << 'Date: ' << `date -R -u`

  tmp << "MD5Sum:\n"
  META.each do |file; sum, size|
    next unless File.exist?(file)

    sum=`md5sum "#{file}"`.split(' ').first
    size=`wc -c "#{file}"`.lstrip
    tmp << " #{sum} #{size}"
  end

  tmp << "SHA1:\n"
  META.each do |file; sum, size|
    next unless File.exist?(file)

    sum=`sha1sum "#{file}"`.split(' ').first
    size=`wc -c "#{file}"`.lstrip
    tmp << " #{sum} #{size}"
  end

  tmp << "SHA256:\n"
  META.each do |file; sum, size|
    next unless File.exist?(file)

    sum=`sha256sum "#{file}"`.split(' ').first
    size=`wc -c "#{file}"`.lstrip
    tmp << " #{sum} #{size}"
  end

  system 'mv', 'Release.tmp', 'Release'
end
