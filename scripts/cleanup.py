#!/usr/bin/python

import random
import re
import os
import sys
from optparse import OptionParser

parser = OptionParser('Usage: %prog [options] <dir>')
parser.add_option('-r', action='store_true', dest='random', default=False, help='random order')
parser.add_option('-t', action='store_true', dest='chrono', default=False, help='reversed chrono order')

options, args = parser.parse_args()

if len(args) != 1:
    parser.error('missing dir')

def IsVideo(f):
    return os.path.isfile(f) and re.match('.*\.(avi|mp4|wmv|mkv)$', f)

dirname = args[0]
os.chdir(dirname)
filenames = [f for f in os.listdir('.') if IsVideo(f)]

if options.random:
    random.shuffle(filenames)

if options.chrono:
    filenames.sort(key=lambda f: os.stat(f).st_mtime, reverse=True)

for filename in filenames:
    print filename
    os.system('smplayer ' + filename)
    while True:
        print 'delete? (y/N/q)',
        cmd = raw_input().rstrip()
        if cmd == 'y':
            os.system('echo ' + filename + ' >> /tmp/cleanup.log')
            os.system('rm ' + filename)
            break
        elif cmd == 'n' or cmd == '':
            break
        elif cmd == 'q':
            sys.exit(0)
