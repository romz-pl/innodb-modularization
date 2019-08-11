/*****************************************************************************

Copyright (c) 2017, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file clone/clone0snapshot.cc
 Innodb physical Snaphot

 *******************************************************/

#include <innodb/mtr/mtr_start.h>
#include <innodb/mtr/mtr_commit.h>
#include <innodb/io/IORequest.h>
#include <innodb/page/page_zip_set_size.h>
#include <innodb/clone/Clone_Snapshot.h>
#include <innodb/tablespace/fil_space_get.h>
#include <innodb/buf_pool/Page_fetch.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/log_types/log_get_lsn.h>
#include <innodb/buf_page/buf_page_mutex_exit.h>
#include <innodb/buf_page/buf_page_mutex_enter.h>

#include "buf0buf.h"
#include "fil0fil.h"
#include "buf0flu.h"



int Clone_Snapshot::get_page_for_write(const page_id_t &page_id,
                                       const page_size_t &page_size,
                                       byte *&page_data, uint &data_size) {
  auto space = fil_space_get(page_id.space());
  IORequest request(IORequest::WRITE);

  mtr_t mtr;
  mtr_start(&mtr);

  ut_ad(data_size >= 2 * page_size.physical());

  data_size = page_size.physical();
  auto encrypted_data = page_data + data_size;

  /* Space header page is modified with SX latch while extending. Also,
  we would like to serialize with page flush to disk. */
  auto block =
      buf_page_get_gen(page_id, page_size, RW_SX_LATCH, nullptr,
                       Page_fetch::POSSIBLY_FREED, __FILE__, __LINE__, &mtr);
  auto bpage = &block->page;

  byte *src_data;

  if (bpage->zip.data != nullptr) {
    ut_ad(bpage->size.is_compressed());
    src_data = bpage->zip.data;
  } else {
    ut_ad(!bpage->size.is_compressed());
    src_data = block->frame;
  }

  memcpy(page_data, src_data, data_size);

  auto cur_lsn = log_get_lsn(*log_sys);
  const auto frame_lsn =
      static_cast<lsn_t>(mach_read_from_8(page_data + FIL_PAGE_LSN));

  buf_page_mutex_enter(block);
  ut_ad(!fsp_is_checksum_disabled(bpage->id.space()));
  /* Get oldest and newest page modification LSN for dirty page. */
  auto oldest_lsn = bpage->oldest_modification;
  auto newest_lsn = bpage->newest_modification;
  buf_page_mutex_exit(block);

  /* If the page is not dirty but frame LSN is zero, it could be half
  initialized page left from incomplete operation. Assign valid LSN and checksum
  before copy. */
  if (frame_lsn == 0 && oldest_lsn == 0) {
    oldest_lsn = cur_lsn;
    newest_lsn = cur_lsn;
  }

  /* If page is dirty, we need to set checksum and page LSN. */
  if (oldest_lsn > 0) {
    ut_ad(newest_lsn > 0);
    /* For compressed table, must copy the compressed page. */
    if (page_size.is_compressed()) {
      page_zip_des_t page_zip;

      page_zip_set_size(&page_zip, data_size);
      page_zip.data = page_data;
#ifdef UNIV_DEBUG
      page_zip.m_start =
#endif /* UNIV_DEBUG */
          page_zip.m_end = page_zip.m_nonempty = page_zip.n_blobs = 0;

      buf_flush_init_for_writing(nullptr, block->frame, &page_zip, newest_lsn,
                                 false, false);
    } else {
      buf_flush_init_for_writing(nullptr, page_data, nullptr, newest_lsn, false,
                                 false);
    }
  }

  BlockReporter reporter(false, page_data, page_size, false);

  const auto page_lsn =
      static_cast<lsn_t>(mach_read_from_8(page_data + FIL_PAGE_LSN));

  const auto page_checksum = static_cast<uint32_t>(
      mach_read_from_4(page_data + FIL_PAGE_SPACE_OR_CHKSUM));

  int err = 0;

  if (reporter.is_corrupted() || page_lsn > cur_lsn ||
      (page_checksum != 0 && page_lsn == 0)) {
    ut_ad(false);
    my_error(ER_INTERNAL_ERROR, MYF(0), "Innodb Clone Corrupt Page");
    err = ER_INTERNAL_ERROR;
  }

  fil_io_set_encryption(request, page_id, space);

  /* Encrypt page if TDE is enabled. */
  if (err == 0 && request.is_encrypted()) {
    Encryption encryption(request.encryption_algorithm());
    ulint data_len;
    byte *ret_data;

    data_len = data_size;

    ret_data = encryption.encrypt(request, page_data, data_size, encrypted_data,
                                  &data_len);
    if (ret_data != page_data) {
      page_data = encrypted_data;
      data_size = static_cast<uint>(data_len);
    }
  }

  /* NOTE: We don't do transparent compression (TDC) here as punch hole
  support may not be there on remote. Also, punching hole for every page
  in remote during clone could be expensive. */

  mtr_commit(&mtr);
  return (err);
}
