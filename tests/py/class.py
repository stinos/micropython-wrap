import upywraptest

simple1 = upywraptest.Simple( 0 )
simple1.Add( 1 )
print( simple1.Value() )
print( simple1 )

simple2 = upywraptest.Simple( 2 )
print( simple2.Value() )

simple1.Plus( simple2 )
print( simple1.Value() )

print( upywraptest.Simple( 1 ).SimpleFunc( upywraptest.Simple( 3 ) ).Value() )

simple1.val = 5
print( simple1.val )
try:
  simple1.val2
except AttributeError:
  print( 'AttributeError' )

print( simple1 == simple2 )
print( simple1 == simple1 )
print( hash( simple1 ) == hash( simple2 ) )
print( hash( simple1 ) == hash( simple1 ) )

print( hasattr( simple1, 'Something' ) )
print( hasattr( simple1, 'val' ) )

print( upywraptest.Simple.x )
print( upywraptest.Simple.y )
print( upywraptest.Simple.z )

try:
  simple1.Plus( 1 )
except TypeError:
  print( 'TypeError' )

try:
  simple1.Plus( 'a' )
except TypeError:
  print( 'TypeError' )

try:
  simple1.Plus( upywraptest.NargsTest() )
except TypeError:
  print( 'TypeError' )

try:
  simple1.SimpleFunc( None )
except Exception:
  print('TypeError')

print(upywraptest.Simple2(10).Value())
print(upywraptest.Simple3(20).Value())
