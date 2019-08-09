#include <innodb/log_arch/Arch_Reset_File.h>


void Arch_Reset_File::init() {
  m_file_index = 0;
  m_lsn = LSN_MAX;
  m_start_point.clear();
}
