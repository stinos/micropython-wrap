import upywraptest
import gc

# gc isn't entirely deterministic and without these lines
# the linux build doesn't collect x, though arguably it should
a = 1
b = 2
c = 3
d = [a] * 13

def fun() :
  x = upywraptest.Context()

fun()

gc.collect()
