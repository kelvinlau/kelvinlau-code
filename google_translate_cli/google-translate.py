#!/usr/bin/env python

import os
import re
import readline
import subprocess
import sys
import urllib

def is_english(text):
  return all(ord(c) in range(128) for c in text)

def call(cmd):
  subprocess.check_call(cmd, shell = True)

def translate(text, src = None, dst = None):
  if not src:
    src = 'auto'
  if not dst:
    dst = 'en' if not is_english(text) else 'zh-tw'
  call(r'wget -U "Mozilla/5.0" -qO - "http://translate.google.com/translate_a/t?client=t&text=%s&sl=%s&tl=%s" | sed "s/\[\[\[\"//" | cut -d \" -f 1' % (urllib.quote(text), src, dst))

LANGUAGES = ['auto', 'zh-tw', 'zh-cn', 'en', 'ja']

def main():
  argv = sys.argv
  if len(argv) <= 1:
    program = os.path.basename(argv[0])
    print 'Usage: %s <phases...> [[<src>]:<dst>]' % program
    print '       %s --shell|-s [[<src>]:<dst>]' % program
    print '  <src>, <dst> are in %s' % repr(LANGUAGES)
    print "  <src> defaults to 'auto'"
    print "  <dst> defaults to 'zh-tw' or 'en', depending on whether input phases are in english"
    print 'Examples:'
    print '  %s I love you' % program
    print '  %s I love you :zh-tw' % program
    print '  %s I love you en:zh-tw' % program
    print '  %s -s' % program
    sys.exit(1)

  src = None
  dst = None
  for i in range(1, len(argv)):
    arg = argv[i]
    if arg.count(':') == 1:
      a, b = arg.split(':')
      if (a == '' or a in LANGUAGES) and b in LANGUAGES:
        src = a
        dst = b
        del argv[i]
        break
  shell_mode = '-s' in argv or '--shell' in argv

  if shell_mode:
    while True:
      try:
        text = raw_input('>> ')
        translate(text, None, None)
      except EOFError:
        print 'Bye'
        break
  else:
    text = ' '.join(argv[1:])
    translate(text, src, dst)

if __name__ == '__main__':
  main()
