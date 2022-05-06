#Text_file_compression_tool

## Intro and technical details

The goal of this project is to write a program that has the ability to compress a text file without pre-processing it.It does so, first by applying the LZW compression algorithm of variable dictionary size (initially 1024 locations) and then forwarding such output to the static Huffman algorithm input. The goal is also to compare the results with the commercial WinRar compression software. Particular attention is paid to _sufficiently large_ files in which statistical features come to the fore..

The code was written and tested in the C ++ programming language and the std :: c ++ 17 standard. The code writing paradigm is completely procedural.

## Implementation

We start by defining following auxiliary declarations:
```c++
using i64 = long long;
constexpr short BYTE_WIDTH = 8;
constexpr unsigned DICT_SIZE = 1024;
```
>Constant expression `DICT_SIZE` represents number of dictionary entries available for LZW procedure.


For simplicity, the program compresses a text file that needs to be submitted to its parent directory named `input.txt` and as a result creates a compressed version of the file as` output.txt`.

```c++
std::ifstream raw("input.txt", std::ios::in);
std::ofstream compressed("output.txt", std::ios::out);

//proccessing
    
raw.close();
compressed.close();
```
>Note: It is not complicated to extend the program with some kind of user interface for delivering input.

*Processing* itself is realized through three stages:

```c++
auto[occurrence, lzwCompressed] = inputToLzw(raw);

std::unique_ptr<std::string[]> huffmanMap = huffman(occurrence);

bitSerialization(lzwCompressed, huffmanMap, compressed);
```
- Reading from file and applying LZW algorithm
- Coding the results of the previous point with Huffman codes
- Serialization of the obtained codes into a continuous bit stream

## File reading and application of LZW compression algorithm

As the idea is text processing within the set of characters defined by the ASCII standard, it is necessary to initialize the first 256 ([0 - 255]) positions within the table (dictionary), which is used by the algorithm itself with individual ASCII (Extended) characters.

```c++
auto occurrence = std::make_unique<i64[]>(DICT_SIZE);
std::unordered_map<std::string, short> dictionary;

for (int i = 0; i < 256; ++i) {
	dictionary[std::string(1, static_cast<char>(i))] = i;
}

std::vector<short> lzwResult;
```

For the sake of later easier statistical analysis that precedes the application of Huffman, in parallel with the processing, records are kept of the occurrence of dictionary entries within the `occurrence` array. This is also the first return value of this function. An array, in this case wrapped in a smart pointer, is the fastest and most intuitive solution, because each code (from segment 0 to `DICT_SIZE` - 1) is mapped directly to its inputs.

The dictionary is implemented as a hash table (_unordered_map_) whose key is a string (part of the text to be processed).

The second return value of the function, `lzwResult`, represents the final result of the algorithm and is returned as a vector of integer values representing the coded values. 

```c++
std::string current = "";
int dictProgress = dictionary.size();
std::string buffLine;
```

The implementation of the algorithm core is direct - a string of symbols from the input is built within the variable `current`. In case that the built character string concatenated with the just loaded symbol is already mapped to its integer code, `current` is expanded and the next symbol is being read, otherwise the current value (which is guaranteed to be mapped) is translated into the code and inserted into the result vector, updates to its number of occurrences are being made and newly-made string gets a spot inside the dictionary:

```c++
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
```
The individual lines of the file are loaded into the `buffLine` variable through which we iterate.

> The local variable `dictProgress` is a dictionary size limit and if it is equal to the global limit set at the beginning of the program, no further mapping within the dictionary is made.

When the end of the file is reached, what is left within `current` is added to the result and returned to the main program:

```c++
lzwResult.push_back(dictionary[current]);
return { std::move(occurrence), lzwResult };
```


## Application of static Huffman

We introduce the structure of the class `Node` which will represent the Huffman tree:

```c++
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
```

Huffman codes will be returned through array, similar to symbol occurrences.

```c++
auto huffmanMap = std::make_unique<std::string[]>(DICT_SIZE);
std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

for (int i = 0; i < DICT_SIZE; ++i) {
	if (!occurrence[i]) continue;
	pq.push(Node(occurrence[i], i));
}
```
For us to be able to retrieve the two nodes with the lowest repetition rate in each iteration of the algorithm, the priority queue with inverted order is used. As this queue internally compares objects of type `Node`, it is necessary to overload the above-mentioned  ` operator> ` which compares the accumulated _frequencies_ of nodes. Each code obtained using the LZW algorithm that appears at least once is inserted into the queue paired with its number of occurrences.

