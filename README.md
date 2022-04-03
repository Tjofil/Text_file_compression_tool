# **Textual files compression using Static Huffman and Lempel-Ziv algorithms**

[![Generic badge](https://img.shields.io/badge/docs-stable-blue.svg)](https://tjofil.github.io/Text_file_compression_tool/)


## Summary

The goal of this project is to write a program that has the ability to compress a text file without pre-processing it. It does so, first by applying the LZW compression algorithm of variable dictionary size (initially 1024 locations) and then forwarding such output to the static Huffman algorithm input. The goal is also to compare the results with the commercial WinRar compression software. Particular attention is paid to _sufficiently large_ files in which statistical features come to the fore..

**Detailed implementation and technical informations can be found inside [detailed documentation](https://tjofil.github.io/Text_file_compression_tool/).**

## Some results and conclusions

The following table shows the results of the program when working with text files of size euqal to a couple of megabytes, downloaded from the Internet.

| File | Original size[KB] | this.cpp - size[KB] | WinRar - siize[KB]
| ------ | ------ | ------ | ------ |
| [world192.txt](https://corpus.canterbury.ac.nz/descriptions/large/world.html) | 2,416 | 1,344 | 702
| [bible.txt](https://corpus.canterbury.ac.nz/descriptions/large/bible.html) | 3,953 | 1,815 | 1,149
| [E.coli.txt](https://corpus.canterbury.ac.nz/descriptions/large/E.coli.html) | 4,530 | 1,182 | 1,294



