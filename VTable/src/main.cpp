#include <iostream>

using namespace std;

class B1 {
public:
  virtual ~B1() {}
  void fnonvirtual() {}
  virtual void f1() {}
  int int_in_b1;
};

class B2 {
public:
  virtual ~B2() {}
  virtual void f2() {cout<<"B2"<<endl;};
  int int_in_b2;
};

class D : public B1, public B2 {
public:
  void d() {}
  void f2() override {cout<<"D"<<endl;};
  int int_in_d;
};

int main()
{

    D  *d  = new D();
    B1 *b1 = d;
    B2 *b2 = d;
    b2->f2();
    cout<<b1<<endl;
    cout<<b2<<endl;
    cout<<d<<endl;
    delete b2;
    cout<<d<<endl;
    return 0;
}
