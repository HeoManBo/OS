//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  if(argint(n, &fd) < 0)
    return -1;
  if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *curproc = myproc();

  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd] == 0){
      curproc->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

int
sys_dup(void)
{
  struct file *f;
  int fd;

  if(argfd(0, 0, &f) < 0)
    return -1;
  if((fd=fdalloc(f)) < 0)
    return -1;
  filedup(f);
  return fd;
}

int
sys_read(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

int
sys_write(void)
{
  struct file *f;
  int n;
  char *p;

  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return filewrite(f, p, n);
}

int
sys_close(void)
{
  int fd;
  struct file *f;

  if(argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

int
sys_fstat(void)
{
  struct file *f;
  struct stat *st;

  if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
    return -1;
  return filestat(f, st);
}

// Create the path new as a link to the same inode as old.
int
sys_link(void)
{
  char name[DIRSIZ], *new, *old;
  struct inode *dp, *ip;

  if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
    return -1;

  begin_op();
  if((ip = namei(old)) == 0){
    end_op();
    return -1;
  }

  ilock(ip);
  if(ip->type == T_DIR){
    iunlockput(ip);
    end_op();
    return -1;
  }
  if(ip->type == T_CS){
	  cprintf("T_CS 타입의 파일에선 심볼릭 링크르 생성할 수 없습니다..!\n");
	  iunlockput(ip);
	  end_op();
	  return -1;
  }

  ip->nlink++;
  iupdate(ip);
  iunlock(ip);

  if((dp = nameiparent(new, name)) == 0)
    goto bad;
  ilock(dp);
  if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
    iunlockput(dp);
    goto bad;
  }
  iunlockput(dp);
  iput(ip);

  end_op();

  return 0;

bad:
  ilock(ip);
  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);
  end_op();
  return -1;
}

// Is the directory dp empty except for "." and ".." ?
static int
isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
    if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if(de.inum != 0)
      return 0;
  }
  return 1;
}

//PAGEBREAK!
int
sys_unlink(void)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], *path;
  uint off;

  if(argstr(0, &path) < 0)
    return -1;

  begin_op();
  if((dp = nameiparent(path, name)) == 0){
    end_op();
    return -1;
  }

  ilock(dp);

  // Cannot unlink "." or "..".
  if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
    goto bad;

  if((ip = dirlookup(dp, name, &off)) == 0)
    goto bad;
  ilock(ip);

  if(ip->nlink < 1)
    panic("unlink: nlink < 1");
  if(ip->type == T_DIR && !isdirempty(ip)){
    iunlockput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));
  if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
  if(ip->type == T_DIR){
    dp->nlink--;
    iupdate(dp);
  }
  iunlockput(dp);

  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);

  end_op();

  return 0;

