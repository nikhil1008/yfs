#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <jsl_log.h>

extent_client::extent_client(const std::string &dst) {
  sockaddr_in dstsock;
  make_sockaddr(dst.c_str(), &dstsock);
  cl = std::unique_ptr<rpcc>(new rpcc(dstsock));
  VERIFY(cl->bind() == 0);
}

extent_protocol::status extent_client::get(extent_protocol::extentid_t eid, 
                                           std::string &buf) {
  jsl_log(JSL_DBG_ME, "extent_client: get %llu\n", eid);
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::get, eid, buf);
  jsl_log(JSL_DBG_ME, "extent_client: get %llu buf = %s\n", eid, buf.c_str());
  return ret;
}

extent_protocol::status extent_client::getattr(extent_protocol::extentid_t eid, 
		                                           extent_protocol::attr &attr) {
  jsl_log(JSL_DBG_ME, "extent_client: getattr %llu\n", eid);
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::getattr, eid, attr);
  return ret;
}

extent_protocol::status extent_client::put(extent_protocol::extentid_t eid, 
                                           std::string buf) {
  jsl_log(JSL_DBG_ME, "extent_client: put %llu %s\n", eid, buf.c_str());
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::put, eid, buf, r);
  return ret;
}

extent_protocol::status extent_client::remove(extent_protocol::extentid_t eid)
{
  jsl_log(JSL_DBG_ME, "extent_client: remove %llu\n", eid);
  extent_protocol::status ret = extent_protocol::OK;
  int r;
  ret = cl->call(extent_protocol::remove, eid, r);
  return ret;
}