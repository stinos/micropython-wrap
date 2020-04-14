import gc
import upywraptest

def TestRefCount(getObject):
  simpleCollection = upywraptest.SimpleCollection()
  simpleCollection.Add(getObject())
  for i in range(20):
    if simpleCollection.Reference(0) == 1:
      print('collect did reclaim object', i)
      break
    gc.collect()
  else:
    print('collect did not reclaim object')

if __name__ == '__main__':
  TestRefCount(lambda: upywraptest.Simple(1))
