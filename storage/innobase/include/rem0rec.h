/*****************************************************************************

Copyright (c) 1994, 2019, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/rem0rec.h
 Record manager

 Created 5/30/1994 Heikki Tuuri
 *************************************************************************/

#ifndef rem0rec_h
#define rem0rec_h

#include <innodb/univ/univ.h>

#include <innodb/bit/UT_BITS_IN_BYTES.h>
#include <innodb/data_types/dtuple_get_data_size.h>
#include <innodb/data_types/dtuple_get_info_bits.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/data_types/dtype_t.h>
#include <innodb/machine/data.h>
#include <innodb/memory/ut_memcpy.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_convert_dtuple_to_rec.h>
#include <innodb/record/rec_convert_dtuple_to_rec_comp.h>
#include <innodb/record/rec_convert_dtuple_to_rec_new.h>
#include <innodb/record/rec_convert_dtuple_to_rec_old.h>
#include <innodb/record/rec_convert_dtuple_to_temp.h>
#include <innodb/record/rec_copy_prefix_to_buf.h>
#include <innodb/record/rec_copy_prefix_to_dtuple.h>
#include <innodb/record/rec_fold.h>
#include <innodb/record/rec_get_converted_extra_size.h>
#include <innodb/record/rec_get_converted_size.h>
#include <innodb/record/rec_get_converted_size_comp.h>
#include <innodb/record/rec_get_converted_size_comp_prefix.h>
#include <innodb/record/rec_get_converted_size_comp_prefix_low.h>
#include <innodb/record/rec_get_converted_size_temp.h>
#include <innodb/record/rec_get_info_bits.h>
#include <innodb/record/rec_get_instant_flag_new.h>
#include <innodb/record/rec_get_instant_flag_new_temp.h>
#include <innodb/record/rec_get_instant_offset.h>
#include <innodb/record/rec_get_n_extern_new.h>
#include <innodb/record/rec_get_n_fields.h>
#include <innodb/record/rec_get_n_fields_instant.h>
#include <innodb/record/rec_get_n_fields_length.h>
#include <innodb/record/rec_get_n_fields_old.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_get_nth_field_instant.h>
#include <innodb/record/rec_get_nth_field_old.h>
#include <innodb/record/rec_get_nth_field_old_instant.h>
#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_get_offsets_func.h>
#include <innodb/record/rec_get_offsets_reverse.h>
#include <innodb/record/rec_get_status.h>
#include <innodb/record/rec_get_trx_id.h>
#include <innodb/record/rec_index_print.h>
#include <innodb/record/rec_init_null_and_len_comp.h>
#include <innodb/record/rec_init_null_and_len_temp.h>
#include <innodb/record/rec_init_offsets.h>
#include <innodb/record/rec_init_offsets_comp_ordinary.h>
#include <innodb/record/rec_init_offsets_temp.h>
#include <innodb/record/rec_n_fields_is_sane.h>
#include <innodb/record/rec_offs_any_extern.h>
#include <innodb/record/rec_offs_any_null_extern.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/rec_offs_make_nth_extern.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_offs_nth_extern.h>
#include <innodb/record/rec_print.h>
#include <innodb/record/rec_print.h>
#include <innodb/record/rec_print_mbr_rec.h>
#include <innodb/record/rec_print_new.h>
#include <innodb/record/rec_print_old.h>
#include <innodb/record/rec_printer.h>
#include <innodb/record/rec_set_bit_field_1.h>
#include <innodb/record/rec_set_deleted_flag_new.h>
#include <innodb/record/rec_set_info_bits_new.h>
#include <innodb/record/rec_set_n_fields.h>
#include <innodb/record/rec_set_n_owned_new.h>

#include "btr0types.h"
#include "dict0boot.h"
#include "dict0dict.h"
#include "page0types.h"

#include <ostream>
#include <sstream>



#endif
