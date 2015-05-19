import upywraptest
import gc

with upywraptest.Context() as p :
  pass

def fun() :
  x = upywraptest.Context()

fun()

gc.collect()
