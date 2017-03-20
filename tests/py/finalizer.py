import upywraptest
import gc

# gc isn't entirely deterministic and without these lines
# the linux build doesn't collect x, though arguably it should
a = 1
b = 2
c = 3

def allocate_some():
  d = [a] * 100
  e = [3] * 100
  f = [4] * 100

def fun():
  x = upywraptest.Context()

fun()
allocate_some()

gc.collect()
