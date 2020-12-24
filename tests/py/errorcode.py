import upywraptest

try:
  upywraptest.NoErrorCode
except AttributeError:
  print('SKIP')
  raise SystemExit()

print(upywraptest.NoErrorCode())
try:
  print(upywraptest.SomeErrorCode())
except RuntimeError:
  print('OK')
