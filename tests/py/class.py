import upywraptest

simple1 = upywraptest.Simple( 0 )
simple1.Add( 1 )
print( simple1.Value() )

simple2 = upywraptest.Simple( 2 )
print( simple2.Value() )

simple1.Plus( simple2 )
print( simple1.Value() )

print( upywraptest.Simple( 1 ).SimpleFunc( upywraptest.Simple( 3 ) ).Value() )
