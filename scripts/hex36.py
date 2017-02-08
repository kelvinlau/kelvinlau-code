#!/usr/bin/env python

import string

ibase = 16
obase = 36
radix = string.digits + string.ascii_lowercase

x = int(raw_input(), ibase)
s = ''
while x:
  s = radix[x % obase] + s
  x /= obase
print s
