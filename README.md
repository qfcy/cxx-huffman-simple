**The English introduction is placed below the Chinese version.**

一个简单的C++霍夫曼编码实现，将输入二进制文件数据压缩成`.compressed`格式，并支持解压。  

#### 命令行
编译：`g++ huffman.cpp -o huffman -O2 -s -Wall`。  
压缩：`huffman file.txt`，程序会生成`file.txt.compressed`作为压缩的结果。  
解压：`huffman file.txt.compressed`，程序会生成`file.txt`。如果`file.txt`已存在，则会提示替换。  

#### 文件结构与格式

文件头部分：  
- 解压后数据大小`n` (8字节无符号整数)  
- 霍夫曼树位图占用的字节数`m` (2字节`ushort`)  
- 霍夫曼树位图，用二进制位表示 (`m`字节，0表示内部节点，1表示叶子结点，详见下文)  
- 霍夫曼树每个节点代表的字符信息，越前面的字符位于树的越浅层，出现的频率越高 （`m`字节，例如：`<空格>eonsa\nritpd)(;clfug*<>+bz/,_m ...`）

文件体部分：  
- 霍夫曼编码的原始文件数据 (任意大小，但解压后的数据需要达到n字节，否则被视为文件截断)  

#### 霍夫曼树位图的存储

这个项目优化了霍夫曼树位图的存储空间，也就是遇到叶子结点之后不再往下存储。  
传统的存储（a[2\*n+1]和a[2\*n+2]表示a[n]的子节点）：  
```
0
10
0011
```
如果霍夫曼树有十几层深，霍夫曼树的大小就会达到KB级别，这是不可接受的。  
由于叶子结点1下面不可能存在新的叶子结点，这里的实现省略了多余的底部内部节点：  
```
0
10
11
```
第三层的11连接到第二层的0。这里省略了两个00，优化了空间。  


A lightweight C++ Huffman encoding implementation that compresses input binary file data into the `.compressed` format and supports decompression.

#### Command Line
- **Compiling**: `g++ huffman.cpp -o huffman -O2 -s -Wall`.  
- **Compression**: `huffman file.txt`  
  The program generates `file.txt.compressed` as the compressed result.  
- **Decompression**: `huffman file.txt.compressed`  
  The program generates `file.txt`. If `file.txt` already exists, the program will prompt for replacement.

#### File Structure and Format

**File Header**:  
- Size of decompressed data `n` (8-byte unsigned long long)  
- Size of the Huffman tree bitmap in bytes `m` (2-byte unsigned short)  
- Huffman tree bitmap represented in binary bits (`m` bytes, where `0` represents an internal node and `1` represents a leaf node, see details below)  
- Character information for each node in the Huffman tree. Characters appearing earlier in the list are closer to the root of the tree and have higher frequencies (`m` bytes, e.g., `<space>eonsa\nritpd)(;clfug*<>+bz/,_m ...`)  

**File Body**:  
- Huffman-encoded original file data (arbitrary size, but the decompressed data must match `n` bytes; otherwise, the file is considered truncated)

#### Storage of the Huffman Tree Bitmap

This project optimizes the storage space for the Huffman tree bitmap by omitting unnecessary internal nodes after encountering leaf nodes.  

**Traditional Storage** (where `a[2*n+1]` and `a[2*n+2]` represent the child node of `a[n]`):  
```
0
10
0011
```
If the Huffman tree has a depth of more than 12 levels, its size could reach kilobytes, which is unacceptable.  

**Optimized Storage**:  
Since no new leaf nodes can exist below a leaf node (`1`), the code omits redundant internal nodes at the bottom:  
```
0
10
11
```
In this example, the third level's `11` connects to the second level's `0`. Two `00` nodes are omitted, optimizing space usage.  
