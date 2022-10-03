import upywraptest

print(upywraptest.StdString('a'))
if upywraptest.HasStringView():
  print(upywraptest.StdStringView('abc'))
else:
  print('abc')
if upywraptest.HasCharString():
  print(upywraptest.CharString('b'))
else:
  print('b')
