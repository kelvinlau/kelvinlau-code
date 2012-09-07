#!/usr/bin/env python

# A script to download music from ting.baidu.com

import sys
import os
import re
import urllib2
from urlparse import urlparse
import simplejson as json
from BeautifulSoup import BeautifulSoup

reload(sys)
sys.setdefaultencoding('utf-8')

class DownloadError(Exception): pass
class NotFoundError(Exception): pass

class MusicInfo(object):
  """Ting music struct."""

  def __init__(self, id, song_name, artist_name):
    self.id = id
    self.song_name = song_name
    self.artist_name = artist_name

  def __repr__(self):
    return u'%s %s - %s' % (self.id, self.artist_name, self.song_name)

class TingDownload(object):
  """Download helper for ting.baidu.com."""

  SEARCH_URL = (u'http://openapi.baidu.com/public/2.0/mp3/info/suggestion?'
                 'format=json&word=%(word)s&callback=window.baidu.sug')
  DOWNLOAD_URL = u'http://ting.baidu.com/song/%s/download'
  TARGET_URL = u'http://ting.baidu.com%s'
  MUSICS_DIR = os.path.expanduser('~/Music')

  def __init__(self, name):
    self.name = name
    if not os.path.exists(self.MUSICS_DIR):
      os.mkdir(self.MUSICS_DIR)

  def Download(self):
    """Main routine."""
    try:
      self.music_info = self.Search()
    except urllib2.URLError, e:
      raise DownloadError(e)
    except NotFoundError, e:
      raise e

    self.target_url = self.FetchMusic()
    self.WriteFile()

  def Search(self):
    """Search for the exact song from keyword."""
    word = urllib2.quote(self.name.encode('utf-8'))
    url = self.SEARCH_URL % {'word': word}
    handler = urllib2.urlopen(url)
    json_text  = handler.read()
    json_result = json.loads(json_text.strip()[17: -2])
    if len(json_result['song']) < 1:
      raise NotFoundError(u"Cannot find song: %s" % self.name)
    elif len(json_result['song']) > 1:
      i = 0
      for song in json_result['song']:
        print '%2d: %10s %10s' % (i, song['songname'], song['artistname'])
        i += 1
      print 'Input index: ',
      i = raw_input()
      if i == '':
        raise NotFoundError
      i = int(i)
      if i < 0 or i >= len(json_result['song']):
        raise NotFoundError
    else:
      i = 0
    entry = json_result['song'][i]
    music = MusicInfo(entry['songid'], entry['songname'], entry['artistname'])
    return music

  def FetchMusic(self):
    """Get the link of music."""
    page = urllib2.urlopen(self.DOWNLOAD_URL %self.music_info.id).read()
    link = BeautifulSoup(page).findAll('a')[-1]
    return self.TARGET_URL % link['href']

  def WriteFile(self):
    """Save music to disk."""
    save_path = os.path.join(self.MUSICS_DIR,
        self.music_info.artist_name + '-' + self.music_info.song_name + '.mp3')

    # TODO: add progress status
    fd = open(save_path, 'w')
    print 'Downloading to:', save_path
    handler = urllib2.urlopen(self.target_url)
    fd.write(handler.read())
    fd.close()

def main():
  for keyword in sys.argv[1:]:
    try:
      TingDownload(keyword).Download()
    except NotFoundError:
      print 'No result found'
    except DownloadError:
      print 'Download error'
    else:
      print 'Download successful'

if __name__ == '__main__':
  main()
