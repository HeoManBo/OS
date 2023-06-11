// On-disk file system format.
// Both the kernel and user programs use this header file.


#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

// Disk layout:
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]
//
// mkfs computes the super block and builds an initial file system. The
// super block describes the disk layout:
struct superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
  uint logstart;     // Block number of first log block
  uint inodestart;   // Block number of first inode block
  uint bmapstart;    // Block number of first free map block
};

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)
//20182607
#define MAX_BLOCK_PER_ADDR 255
//#define MAX_BLOCK_PER_ADDR 255 //하나의 addr당 255개의 블락 저장 가능 
#define CS_MAXFILE (NDIRECT * MAX_BLOCK_PER_ADDR) //CS 타입의 파일이 가질 수 있는 전체 블럭수
// On-disk inode structure
struct dinode {
  short type;           // File type
  short major;          // Major device number (T_DEV only)
  short minor;          // Minor device number (T_DEV only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT+1];   // Data block addresses 
};

// Inodes per block.
// 디스크 블락당 아이노드 수
#define IPB           (BSIZE / sizeof(struct dinode))

// 블락당 비트맵의 비트 수
// Block containing inode i
#define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)

// Bitmap bits per block
// 하나의 비트 데이터 블락은 4096개를 표시할 수 있음
#define BPB           (BSIZE*8)

// 비트 데이터 블락에서 몇 번째 비트맵 블락인가?
// Block of free map containing bit for block b
#define BBLOCK(b, sb) (b/BPB + sb.bmapstart)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};

