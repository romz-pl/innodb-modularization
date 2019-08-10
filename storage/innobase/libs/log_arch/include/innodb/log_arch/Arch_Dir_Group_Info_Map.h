#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Recv_Group_Info.h>

#include <map>
#include <string>

/** Mapping of group directory name to information related to the group. */
using Arch_Dir_Group_Info_Map = std::map<std::string, Arch_Recv_Group_Info>;
