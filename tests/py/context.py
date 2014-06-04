import upywraptest
if upywraptest.HasFinaliser() :
  import gc

with upywraptest.Context() as p :
  pass

def fun() :
  x = upywraptest.Context()

fun()

if upywraptest.HasFinaliser() :
  gc.collect()
else :
  print( '__del__' )
