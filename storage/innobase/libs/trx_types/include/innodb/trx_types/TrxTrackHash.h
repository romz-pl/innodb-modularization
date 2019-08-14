#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/TrxTrack.h>

struct TrxTrackHash {
  size_t operator()(const TrxTrack &key) const { return (size_t(key.m_id)); }
};
