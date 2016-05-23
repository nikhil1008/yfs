// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include "lock_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h> 
#include <jsl_log.h> 
#include <sstream> 

// TODO at some point use const & wherever possible
std::string yfs_client::serialize_dir(std::vector<dirent> dir) {
  std::string res;
  // TODO antipattern? make that more efficient
  // TODO check that name can't contain "/"
  for (auto const &de : dir) {
    res += de.name; 
    res += "/"; 
    res += filename(de.inum); 
    res += "/"; 
  }
  return res;
} 

void yfs_client::deserialize_dir(std::string s, std::vector<dirent> &r){ 
   std::stringstream ss(s);
   std::string item1, item2;
   while (std::getline(ss, item1, '/')) {
       VERIFY(std::getline(ss, item2, '/'));
       r.push_back(dirent(item1, n2i(item2)));
   }
} 

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
  ec = new extent_client(extent_dst);
  srand (time(NULL)); 
  // TODO lock_dst ?

  // create root directory
  const int root_inum = 1;
  VERIFY(ec->put(root_inum, "") == extent_protocol::OK);
}

int yfs_client::create(inum parent, const char *name, inum &file_inum) {
 jsl_log(JSL_DBG_ME, "yfs_client_create\n");
  std::vector<dirent> content;
  read_dir(parent, content);
  for (dirent const &de : content) {
    if (de.name == name) { return -1; }
  }
  file_inum = fresh_inum(false);
  content.push_back(dirent(name, file_inum));
  std::string new_dir = serialize_dir(content);
  VERIFY(ec->put(parent, new_dir) == extent_protocol::OK);
  VERIFY(ec->put(file_inum, "") == extent_protocol::OK);
  return 0;
}

bool yfs_client::lookup(inum parent, const char *name, inum &file_inum) {
 jsl_log(JSL_DBG_ME, "yfs_client_lookup\n");
  std::vector<dirent> content;
  read_dir(parent, content);
  for (dirent const &de : content) {
    if (de.name == name) {
      file_inum = de.inum;
      return true; 
    }
  }
  return false;
}

void yfs_client::read_dir(inum parent, std::vector<dirent> &v) {
   jsl_log(JSL_DBG_ME, "yfs_client_read_dir\n");
   VERIFY(isdir(parent));  
   std::string buf;
   VERIFY(ec->get(parent, buf) == extent_protocol::OK);
   deserialize_dir(buf, v);
}

yfs_client::inum
yfs_client::fresh_inum(bool is_dir)
{
  unsigned int r = rand(); 
  if (is_dir) { r |= 0x80000000; }
  return (inum) r;
}

yfs_client::inum
yfs_client::n2i(std::string n)
{
  std::istringstream ist(n);
  unsigned long long finum;
  ist >> finum;
  return finum;
}

std::string
yfs_client::filename(inum inum)
{
  std::ostringstream ost;
  ost << inum;
  return ost.str();
}

bool
yfs_client::isfile(inum inum)
{
  if(inum & 0x80000000)
    return true;
  return false;
}

bool
yfs_client::isdir(inum inum)
{
  return ! isfile(inum);
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
  int r = OK;
  // You modify this function for Lab 3
  // - hold and release the file lock

  printf("getfile %016llx\n", inum);
  extent_protocol::attr a;
  if (ec->getattr(inum, a) != extent_protocol::OK) {
    r = IOERR;
    goto release;
  }

  fin.atime = a.atime;
  fin.mtime = a.mtime;
  fin.ctime = a.ctime;
  fin.size = a.size;
  printf("getfile %016llx -> sz %llu\n", inum, fin.size);

 release:

  return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
  int r = OK;
  // You modify this function for Lab 3
  // - hold and release the directory lock

  printf("getdir %016llx\n", inum);
  extent_protocol::attr a;
  if (ec->getattr(inum, a) != extent_protocol::OK) {
    r = IOERR;
    goto release;
  }
  din.atime = a.atime;
  din.mtime = a.mtime;
  din.ctime = a.ctime;

 release:
  return r;
}