bad:
  iunlockput(dp);
  end_op();
  return -1;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];

  //인자로 넘어오는 path에 대한 디렉토리를 찾고
  //없다면 0을 리턴함. -> 디렉토리가 없다는 뜻.
  if((dp = nameiparent(path, name)) == 0)
    return 0;
  ilock(dp);

  //이미 존재하는 동일한 파일 이름에 대하여
  //또 create하는 경우 
  //dirlookup의 리턴 값 != 0 --> 해당 존재하는 파일의 아이노드 구조체를
  //디스크에서 꺼내옴.
  //dirlookup == 0 -> 해당 이름에 존재하는 파일이 디렉도리에 없음. 
  //만약 T_FILE 형태라면 해당 아이노드를 리턴함.
  if((ip = dirlookup(dp, name, 0)) != 0){
    iunlockput(dp);
    ilock(ip);
	//20182607..
	//이미 존재하는 아이노드 구조체 반환
	if(type == T_CS && ip->type == T_CS){
		return ip;
	}
	else if(type == T_FILE && ip->type == T_FILE)
      return ip;
    iunlockput(ip);
    return 0;
  }

  //디스크에 비어있는 아이노드 구조체를 반환하는 함수
  //만약 0이라면 --> dev 디스크의 모든 아이노드 구조체가 파일을 가리키고있음
  //즉 생성할 수 있는 파일의 개수를 넘어섬
  //그렇기에 panic;
  if((ip = ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  
  //inode 상태 업데이트 --> 즉 해당 위치에 아이노드 구조체 저장. 
  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  //만들려는게 디렉토리인 경우..
  if(type == T_DIR){  // Create . and .. entries.
    dp->nlink++;  // for ".."
    iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  //현재 디렉토리에 새로운 파일(디렉토리든 뭐든) 추가 
  if(dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");

  //디렉토리 락 해제 
  iunlockput(dp);

  //생성한 아이노드 구조체 반환 
  return ip;
}

int
sys_open(void)
{
  char *path;
  int fd, omode;
  struct file *f;
  struct inode *ip;

  if(argstr(0, &path) < 0 || argint(1, &omode) < 0)
    return -1;

  begin_op();

  //이름에 해당하는 파일 아이노드 구조체를 가져오는 작업..
  if(omode & O_CREATE){
	  //20182607 O_CS 플래그 옵션인 경우
	  //파일의 타입을 T_CS로 저장한다. 
	  if(omode & O_CS){
		  ip = create(path, T_CS, 0,0);
	  }
	  else{
    	ip = create(path, T_FILE, 0, 0);
	  }
    //아이노드 구조체를 할당받지 못한경우 -1로 종료.. 즉 파일을 만들지 못함...
    if(ip == 0){
      end_op();
      return -1;
    }
   }else { //O_CREAT flag 옵션이 없는 경우.
	//해당 경로에 인자로 넘어온 이름이 없는 경우 -1리턴..
    if((ip = namei(path)) == 0){
      end_op();
      return -1;
    }
    ilock(ip);
	//디렉토리인데 RD 옵션을 안준경우... -1리턴..
    if(ip->type == T_DIR && omode != O_RDONLY){
      iunlockput(ip);
      end_op();
      return -1;
    }
  }
  //현재 프로세스에 비어있는 파일 테이블 자리에
  //해당 아이노드를 배정하고 파일 테이블에 배정한 번호를 리턴한다.
  //파일 구조체 할당 
  //파일 테이블을 뒤져 할당할 수 없거나... 
  //현재 프로세스에서 열고 있는 파일의 수를 넘어서는 경우...
  //-1 리턴...
  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  end_op();

  //파일은 INODE이고..
  //해당 파일이 가리키는 아이노드 ip는 
  //이전에 연 아이노드를 가리키게함..
  //시작 offset == 0..
  //플래그에 맞게 Read, write 권한 설정...
  f->type = FD_INODE;
  f->ip = ip;
  f->off = 0;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
  return fd;
}

int
sys_mkdir(void)
{
  char *path;
  struct inode *ip;

  begin_op();
  if(argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_mknod(void)
{
  struct inode *ip;
  char *path;
  int major, minor;

  begin_op();
  if((argstr(0, &path)) < 0 ||
     argint(1, &major) < 0 ||
     argint(2, &minor) < 0 ||
     (ip = create(path, T_DEV, major, minor)) == 0){
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

int
sys_chdir(void)
{
  char *path;
  struct inode *ip;
  struct proc *curproc = myproc();
  
  begin_op();
  if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);
  if(ip->type != T_DIR){
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  iput(curproc->cwd);
  end_op();
  curproc->cwd = ip;
  return 0;
}

int
sys_exec(void)
{
  char *path, *argv[MAXARG];
  int i;
  uint uargv, uarg;

  if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv))
      return -1;
    if(fetchint(uargv+4*i, (int*)&uarg) < 0)
      return -1;
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    if(fetchstr(uarg, &argv[i]) < 0)
      return -1;
  }
  return exec(path, argv);
}

int
sys_pipe(void)
{
  int *fd;
  struct file *rf, *wf;
  int fd0, fd1;

  if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
    return -1;
  if(pipealloc(&rf, &wf) < 0)
    return -1;
  fd0 = -1;
  if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
    if(fd0 >= 0)
      myproc()->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  fd[0] = fd0;
  fd[1] = fd1;
  return 0;
}

//printinfo함수 구현
int 
sys_printinfo(void)
{
	char* filename = 0;
	struct file* f=0;
	//파일 이름 가져온다
	if(argstr(1, &filename) < 0){
		cprintf("cant get filename..\n");
		return -1;
	}
	//파일 이름에 대응하는 파일 구조체 정보 얻어옴
	if(argfd(0,0,&f) < -1){
		cprintf("cant get file struct..\n");
		return -1;
	}
	if(filename != 0 && f != 0){
		cprintf("FILE NAME: %s\n", filename);
		cprintf("INODE NUM : %d\n", f->ip->inum);
		if(f->ip->type != T_CS){
			cprintf("FILE TYPE: FILE\n");
			cprintf("FILE SIZE: %d\n", f->ip->size);
			int flag=0;
			for(int i=0; i<NDIRECT; i++){
				if(flag == 0){
					cprintf("DIRECT BLOCK INFO: \n");
					flag=1;
				}
				if(f->ip->addrs[i] != 0){
					cprintf("[%d] %d\n", i, f->ip->addrs[i]);
				}
				else if(f->ip->addrs[i] == 0){
					break;
				}
			}
		}
		else if(f->ip->type == T_CS){
			cprintf("FILE TYPE: CS\n");
			cprintf("FILE SIZE : %d\n", f->ip->size);
			int flag=0;
			for(int i=0; i<NDIRECT; i++){
				if((f->ip->addrs[i] >> 8) != 0){
					if(flag == 0){
						cprintf("DIRECT BLOCK INFO: \n");
						flag = 1;
					}
					int x = f->ip->addrs[i] & 0x000000ff;
					int total_size = (f->ip->addrs[i] & 0xffffff00) + x;
					cprintf("[%d] %d (num : %d, length : %d)\n",i, total_size, f->ip->addrs[i] >> 8, (f->ip->addrs[i] & 0x000000ff));
				}
				else if((f->ip->addrs[i] >> 8) == 0){
					break;
				}
			}
		}
		cprintf("\n");
	}
	return 0;
}
