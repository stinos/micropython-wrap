import upywraptest

simple1 = upywraptest.Simple( 0 )
simple1.Add( 1 )
print( simple1.Value() )

simple2 = upywraptest.Simple( 2 )
print( simple2.Value() )

simple1.Plus( simple2 )
print( simple1.Value() )

print( upywraptest.Simple( 1 ).SimpleFunc( upywraptest.Simple( 3 ) ).Value() )

simple1.val = 5
print( simple1.val )

print( simple1 == simple2 )
print( simple1 == simple1 )
print( simple1.__hash__() == simple2.__hash__() )
print( simple1.__hash__() == simple1.__hash__() )
