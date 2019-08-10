#include <innodb/clone/pfs.h>

#include "include/mysql/psi/psi_base.h"

/** Performance schema stage event for monitoring clone file copy progress. */
PSI_stage_info srv_stage_clone_file_copy = {
    0, "clone (file copy)", PSI_FLAG_STAGE_PROGRESS, PSI_DOCUMENT_ME};

/** Performance schema stage event for monitoring clone redo copy progress. */
PSI_stage_info srv_stage_clone_redo_copy = {
    0, "clone (redo copy)", PSI_FLAG_STAGE_PROGRESS, PSI_DOCUMENT_ME};

/** Performance schema stage event for monitoring clone page copy progress. */
PSI_stage_info srv_stage_clone_page_copy = {
    0, "clone (page copy)", PSI_FLAG_STAGE_PROGRESS, PSI_DOCUMENT_ME};
