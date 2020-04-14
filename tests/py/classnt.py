""" This tests some internals and special considerations for inheriting (both in C++ and uPy). """

import classref
import upywraptest

class Derived(upywraptest.Simple2):
  def __init__(self):
    super().__init__()

  def __str__(self):
    return 'Derived' + super().__str__()

simpleCollection = upywraptest.SimpleCollection()
s2 = upywraptest.Simple2()

if upywraptest.FullTypeCheck():
  try:
    simpleCollection.Add(s2)
  except TypeError:
    print("SKIP")
    raise SystemExit

simpleCollection.Add(s2)
simpleCollection.Add(Derived())
s1 = simpleCollection.Get(0)
d1 = simpleCollection.Get(1)

# Since Simple2 overrides Str() in C++ this will use the override.
print(s1)

# However since s1 is of Python type Simple it doesn't have this method
try:
  s1.Name()
except AttributeError:
  pass

# Likewise, this will *not* call Derived.__str__!
print(d1)

# Just to verify, this must be 3: s2, the one in the collection and s1.
print('ref count', simpleCollection.Reference(0))

classref.TestRefCount(lambda: upywraptest.Simple2())
# TODO: segfaults on linux
# classref.TestRefCount(lambda: Derived())
