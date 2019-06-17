import upywraptest

try:
  upywraptest.NullOpt
except:
  print('SKIP')
  raise SystemExit()

print(upywraptest.NullOpt() is None)
print(upywraptest.OptionalInt())
print(upywraptest.OptionalVector())
upywraptest.OptionalArgument(None)
upywraptest.OptionalArgument([1, 2])
