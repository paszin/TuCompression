#include <vector>
#include <set>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <random>

using namespace std;

int main()
{
	random_device rnd_device;
	mt19937 mersenne_engine {rnd_device()};
	uniform_int_distribution<int> dist {1, 1000000};

	auto gen = [&dist, &mersenne_engine]() {
		return dist(mersenne_engine);
	};

	vector<int> vec(1000000);
	generate(begin(vec), end(vec), gen);

	{
		auto start = std::chrono::system_clock::now();

		set<int> setDict(vec.begin(), vec.end());
		std::vector<int> sorted(setDict.begin(), setDict.end());

		auto elapsed = chrono::system_clock::now() - start;
		cout << "Time: " << chrono::duration_cast<chrono::milliseconds>(elapsed).count() << endl;
	}


	{
		auto start = std::chrono::system_clock::now();

		vector<int> vecDict = vec;
		sort(vecDict.begin(), vecDict.end());
		unique(vecDict.begin(), vecDict.end());

		auto elapsed = chrono::system_clock::now() - start;
		cout << "Time: " << chrono::duration_cast<chrono::milliseconds>(elapsed).count() << endl;
	}
}