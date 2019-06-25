#include <innodb/logger/srv_get_server_errmsgs.h>
#include "sql/derror.h"


const char *srv_get_server_errmsgs(int errcode) {
  return (error_message_for_error_log(errcode));
}
