import upywraptest

try:
  upywraptest.TwoKw1()
except TypeError:
  print("TypeError")

try:
  upywraptest.TwoKw2(x=1)
except TypeError:
  print("TypeError")

print("TwoKw1")
upywraptest.TwoKw1(3)
upywraptest.TwoKw1(a=3)
upywraptest.TwoKw1(2, 3)
upywraptest.TwoKw1(2, b=3)
upywraptest.TwoKw1(a=2, b=3)

print("TwoKw2")
upywraptest.TwoKw2()
upywraptest.TwoKw2(3)
upywraptest.TwoKw2(3, 2)
upywraptest.TwoKw2(a=4)
upywraptest.TwoKw2(b=4)
upywraptest.TwoKw2(b=4, a=5)
upywraptest.TwoKw2(5, b=3)

try:
  upywraptest.KwargsTest(b=4)
except TypeError:
  print("TypeError")
upywraptest.KwargsTest()
upywraptest.KwargsTest(b="4")
kw = upywraptest.KwargsTest(c=5, b="4")
print("TwoKw1")
kw.TwoKw1(3)
kw.TwoKw1(a=3)
kw.TwoKw1(2, 3)
kw.TwoKw1(2, b=3)
kw.TwoKw1(a=2, b=3)
print("TwoKw2")
kw.TwoKw2()
kw.TwoKw2(3)
kw.TwoKw2(3, 2)
kw.TwoKw2(a=4)
kw.TwoKw2(b=4)
kw.TwoKw2(b=4, a=5)
kw.TwoKw2(5, b=3)
