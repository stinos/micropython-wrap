import upywraptest

m = upywraptest.Map1({"a": 1, "b": 2, "def": 444})
print([(k, m[k]) for k in sorted(m.keys())])
m = upywraptest.Map2({"a": [1], "b": [2]})
print([(k, m[k]) for k in sorted(m.keys())])
