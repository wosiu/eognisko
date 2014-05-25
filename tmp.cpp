/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */
#include <vector>
#include <iostream>
#include <cstdint>
#include <climits>
#include <string>
#include <map>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
using namespace std;


int main() {
	boost::asio::ip::udp::endpoint ep;

	cout << ep.address().to_string() << endl;
	cout << ep << endl;
	return 0;
}