```c++
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
```

The idea of ​​implementing the algorithm is as follows:

- Two nodes with the lowest number of occurrences are retrieved from the queue.
- As with the intuitive procedure visually demonstrated by drawing the tree, they are being _taken_ by parent node. The role of the parent node without diminishing the generality will be taken over by the first node retrieved.
- As the vector `idx` of each node represents all codes for which that node became responsible by taking on the role of parent node (initially each node is _responsible_ only for itself), it is necessary to update the current paths to these codes by iterating through both vectors, again, without diminutions of generality, codes _under_ the first node retrieved get 0 while all those for which the second node is responsible get 1 in their currently formed Huffman path.
- By invoking the function `combine`, we append all the elements of the vector of the same name of the second node to the` idx` vector of the first node and add the cumulative numbers of occurrences.
- The newly created parent node is put back into priority queue.

> The procedure ends when one node remains in the queue.

It is not hard to notice that this kind of procedure forms a binary path for each code _bottom-up_ towards the top of the obtained tree, so in the end it is enough to return the mapped strings, each in reverse order:

```c++
for (int i = 0; i < DICT_SIZE; ++i)
	std::reverse(huffmanMap[i].begin(), huffmanMap[i].end());

return huffmanMap;
```

## Serialization of binary codes into a output text file

The idea behind this procedure is extremely simple, we iterate through the result codes of the LZW algorithm, the corresponding Huffman equivalents that are of type string are being mapped and bit by bit (character by character) _printed_ into the characters that are ending up in the output file.

```c++
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
```
The current progress of the character (variable `imprint`), which is next to be printed, is stored within the `progress` variable. When the number of already imprinted bits is equal to the bit width of the print character, `imprint` is passed to the output stream towards the file and together with evidence variables is reverted to its default value. The next free location within a character is easily _hit_ by moving the bit unit a certain number of times which is a function of the total width of the character and the current progress within it.

In the end, `progress` is sent as the last character so that the eventual error within the penultimate sent character could be corrected during the decompression.

> The last two lines are intuitively added.


## Results and conclusions

The following table shows the results of the program when working with text files of size euqal to a couple of megabytes, downloaded from the Internet.

| File | Original size[KB] | this.cpp - size[KB] | WinRar - siize[KB]
| ------ | ------ | ------ | ------ |
| [world192.txt](https://corpus.canterbury.ac.nz/descriptions/large/world.html) | 2,416 | 1,344 | 702
| [bible.txt](https://corpus.canterbury.ac.nz/descriptions/large/bible.html) | 3,953 | 1,815 | 1,149
| [E.coli.txt](https://corpus.canterbury.ac.nz/descriptions/large/E.coli.html) | 4,530 | 1,182 | 1,294

The last column of the table gives the results achieved by compressing the same file in .ZIP format using the default settings for the mentioned WinRar format. It is interesting to note the compression efficiency of the third file, which is a serialized genome of the bacterium which actually means that it is composed of a small set of characters and that it, intuitively, should express the high existence of memory.

> Note:
The mentioned settings for the mentioned WinRar program format are far from the _best_ compression that some of the commercial programs can offer.

All files can be found and downloaded from the site [the cantebury corpus](https://corpus.canterbury.ac.nz/index.html).

The table below shows the results in working with [bible.txt](https://corpus.canterbury.ac.nz/descriptions/large/bible.html) file for different sizes of the global variable `DICT_SIZE` which in previous cases had a default value of 1024. In addition to testing, execution times for each of the cases are included.

> Reminder: The original file size was 3,953 KB

| DICT_SIZE | Size after compression[KB] | Execution time[ms] 
| ------ | ------ | ------ |
| 1024 | 1,815 | 8,532 
| 2048 | 1,710 | 9,075 
| 4096 | 1,609 | 9,252 
| 8192 | 1,516 | 9,761 
| 16384 | 1,428 | 9,553 

In the given example, it can be noticed that for the exponential growth of the number of entries in the table and in this interval, we get an approximate linear decrease in the size of the output file, which is somewhat expected for this type of files.

Finally, it is good to notice that the execution time of the program minimally depends on the size of the dictionary used, ie. that the dominant factor in the time complexity of the program is certainly the size of the input file.
