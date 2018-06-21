#include <vector>
#include <set>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <numeric>
#include <iomanip>

using namespace std;


template <typename D, typename C>
std::pair<std::vector<D>, std::vector<C>> compress(const std::vector<D> &column) {
	std::vector<D> dictionary(column.begin(), column.end());
	std::sort(dictionary.begin(), dictionary.end());
	auto last = std::unique(dictionary.begin(), dictionary.end());
	dictionary.erase(last, dictionary.end());

	std::vector<C> attributeVector(column.size());

	std::unordered_map<D, C> lookup(dictionary.size());
	C j = 0;
	for (const auto &key : dictionary) {
		lookup[key] = j;
		++j;
	}

	int i = 0;
	for (const auto &cell : column) {
		auto index = lookup[cell];
		attributeVector[i] = index;
		++i;
	}
	return std::pair(dictionary, attributeVector);
}

template <typename D, typename C>
std::pair<std::vector<D>, std::vector<C>> compress_v2(const std::vector<D> &column) {
	std::vector<D> dictionary(column.begin(), column.end());
	std::sort(dictionary.begin(), dictionary.end());
	auto last = std::unique(dictionary.begin(), dictionary.end());
	dictionary.erase(last, dictionary.end());

	std::vector<C> attributeVector(column.size());

	int i = 0;
	for (const auto &cell : column) {
		auto index = std::distance(dictionary.begin(), std::lower_bound(dictionary.begin(), dictionary.end(), cell));
		attributeVector[i] = index;
		++i;
	}
	return std::pair(dictionary, attributeVector);
}

int main()
{
	random_device rnd_device;
	mt19937 mersenne_engine {rnd_device()};
	uniform_int_distribution<int> dist {1, 1000000};

	auto gen = [&dist, &mersenne_engine]() {
		return dist(mersenne_engine);
	};

	vector<int> vec(1000000);

	int runs = 100;
	generate(begin(vec), end(vec), gen);
	cout << "With map" << endl;
	{
		vector<size_t> times;
		for (int i = 0; i < runs; ++i) {
			auto start = std::chrono::system_clock::now();
			volatile auto result = compress<int, uint8_t>(vec);
			auto elapsed = chrono::system_clock::now() - start;
			auto time = chrono::duration_cast<chrono::nanoseconds>(elapsed).count();
			times.push_back(time);
		}
		size_t sum = std::accumulate(times.begin(), times.end(), 0.0);
		size_t mean = sum / times.size();
		size_t var = 0;
		std::for_each (times.begin(), times.end(), [&](const size_t d) {
			var += (d - mean) * (d - mean);
		});
		size_t stddev = std::sqrt(var / (times.size() - 1));
		cout << "Mean (ns):\t" << mean << endl;
		cout << "Mean (ms):\t" << (mean / 1000000) << endl;
		cout.setf(ios::fixed);
		cout << "Stddev (ns):\t" << setprecision(0) << stddev << endl;
	}
	cout << "With lower_bound" << endl;
	{
		vector<size_t> times;
		for (int i = 0; i < runs; ++i) {
			auto start = std::chrono::system_clock::now();
			volatile auto result = compress_v2<int, uint8_t>(vec);
			auto elapsed = chrono::system_clock::now() - start;
			auto time = chrono::duration_cast<chrono::nanoseconds>(elapsed).count();
			times.push_back(time);
		}
		size_t sum = std::accumulate(times.begin(), times.end(), 0.0);
		size_t mean = sum / times.size();
		size_t var = 0;
		std::for_each (times.begin(), times.end(), [&](const size_t d) {
			var += (d - mean) * (d - mean);
		});
		size_t stddev = std::sqrt(var / (times.size() - 1));
		cout << "Mean (ns):\t" << mean << endl;
		cout << "Mean (ms):\t" << (mean / 1000000) << endl;
		cout.setf(ios::fixed);
		cout << "Stddev (ns):\t" << setprecision(0) << stddev << endl;
	}
}