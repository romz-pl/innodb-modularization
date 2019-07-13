#include <innodb/logger/srv_get_server_errmsgs.h>

#ifndef UNIV_NO_ERR_MSGS

#include "sql/derror.h"

const char *srv_get_server_errmsgs(int errcode) {
  return (error_message_for_error_log(errcode));
}


#endif /* !UNIV_NO_ERR_MSGS */
