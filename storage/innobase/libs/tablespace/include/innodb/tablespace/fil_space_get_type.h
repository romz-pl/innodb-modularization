#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

fil_type_t fil_space_get_type(space_id_t space_id);

#endif
#endif
