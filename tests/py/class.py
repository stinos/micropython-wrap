import upywraptest

print(callable(upywraptest.Simple(0)))
print(callable(upywraptest.Simple2()))

simple1 = upywraptest.Simple(0)
print(simple1())
simple1.Add(1)
print(simple1.Value())
print(simple1())
print(simple1)

simple2 = upywraptest.Simple(2)
print(simple2.Value())

simple1.Plus(simple2)
print(simple1.Value())

print(upywraptest.Simple(1).SimpleFunc(upywraptest.Simple(3)).Value())

simple1.val = 5
print(simple1.val)
try:
  simple1.val2
except AttributeError:
  print('AttributeError')

print(simple1 == simple2)
print(simple1 == simple1)
print(hash(simple1) == hash(simple2))
print(hash(simple1) == hash(simple1))
# Since we do not use MP_TYPE_FLAG_EQ_CHECKS_OTHER_TYPE this is automatically false.
# However if we would use that, forwarding __eq__ to Simple::operator == directly isn't
# really conforming to Python since it would raise a TypeError trying to convert the rhs.
# If you want something like that the correct way would be to have a wrapper function
# like Eq(const Simple&, mp_obj_t rhs) and then manually check types etc.
print(simple1 == upywraptest.NargsTest())
print(simple1 == upywraptest.Simple(5))
print(simple1 != upywraptest.Simple(5))
print(simple1 == upywraptest.Simple(6))

print(hasattr(simple1, 'Something'))
print(hasattr(simple1, 'val'))

print(upywraptest.Simple.x)
print(upywraptest.Simple.y)
print(upywraptest.Simple.z)

try:
  simple1.Plus(1)
except TypeError:
  print('TypeError')

try:
  simple1.Plus('a')
except TypeError:
  print('TypeError')

if upywraptest.FullTypeCheck():
  try:
    # Drawback of current implementation without UPYWRAP_FULLTYPECHECK:
    # this is allowed, causing undefined behavior.
    simple1.Plus(upywraptest.NargsTest())
  except TypeError:
    print('TypeError')

  try:
    # Simple2 wraps native NewSimple and the latter derives
    # from native Simple but this is not allowed because with
    # UPYWRAP_FULLTYPECHECK the base pointer types are not equal.
    simple1.Plus(upywraptest.Simple2())
  except TypeError:
    print('TypeError')
else:
  print('TypeError')
  print('TypeError')

try:
  simple1.SimpleFunc(None)
except Exception:
  print('TypeError')

print(upywraptest.Simple2().Value())
print(upywraptest.Simple3(20).Value())

print(upywraptest.IsNullPtr(None))
print(upywraptest.IsNullSharedPtr(None))

# Derive from native class.
class Derived1(upywraptest.Simple):
  def __init__(self, arg=0):
    super().__init__(arg)

  def Add(self, a):
    return super().Add(a)

# Derive from native class, omitting __init__, which is a different
# code path (see where subobj[0] gets assigned in objtype.c).
class Derived2(upywraptest.Simple2):
  pass

# One deeper in hierarchy.
class Derived3(Derived1):
  def __init__(self, arg=0):
    super().__init__(arg)

derived1 = Derived1(0)
derived1.Add(1)
derived2 = Derived2()
derived3 = Derived3(3)
print(derived1, derived2, derived3)

derived1.Plus(derived3)
derived3.Plus(derived1)
simple1.Plus(derived1)
simple1.Plus(derived3)
print(derived1, derived3, simple1)

print(upywraptest.Simple(4) == derived1)
print(derived1 == upywraptest.Simple(3))
print(derived1 == Derived3(4))
