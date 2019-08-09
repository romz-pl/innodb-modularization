#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_arch/Arch_Reset_File.h>

#include <deque>

/* Structure representing list of archived files. */
using Arch_Reset = std::deque<Arch_Reset_File>;
