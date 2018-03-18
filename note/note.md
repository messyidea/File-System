整个模拟文件系统的大小是预先固定的。
一个super_node 占用 1024字节， 一个inode占用64字节，一个block占用512字节
上面这些和注释里面写的一样（这里的占用是分配的意思，比如现在一个inode大小比64字节少）

(2 block) (1 block) (512 inode = 64 block) (1024 block)
上面这句是预先设定的文件系统的空间分配

(2 block) 指文件系统的超级块，对应了filsys的结构体。分配了1024字节，即两个块大小

```
struct filsys {
    int s_isize;
    int s_fsize;
    int s_nfree;
    int s_free[100];
    int s_ninode;
    int s_inode[100];
};
```

s_isize 是文件系统的inode的总数，s_fsize 是文件系统的block的总数
s_ninode 表示还有多少inode没有使用，s_nfree表示还有多少block没有使用。
s_free数组是一个块缓冲区，每次分配块的时候(调用balloc函数)先从这里拿，拿完了之后会搜索整个文件系统，把没分配的块的标号放在里面，缓冲区主要是为了加速块的分配。
s_inode数组是一个inode缓冲区，作用和s_free类似。（实际unixV6文件系统里面inode分配也是这样的）
(一般的文件系统的超级块还会放一些类似格式，名字之类的磁盘基本信息，这里省略了)

(1 block) 是一张块表，一共分配了一个块大小。对应used_block结构体。

```
struct used_block {
    int u[32];
};
```

32个int，每个int32位，所以一共32×32=1024位，与后面的1024个block对应。
因为block是纯粹的内容，所以在block里面是无法知道该block有没有被使用，所以需要位表，而inode是结构体，里面有is_used的成员变量，所以不需要位表。

(512 inode = 64 block) 指inode区域，文件系统中每个文件或者文件夹都对应一个inode，inode里面有个i_addr[8]成员变量，这个代表着该inode的内容占用了哪几个块。
系统读取文件的时候，一般先读取inode，然后根据i_addr访问文件的数据。
inode里面还有一个i_mode变量表示文件权限，里面有一个位是预先设定好的big位，如果该位为1，则i_addr的最后一个数据对应的块是一个块的索引(现代文件系统一般有多级索引，这个可以根据需求设计)

(1024 block)指文件系统存放数据的1024个块。

(2 block) (1 block) (512 inode = 64 block) (1024 block)，这一串把那些分配的空间都换算成块了
#define FILESYSTEMSIZE (512 \* (3 + 64 + 1024))
一个块(即block)占用512字节，整个文件系统预设一共需要2+1+64+1024个块，所以FILESYSTEMSIZE就是这么大。

