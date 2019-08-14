#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/TrxTrack.h>

/**
Comparator for TrxMap */
struct TrxTrackCmp {
  bool operator()(const TrxTrack &lhs, const TrxTrack &rhs) const {
    return (lhs.m_id < rhs.m_id);
  }
};
