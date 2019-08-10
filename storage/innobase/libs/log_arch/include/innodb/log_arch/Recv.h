#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Dblwr_Ctx.h>
#include <innodb/log_arch/Arch_Dir_Group_Info_Map.h>
#include <innodb/log_arch/Arch_Page_Sys.h>

/** Recovery system data structure for the archiver. */
class Arch_Page_Sys::Recv {
 public:
  /** Constructor: Initialize members */
  Recv(const char *dir_name) : m_arch_dir_name(dir_name) {}

  /** Destructor: Close open file and free resources */
  ~Recv() {}

  /** Initialise the archiver's recovery system.
  @return error code. */
  dberr_t init();

  /** Scan the archive directory and fetch all the group directories.
  @return true if the scan was successful. */
  bool scan_group();

#ifdef UNIV_DEBUG
  /** Print information related to the archiver recovery system added
  for debugging purposes. */
  void print();
#endif

  /** Parse for group information and fill the group.
  @param[in,out]	page_sys	global dirty page archive system
  @return error code. */
  dberr_t fill_info(Arch_Page_Sys *page_sys);

  /** Disable copy construction */
  Recv(Recv const &) = delete;

  /** Disable assignment */
  Recv &operator=(Recv const &) = delete;

 private:
  /** Read all the group directories and store information related to them
  required for parsing.
  @param[in]	file_path	file path information */
  void read_group_dirs(const std::string file_path);

  /** Read all the archived files belonging to a group and store information
  related to them required for parsing.
  @param[in]	dir_path	dir path information
  @param[in]	file_path	file path information */
  void read_group_files(const std::string dir_path,
                        const std::string file_path);

 private:
  /** Archive directory. */
  std::string m_arch_dir_name;

  /** Doublewrite buffer context. */
  Arch_Dblwr_Ctx m_dblwr_ctx{};

  /** Mapping of group directory names and group information related to
  the group. */
  Arch_Dir_Group_Info_Map m_dir_group_info_map{};
};
