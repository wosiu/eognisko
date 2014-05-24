/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */
#include <vector>
#include <iostream>
#include <cstdint>
#include <climits>
#include <map>
#include <boost/thread/mutex.hpp>
using namespace std;

class chuj {
public:
	chuj(int &a) : a(a) {};
	int a;

};

int main() {

	int b = 4;
	chuj c(b);
	cout << c.a << endl;
	b++;
	cout << c.a << endl;

	return 0;
}

