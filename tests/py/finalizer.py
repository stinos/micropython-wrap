import upywraptest
import gc

# gc isn't entirely deterministic and without these lines
# the linux build doesn't collect x, though arguably it should
a = 1
b = 2
c = 3

def allocate_some():
  d = [a] * 100  # noqa
  e = [3] * 100  # noqa
  f = [4] * 100  # noqa

def fun():
  x = upywraptest.Context()  # noqa

fun()
allocate_some()

gc.collect()
