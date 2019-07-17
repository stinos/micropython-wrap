import upywraptest

class X:
  def __init__(self):
    self.a = 1

class X2:
  def __init__(self):
    self.x = X()

a = 1
x = X()
x2 = X2()

upywraptest.TestVariables()

print(a)
print(x.a)
print(x2.x.a)
