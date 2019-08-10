#pragma once

#include <innodb/univ/univ.h>

#include "mysql/components/services/psi_stage_bits.h"

/** Performance schema stage event for monitoring clone file copy progress. */
extern PSI_stage_info srv_stage_clone_file_copy;

/** Performance schema stage event for monitoring clone redo copy progress. */
extern PSI_stage_info srv_stage_clone_redo_copy;

/** Performance schema stage event for monitoring clone page copy progress. */
extern PSI_stage_info srv_stage_clone_page_copy;
