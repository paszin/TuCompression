#include <iostream>
#include <chrono>
#include <functional>
#include <bitset>

using namespace std;

int main()
{
	cout << "1,2,3,4, = 2^8" << endl;
	cout << sizeof(bitset<1>) << endl;
	cout << sizeof(bitset<2>) << endl;
	cout << sizeof(bitset<3>) << endl;
	cout << sizeof(bitset<4>) << endl;
	cout << "8 -> 64 = 2^8" << endl;
	cout << sizeof(bitset<8>) << endl;
	cout << sizeof(bitset<16>) << endl;
	cout << sizeof(bitset<24>) << endl;
	cout << sizeof(bitset<32>) << endl;
	cout << sizeof(bitset<40>) << endl;
	cout << sizeof(bitset<48>) << endl;
	cout << sizeof(bitset<56>) << endl;
	cout << sizeof(bitset<64>) << endl;
	cout << "65 -> 128 = 2^16" << endl;
	cout << sizeof(bitset<72>) << endl;
	cout << sizeof(bitset<128>) << endl;
	cout << "256 = 2^32" << endl;
	cout << sizeof(bitset<256>) << endl;
	cout << "512 = 2^64" << endl;
	cout << sizeof(bitset<512>) << endl;
	cout << "1024 = 2^128" << endl;
	cout << sizeof(bitset<1024>) << endl;
	cout << "2048 = 2^256" << endl;
	cout << sizeof(bitset<2048>) << endl;
	cout << "4096 = 2^512" << endl;
	cout << sizeof(bitset<4096>) << endl;

	return 0;
}