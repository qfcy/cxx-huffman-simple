#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <queue>
#include <bitset>
#include <unistd.h>
using namespace std;

using ul=size_t; // 32位为unsigned int,64位为unsigned long long
using ull=unsigned long long;
using uchar=unsigned char;
using ushort=unsigned short;
using uint=unsigned int;
struct HuffmanTree{
    HuffmanTree *left;
    bool isLeave;
    uchar c;size_t weight;
    HuffmanTree *right;
};
struct HuffCode{
    ul code;
    size_t bitcnt;
    friend ostream& operator<<(ostream &cout_, const HuffCode &h) {
        for(int i=h.bitcnt-1;i>=0;i--){
            cout_<<((h.code&(1<<i))>>i); // 取第i位(i从0开始)
        }
        return cout_;
    }
};
const char *EXTENSION=".compressed";
pair<uchar,uchar> splitByte(ul byte,int offset){
    // 分割字节数据，如分割00110100,3得到(00)00110,100(00000)，允许offset大于8或者为负数
    uchar low,high;
    if(offset<0){
        low=byte<<(-offset);
        high=0;
    } else {
        low=byte>>offset;
        if(offset>8) high=byte>>(offset-8);
        else high=byte<<(8-offset);
    }
    return make_pair(low,high);
}
void getEncodings(map<uchar,HuffCode> &result,const HuffmanTree *tree,size_t level,ul code){ // 中序遍历
    if(tree->isLeave)result[tree->c]=HuffCode{code,level};
    if(tree->left)getEncodings(result,tree->left,level+1,code<<1);
    if(tree->right)getEncodings(result,tree->right,level+1,(code<<1)|1); //右侧代表1
}
HuffmanTree *buildHuffmanTree(map<uchar,size_t> count){
    // 根据字符频率建立霍夫曼树
    auto compare=[](HuffmanTree *a,HuffmanTree *b){
        return a->weight>b->weight; // 最小堆
    };
    priority_queue<HuffmanTree*,vector<HuffmanTree*>,decltype(compare)> que(compare);

    for(auto [c,cnt]:count){ // 初始化霍夫曼树
        HuffmanTree *node=new HuffmanTree{nullptr,true,c,cnt,nullptr};
        que.push(node);
    }
    while(que.size()>1){ // 建立霍夫曼树
        HuffmanTree *left=que.top();que.pop();
        HuffmanTree *right=que.top();que.pop();
        HuffmanTree *node=new HuffmanTree{left,false,'\0',left->weight+right->weight,right};
        que.push(node);
    }
    HuffmanTree *tree=que.top();que.pop();
    return tree;
}
void freeTree(HuffmanTree *tree){ // 释放霍夫曼树的内存
    if(tree->left)freeTree(tree->left);
    if(tree->right)freeTree(tree->right);
    delete tree;
}
uchar *encodeTree(HuffmanTree *tree,size_t size,size_t depth,size_t *output_size){
    // 参数size: 字符数量
    deque<HuffmanTree*> que;
    bool *bitmap=new bool[1<<depth]; // 二叉树的位图
    memset(bitmap,0,sizeof(bool)*(1<<depth));
    bitmap[0]=tree->isLeave;
    // 建立位图
    uchar *chars=new uchar[size]; // 存储叶子结点的字符
    size_t char_idx=0,idx=0; // char_idx:chars的当前索引,idx:bitmap的索引
    que.push_back(tree);
    while(!que.empty()){
        HuffmanTree *node=que.front();
        que.pop_front();
        bitmap[idx++]=node->isLeave;
        if(node->isLeave){
            chars[char_idx++]=node->c;
        }else{
            if(node->left)que.push_back(node->left);
            if(node->right)que.push_back(node->right);
        }
    }
    // 组装结果
    *output_size=sizeof(ushort)+(1+idx/8)+size; // idx是最后一个节点，因此idx+1为实际存储二叉树需要的大小
    uchar *result=new uchar[*output_size];
    memset(result,0,*output_size);
    size_t bitmapSizeByte=1+idx/8; // 二叉树位图用的大小(字节)
    *((ushort*)result)=bitmapSizeByte;
    size_t index=0; // bitmapSize=1+idx; // bitmapSize:位图大小(bit)
    for(size_t i=sizeof(ushort);i<bitmapSizeByte+sizeof(ushort);i++){
        bool flag=false;
        for(int j=7;j>=0;j--){
            if(bitmap[index])result[i]|=1<<j;
            index++;
            if(index>=idx){
                flag=true;break;
            }
        }
        if(flag)break;
    }
    memcpy(result+sizeof(ushort)+bitmapSizeByte,chars,size); // 叶子结点的字符
    delete[] bitmap;delete[] chars;
    return result;
}
HuffmanTree *decodeTree(const uchar *data,size_t *tree_size){
    size_t bitmapSizeByte=*((ushort*)data);
    size_t bitmap_size=bitmapSizeByte*8; // 位图大小(bit)
    bool *bitmap=new bool[bitmap_size]; // 转换为二叉树的位图
    size_t leave_cnt=0;
    size_t idx=0;
    for(size_t i=sizeof(ushort);i<bitmapSizeByte+sizeof(ushort);i++){
        for(int j=7;j>=0;j--){
            bool value=((data[i]&(1<<j))>>j);
            bitmap[idx++]=value;
            if(value)leave_cnt++;
            if(idx>=bitmap_size)break;
        }
    }

    *tree_size=sizeof(ushort)+bitmapSizeByte+leave_cnt;
    deque<HuffmanTree*> que;size_t char_idx=0;idx=0;
    const uchar *chars=data+(sizeof(ushort)+bitmapSizeByte);
    HuffmanTree *root=new HuffmanTree{nullptr,bitmap[idx++],'\0',0,nullptr};
    if(root->isLeave)root->c=chars[char_idx++];
    else que.push_back(root);
    while(!que.empty()){ // 要求每条分支必须能到达叶子节点
        HuffmanTree *node=que.front();
        que.pop_front();
        HuffmanTree *left=new HuffmanTree{nullptr,
            (idx<bitmap_size)?(bitmap[idx++]):false,
            '\0',0,nullptr};
        if(left->isLeave)left->c=chars[char_idx++];
        else que.push_back(left);
        HuffmanTree *right=new HuffmanTree{nullptr,
            (idx<bitmap_size)?(bitmap[idx++]):false,
            '\0',0,nullptr};
        if(right->isLeave)right->c=chars[char_idx++];
        else que.push_back(right);
        node->left=left;node->right=right;
    }
    delete[] bitmap;
    return root;
}
uchar *compress(ull data_size, const uchar *data, ull *output_size) {
    if(data_size==0){ // 特殊处理空文件
        *output_size=sizeof(ull);
        uchar *res=new uchar[sizeof(ull)];
        *((ull*)res)=0;
        return res;
    }

    map<uchar,size_t> count;
    for(size_t i=0;i<data_size;i++){
        if(count.find(data[i])==count.end()){
            count[data[i]]=1;
        }else{
            count[data[i]]++;
        }
    }
    auto compare=[&count](pair<uchar,size_t> a,pair<uchar,size_t> b){
        return a.second<b.second;
    };
    vector<pair<uchar,size_t>> sortedcnt;
    for(auto [c,cnt]:count) sortedcnt.emplace_back(c,cnt);
    sort(sortedcnt.begin(),sortedcnt.end(),compare);

    HuffmanTree *tree=buildHuffmanTree(count);
    map<uchar,HuffCode> huffcoding;
    getEncodings(huffcoding,tree,0,0); // 建立霍夫曼编码
    //for(auto [c,code]:huffcoding)cout<<c<<':'<<code<<" ("<<count[c]<<')'<<endl;

    size_t depth=0,wpl=0; // 计算WPL，得到压缩数据大小
    for(auto [c,code]:huffcoding){
        depth=max(depth,code.bitcnt); // 树的最大深度
        wpl+=code.bitcnt*count[c];
    }
    size_t tree_size,leave_cnt=count.size();
    uchar *encoded_tree=encodeTree(tree,leave_cnt,depth+1,&tree_size); // 获得二叉树的编码

    // 写入文件头
    *output_size=sizeof(ull)+tree_size+(wpl+7)/8; // 计算压缩文件大小
    uchar *result=new uchar[(*output_size)+1]; // 压缩数据多预留一个字节
    *((ull*)result)=data_size; // 原始数据大小
    memcpy(result+sizeof(ull),encoded_tree,tree_size); // 霍夫曼树
    uchar *compressed=result+(sizeof(ull)+tree_size);
    memset(compressed,0,sizeof(uchar)*((wpl+7)/8+1));
    size_t offset=0,bit=0; //offset:字节偏移量，bit:位偏移量(0<=bit<8)
    // 开始写入压缩数据
    for(size_t i=0;i<data_size;i++){
        ul code=huffcoding[data[i]].code;
        int bitcnt=huffcoding[data[i]].bitcnt;
        while(bitcnt>0){
            ul code8bit=(bitcnt>8 && bit+bitcnt>16)?(code&(255<<(bitcnt-8))):code; //取高8个字节
            auto [low,high]=splitByte(code8bit,bit+bitcnt-8); // 分割为两个字节
            compressed[offset]|=low;compressed[offset+1]|=high; // 写入霍夫曼编码
            bitcnt-=8; // 余下需要处理的bit数
            if(bitcnt>=0){
                offset++;
            }
        }
        bit+=bitcnt&(8-1); // 相当于bitcnt%8，避免bitcnt为负数时%计算出错
        if(bit>=8){
            offset++;bit=bit%8;
        }
    }
    //for(size_t i=0;i<(wpl+7)/8+1;i++)cout<<bitset<8>(compressed[i])<<' ';
    //cout<<endl;
    delete[] encoded_tree;freeTree(tree);
    return result;
}

