#!/usr/bin/python

import re
import os
import sys
import time
import shutil
from optparse import OptionParser

parser = OptionParser('Usage: %prog [options] <dir>')
parser.add_option('-n', action='store_true', dest='dryrun', default=False, help='dryrun')

options, args = parser.parse_args()

if len(args) != 1:
    print 'Usage: cat-date.py <dir>'
    sys.exit(1)

def FileDate(filename):
    stat = os.stat(filename)
    return time.strftime('%Y/%m', time.localtime(stat.st_mtime))

def FileSize(filename):
    stat = os.stat(filename)
    return stat.st_size

def AllFiles(top):
    if os.path.isfile(top):
        yield top
        return
    for (dirpath, dirnames, filenames) in os.walk(top):
        for fn in filenames:
            yield os.path.join(dirpath, fn)

def SimpleName(x):
    m = re.search(r'(.*[^a-zA-Z])?([a-zA-Z]{2,4})[^0-9]*(\d{2,4})\.(wmv|mkv|avi|mp4)', x)
    if m:
        return '%s-%s.%s' % (m.group(2).lower(), m.group(3), m.group(4))
    else:
        return os.path.basename(x)

dirname = args[0]
os.chdir(dirname)
for x in [x for x in os.listdir('.') if not re.match(r'\d{4}$', x)]:
    date = FileDate(x)
    fns = list(AllFiles(x))
    videos = list(y for y in fns if re.search(r'\.(wmv|mkv|avi|mp4)$', y))
    videos = [y for y in videos if FileSize(y) > 100E6]
    if any(re.search(r'\.part$', y) for y in fns):
        print 'Unfinished:', x
        continue
    if any(FileSize(y) > 100E6 and y not in videos for y in fns):
        print 'Contains unknown big file:', x
        continue
    if not options.dryrun and not os.path.isdir(date):
        os.makedirs(date)
    should_remove = os.path.isdir(x)
    for y in videos:
        dest = os.path.join(date, SimpleName(y))
        print 'Moving %s to %s' % (y, dest)
        if not os.path.isfile(dest):
            if not options.dryrun:
                os.rename(y, dest)
        else:
            print 'Already exists:', dest
            should_remove = False
    if should_remove:
        if not options.dryrun:
            shutil.rmtree(x)
        print 'Removing %s' % x
