#!/usr/bin/python

# Turn a commands file into a .exp file
# Usage: cd into examples directory and then run this script

import sys

f = open('commands', 'r')

sys.stdout.write('set pcieexpected {')

for line in f:
  if line.startswith('# Expected: '):
    sys.stdout.write('#\\s+avm_mem_readdata:\\s+')
    sys.stdout.write(line.split(' ')[2].strip()[2:])
    sys.stdout.write('\\n')
  elif line.startswith('# Expected result: '):
    sys.stdout.write('# At t=\\s+\\d+ clk=1 finish=1 return_val=\\s*')
    sys.stdout.write(line.split(' ')[3].strip())
    sys.stdout.write('\\n')
  elif line.startswith('poll_complete' ):
    sys.stdout.write('.*\\n?')

print '}'