uchar *decompress(ull data_size, const uchar *data, ull *output_size) {
    // 处理文件头
    *output_size=*((ull *)data); // 解压缩后的大小
    if(*output_size==0){ // 特殊处理空文件
        uchar *res=new uchar[1]; // 哨兵
        return res;
    }
    uchar *result=new uchar[*output_size];
    size_t tree_size;
    HuffmanTree *tree=decodeTree(data+sizeof(ull),&tree_size);
    HuffmanTree *root=tree;
    size_t header_size=sizeof(ull)+tree_size;
    data+=header_size;
    ull cnt=0;

    if(root->isLeave){ // 文件只有一种字符，根节点本身就是叶子结点的特殊情况
        memset(result,root->c,*output_size);
        freeTree(root);return result;
    }
    // 使用自动机处理输入
    for(size_t i=0;i<data_size-sizeof(ull)-tree_size;i++){
        bool completed=false;
        for(int j=7;j>=0;j--){
            int byte=(data[i]&(1<<j))>>j;
            if(byte==0){
                if(tree->left==nullptr){
                    printf("Invalid bit %d at %zu (bit %d)\n",byte,i+header_size,j);
                    freeTree(root);return NULL;
                }
                tree=tree->left;
            }else{
                if(tree->right==nullptr){
                    printf("Invalid bit %d at %zu (bit %d)\n",byte,i+header_size,j);
                    freeTree(root);return NULL;
                }
                tree=tree->right;
            }
            if(tree->isLeave){
                result[cnt++]=tree->c;
                if(cnt>=*output_size){
                    completed=true;break; // 处理完毕，忽略后面的数据
                }
                tree=root;
            }
        }
        if(completed)break;
    }
    if(cnt<*output_size){
        printf("Warning: truncated data (expected %llu, got %llu)\n",*output_size,cnt); // 不返回NULL
        *output_size=cnt;
    }
    freeTree(root);
    return result;
}

