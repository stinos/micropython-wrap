import upywraptest
import gc

# gc isn't entirely deterministic and without these two lines
# the linux build doesn't collect x, though arguably it should
a = 1
b = 2

def fun() :
  x = upywraptest.Context()

fun()

gc.collect()
