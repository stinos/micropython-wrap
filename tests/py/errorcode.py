import upywraptest

if not upywraptest.HasErrorCode():
  print('SKIP')
  raise SystemExit()

print(upywraptest.NoErrorCode())
try:
  print(upywraptest.SomeErrorCode())
except RuntimeError:
  print('OK')
