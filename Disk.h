#pragma once
//#include "XX_filesystem.h"
#include "ACI.h"
#include "Directory.h"
//#include "FileSystem.h"
// Block
class Block
{
private:
	int No; // 块号

public:
	Block(int No);

	// 格式化块
	virtual void format() = 0;

	// 打印块信息
	virtual void printInfo() = 0;

	// 块接口
	int getNO(); // 获取块号

	virtual void block_read(FILE* fpr) = 0; // 读入块内数据
	virtual void block_write(FILE* fpw) = 0; // 写入磁盘文件
};

// 数据块
class DataBlock :
	public Block
{
private:
	// 虚拟磁盘文件缓存,用于写回在写文件方式打开时被清空的文件
	char* buffer;
public:
	DataBlock();
	~DataBlock();
	
	// 格式化数据块
	virtual void format();

	// 打印数据块信息
	virtual void printInfo(); // TODO

	// 数据块接口
	char* buffer_return(); // 返回指向数据块数据的指针
	void buffer_write(char buf[Block_Num * BLOCK_SIZE]); // 根据传入的数组改写数据块的数据

	virtual void block_read(FILE* fpr); // 读入数据块
	virtual void block_write(FILE* fpw); // 写入磁盘文件

};

class Superblock :
	public Block
{
private:
	const int s_Inode_Num = INODE_NUM; // 总Inode数
	int free_Inode_Num; // 空闲Inode数
	const int s_Block_Num = Block_Num; // 总Block数
	int free_Block_Num; // 空闲Block数

public:
	Superblock(int free_INum, int free_BNum);
	Superblock();

	// 格式化Superblock
	virtual void format();

	// 打印Superblock信息
	virtual void printInfo();

	// Superblock接口
	virtual void block_read(FILE* fpr); // 读入Superblock
	virtual void block_write(FILE* fpw); // 写入磁盘文件

	void use_renew();
};

class Block_Bitmap :
	public Block
{
private:
	bool b_isUsed[Block_Num]; // Block占用情况
public:
	Block_Bitmap(bool isUsed[Block_Num]);
	Block_Bitmap();

	// 格式化Block位图
	virtual void format();

	// 打印Block占用情况
	virtual void printInfo();

	// Block_Bitmap接口
	virtual void block_read(FILE* fpr); // 读入Block_Bitmap
	virtual void block_write(FILE* fpw); // 写入磁盘文件

	int balloc();

	void use_renew(int b_index);
};

class Inode_Bitmap :
	public Block
{
private:
	bool i_isUsed[INODE_NUM]; // Inode占用情况
public:
	Inode_Bitmap(bool isUsed[INODE_NUM]);
	Inode_Bitmap();

	// 格式化Inode位图
	virtual void format();

	// 打印Inode占用情况
	virtual void printInfo();

	// Inode_Bitmap接口
	virtual void block_read(FILE* fpr); // 读入Inode_Bitmap
	virtual void block_write(FILE* fpw); // 写入磁盘文件

	int ialloc();

	void use_renew(int i_index);
};

class Inode
{
private:
	int i_No; // Inode编号
	int f_mode; // 文件类型
	int f_size; // 文件大小
	int c_time; // 创建时间
	int block_index[BLOCK_INDEX]; // Inode指向的块号序列

public:
	Inode(int No, int mode, int size, int time_c, int index[BLOCK_INDEX]);
	Inode(int No);
	Inode();

	// 格式化Inode
	void format();

	// 打印该Inode信息
	void printInfo();

	// Inode接口
	int getI_No(); // 获得Inode编号
	void setI_No(int No); // 设置Inode编号

	int getF_Mode(); // 获得文件类型
	void setF_Mode(int mode); // 设置文件类型

	int getF_Size(); // 获得文件大小
	void setF_Size(int size); // 设置文件大小

	int getC_Time(); // 获得创建时间
	void setC_Time(int time); // 设置创建时间

	bool addBlock(int index); // 增加可用块序号

	char* getFile(char* buffer); // 返回该Inode指向的文件

	int getIndex(int No); // 返回第No号块的块号
};

// Inode表
class Inode_Label :
	public Block
{
private:
	Inode inode[INODE_NUM];
public:
	Inode_Label();
	//~Inode_Label();

	// 格式化Inode表
	virtual void format();

	// 打印Inode表信息
	virtual void printInfo(); /*TODO*/

	// Inode_Label接口
	virtual void block_read(FILE* fpr); // 读入Inode_Label
	virtual void block_write(FILE* fpw); // 写入磁盘文件

	Inode* getInode(int No); // 返回第No号Inode

	void use_renew(int i_index, int b_index, int mode, int f_size, Directory dir);
};

// Disk单元
class Disk
{
private:
	Superblock spb; // Disk单元中的Superblock
	Block_Bitmap b_bmap; // Disk单元中的Block位图
	Inode_Bitmap i_bmap; // Disk单元中的Inode位图
	Inode_Label i_label; // Disk单元中的Inode表
	DataBlock d_block; // 数据块缓存

public:

	void format(Block& b); // Disk单元内各块区格式化
	void disk_format(); // Disk单元格式化

	/*
	* Disk单元接口
	*/ 

	// 将磁盘文件读入Disk单元
	
	void block_read(Block& b, FILE* fpr); // Disk单元内各块区数据读入
	void disk_read(FILE* fpr); // Disk单元读入
		
	// 将Disk单元存入磁盘文件
	
	void block_write(Block& b, FILE* fpw); // Disk单元内各块区数据写出
	void disk_write(FILE* fpw); // // Disk单元写出

	// 返回位图情况
	int d_balloc();
	int d_ialloc();

	// Disk与目录的接口
	Dentry dentry_read(int dentry_address); // 从数据块读取目录项
	Dentry dentry_read(int dentry_address, char* buf); // 从数据块读取目录项
	void dentry_write(int dentry_address, Dentry dentry); // 把目录项写入数据块
	void dentry_write(int dentry_address, char* buf, Dentry dentry); // 把目录项写入数据块

	Directory dir_read(int i_No); // 从buffer读取目录 TEST
	void dir_write(int i_No, Directory dir); // 把目录写入buffer TEST

	// 占用空间时更新Disk
	void use_renew(int b_index, int i_index, int mode, int f_size, Directory dir);
};