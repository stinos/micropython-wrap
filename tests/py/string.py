import upywraptest

print(upywraptest.StdString('a'))
print(upywraptest.StdStringView('abc'))
if upywraptest.HasCharString():
  print(upywraptest.CharString('b'))
else:
  print('b')
