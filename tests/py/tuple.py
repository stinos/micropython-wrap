import upywraptest

tup1 = [ 0, True, 1 ]
print( upywraptest.Tuple1( tup1 ) )
tup2 = [ tup1, 'a', [ tup1, tup1 ] ]
print( upywraptest.Tuple2( tup2 ) )
