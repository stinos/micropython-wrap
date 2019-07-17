import upywraptest

if upywraptest.HasExceptions():
  try:
    upywraptest.Throw('oops')
  except RuntimeError as err:
    print(err)
else:
  print('oops')
