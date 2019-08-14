#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/TrxTrack.h>
#include <innodb/trx_types/TrxTrackCmp.h>
#include <innodb/allocator/ut_allocator.h>

#include <set>
typedef std::set<TrxTrack, TrxTrackCmp, ut_allocator<TrxTrack>> TrxIdSet;


// #include <unordered_set>
//
// typedef std::unordered_set<TrxTrack, TrxTrackHash, TrxTrackHashCmp> TrxIdSet;
