#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <queue>

using i64 = long long;
constexpr short BYTE_WIDTH = 8;
constexpr unsigned DICT_SIZE = 1024;

struct LzwProduct {
	std::unique_ptr<i64[]> occurrence;
	std::vector<short> table;
};

LzwProduct inputToLzw(std::ifstream& is) {
	auto occurrence = std::make_unique<i64[]>(DICT_SIZE);
	std::unordered_map<std::string, short> dictionary;

	for (int i = 0; i < 256; ++i) {
		dictionary[std::string(1, static_cast<char>(i))] = i;
	}
	std::vector<short> lzwResult;

	std::string current = "";
	int dictProgress = dictionary.size();
	std::string buffLine;

	while (std::getline(is, buffLine)) {
		buffLine += '\n';
		for (const auto ch : buffLine) {
			if (dictionary.find(current + ch) != dictionary.end())
				current += ch;
			else {
				lzwResult.push_back(dictionary[current]);
				++occurrence[dictionary[current]];
				if (dictProgress != DICT_SIZE) {
					dictionary[current + ch] = dictProgress++;
				}
				current = ch;
			}
		}
	}
	lzwResult.push_back(dictionary[current]);
	return { std::move(occurrence), lzwResult };
}

struct Node {
	double freq;
	std::vector<int> idx;
	Node(double _freq, int _idx) : freq(_freq) {
		idx.push_back(_idx);
	}
	void combine(const Node& n) {
		freq += n.freq;
		for (const auto el : n.idx)
			idx.push_back(el);
	}
	bool operator>(const Node& n1) const {
		return this->freq > n1.freq;
	}
};

std::unique_ptr<std::string[]> huffman(const std::unique_ptr<i64[]>& occurrence) {

	auto huffmanMap = std::make_unique<std::string[]>(DICT_SIZE);
	std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

	for (int i = 0; i < DICT_SIZE; ++i) {
		if (!occurrence[i]) continue;
		pq.push(Node(occurrence[i], i));
	}

	while (pq.size() > 1) {
		Node n1 = pq.top();
		pq.pop();
		Node n2 = pq.top();
		pq.pop();
		for (const auto el : n1.idx) huffmanMap[el] += "0";
		for (const auto el : n2.idx) huffmanMap[el] += "1";
		n1.combine(n2);
		pq.push(n1);
	}
	for (int i = 0; i < DICT_SIZE; ++i)
		std::reverse(huffmanMap[i].begin(), huffmanMap[i].end());

	return huffmanMap;
}

void bitSerialization(const std::vector<short>& lzwCompressed,
	std::unique_ptr<std::string[]>& huffmanMap,
	std::ofstream& os) {

	unsigned char imprint = 0;
	short progress = 0;

	for (const auto el : lzwCompressed) {
		const std::string& target = huffmanMap[el];
		for (const auto ch : target) {
			if (ch == '1') {
				imprint |= (0x1 << (BYTE_WIDTH * sizeof(imprint) - 1 - progress));
			}
			++progress;
			if (progress == BYTE_WIDTH * sizeof(imprint)) {
				os << imprint;
				progress = 0;
				imprint = 0;
			}
		}
	}
	imprint = progress;
	os << imprint;
}

int main() {

	std::ifstream raw("bible.txt", std::ios::in);
	std::ofstream compressed("output.txt", std::ios::out);

	auto [occurrence, lzwCompressed] = inputToLzw(raw);

	std::unique_ptr<std::string[]> huffmanMap = huffman(occurrence);

	bitSerialization(lzwCompressed, huffmanMap, compressed);

	raw.close();
	compressed.close();

	return 0;
}