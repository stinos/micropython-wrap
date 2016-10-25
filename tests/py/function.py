import upywraptest
import math

def func1() :
  print( 'func1' )

def func1gen( a ) :
  def fn() :
    print( a )
  return fn

upywraptest.Func1( func1 )
upywraptest.Func1( func1gen( 'func1' ) )
upywraptest.Func1( lambda : print( 'func1' ) )


def func2( a ) :
  print( a )

upywraptest.Func2( func2 )


def func3() :
  return 3

upywraptest.Func3( func3 )


def func4( i, s ) :
  return '{}{}'.format( i, s )

upywraptest.Func4( func4 )

#native functions
upywraptest.Func5( math.fabs )
upywraptest.Func5( upywraptest.Func6 )

#more than 3 arguments
def Take4( a, b, c, d ):
  return a + b + c + d

print( upywraptest.Func8( upywraptest.Func7 ) )
print( upywraptest.Func8( Take4 ) )
