#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <list>

class Arch_Group;

/** A list of archive groups */
using Arch_Grp_List = std::list<Arch_Group *, ut_allocator<Arch_Group *>>;
