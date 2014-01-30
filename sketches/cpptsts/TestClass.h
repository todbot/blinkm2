
#ifndef TESTCLASS_H
#define TESTCLASS_H

template <int N>
class TestClass
{
	enum { arrsize = N };

public:
    int getSize() { return arrsize; }
    int elementAt(int i) { return _array[i]; }
    void setAt(int i, int j) { _array[i] = j; }

private:
    //int len = N;
    int _array[N];
};

#endif