bool isFile(const char *filename){
    return access(filename, F_OK) == 0;
}
bool ask_replace(const char *filename){
    //while (getchar() != '\n'); // 清空缓冲区
    printf("\nReplace existing file %s ? (Y/N)",filename);
    string line;getline(cin,line);
    char response=line[0];
    return response=='Y' || response=='y';
}
void lowerstr(char *str){ // 替代strlwr
    size_t i=0;
    while(str[i]!='\0'){
        if(str[i]>='A' && str[i]<='Z')
            str[i]+=0x20;
        i++;
    }
}
void process_file(const char *filename) {
    // 打开文件
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open %s\n",filename);return;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    ull data_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配内存以存储文件内容
    char *data = new char[data_size];
    if (!data) {
        fclose(file);fprintf(stderr, "Memory allocation failed for file data\n");
        return;
    }

    // 读取文件内容
    fread(data, 1, data_size, file);
    fclose(file);

    // 处理压缩或解压缩
    ull output_size,len=strlen(filename);
    char *new_filename,*result = NULL;
    bool is_decompress=false;

    char *lower_filename=new char[len+1]; // 转换为小写
    strcpy(lower_filename,filename);lowerstr(lower_filename);
    if (len>=strlen(EXTENSION) && \
        strcmp(lower_filename + len - strlen(EXTENSION), EXTENSION) == 0) {
        // 解压文件
        is_decompress=true;
        result = (char*)decompress(data_size, (uchar*)data, &output_size);
        new_filename=new char[len-strlen(EXTENSION)+1];
        strncpy(new_filename,filename,len-strlen(EXTENSION));
        new_filename[len-strlen(EXTENSION)]='\0';
        printf("Decompressing to \"%s\" ... ",new_filename);
    } else {
        // 压缩文件
        result = (char*)compress(data_size, (uchar*)data, &output_size);
        new_filename=new char[len+strlen(EXTENSION)+1];
        strcpy(new_filename,filename);
        strcat(new_filename,EXTENSION);
        printf("Compressing to \"%s\" ... ",new_filename);
    }

    if(result==NULL){
        printf("failed\n");
    }else if(!(isFile(new_filename) && !ask_replace(new_filename))){
        FILE *out = fopen(new_filename, "wb");
        if (!out) {
            printf("Failed to open output file %s\n",new_filename);return;
        }
        fwrite(result, sizeof(char), output_size, out);
        double raw=data_size,compressed=output_size;
        if(is_decompress)swap(raw,compressed);
        printf("completed  Compression ratio: %.2f%%\n",compressed/raw*100);
        fclose(out);
    }

    delete[] data;delete[] result;
    delete[] lower_filename;delete[] new_filename;
}

int main(int argc, const char *argv[]) {
    if (argc == 1) {
        printf("Usage: %s file1 file2 ...\n",argv[0]);
        return 1;
    }

    for(int i=1;i<argc;i++)
        process_file(argv[i]);

    return 0;
}