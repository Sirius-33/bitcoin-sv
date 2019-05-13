// Copyright (c) 2019 The Bitcoin SV developers.
// Distributed under the Open BSV software license, see the accompanying file LICENSE.

#ifndef BITCOINSV_CANDIDATES_H
#define BITCOINSV_CANDIDATES_H

#include "primitives/block.h"

#include <atomic>
#include <mutex>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

// Allow candidate IDs to be unique
using MiningCandidateId = boost::uuids::uuid;


/**
 * A mining candidate is a potential block, it is complete apart from the Proof of Work. A mining candidate always has
 * a previous block.
 *
 * Mining candidates can be instantiated by the CMiningCandidateManager.
 *
 * Each mining candidate has an id which identifies the mining candidate. ID's can be compared for equality but should
 * otherwise be treated as opaque. ID's are unique and not re-used for a particular mining candidate manager.
 */
class CMiningCandidate {
    friend class CMiningCandidateManager;
public:
    CBlockRef GetBlock() const { return mBlock; };
    MiningCandidateId GetId() const { return mId; };

private:
    CMiningCandidate(MiningCandidateId id, const CBlockRef& block) : mBlock{block}, mId{id} {}

    CBlockRef mBlock { nullptr };
    MiningCandidateId mId {};
};
using CMiningCandidateRef = std::shared_ptr<CMiningCandidate>;


/**
 * The mining candidate manager owns a collection of mining candidates.
 */
class CMiningCandidateManager {
public:
    CMiningCandidateRef Create(const CBlockRef& block);
    CMiningCandidateRef Get(const MiningCandidateId& candidateId) const;

    void Remove(MiningCandidateId candidateId) {
        std::lock_guard<std::mutex> lock(mMutex);
        mCandidates.erase(candidateId);
    };
    size_t Size() const {
        std::lock_guard<std::mutex> lock(mMutex);
        return mCandidates.size();
    };

    void RemoveOldCandidates();

private:
    mutable std::mutex mMutex {};   // we don't expect much concurrency, a simple exclusive mutex is sufficient

    using CandidateMap = std::map<MiningCandidateId, CMiningCandidateRef>;
    CandidateMap mCandidates {};

    std::atomic_uint mPrevHeight {0};

    boost::uuids::random_generator mIdGenerator {};
};


#endif //BITCOINSV_CANDIDATES_H
