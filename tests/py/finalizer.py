import upywraptest
import gc

def fun() :
  x = upywraptest.Context()

fun()

gc.collect()
