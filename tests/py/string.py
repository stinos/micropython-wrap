import upywraptest

print( upywraptest.StdString( 'a' ) )
if upywraptest.HasCharString() :
  print( upywraptest.CharString( 'b' ) )
else :
  print( 'b' )
