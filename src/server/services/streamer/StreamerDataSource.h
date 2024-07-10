#pragma once
#ifndef StreamerDataSource_H_ // Updated ifndef directive
#define StreamerDataSource_H_ // Updated define directive

#include <map>
#include <set> //TODO: move to unordered_set
#include <unordered_map>
#include <vector>
#include "../BaseService.h"
#include "server/services/optimizer/Optimizer.h"

using namespace fts3::optimizer;
using namespace fts3::server;

class StreamerService;

//namespace fts3 {
//namespace streamer {

enum EventType {
    TRANSFER_START,
    TRANSFER_CALLBACK_PM,
    TRANSFER_COMPLETE,
    UNDEFINED
};

struct StreamerPerfMarker {
    EventType eventType;
    std::string src;
    std::string dst;
    std::string jobId;
    uint64_t fileId;
    uint64_t timestamp;
    uint64_t transferred; // in bytes
    uint64_t userFileSize; // in bytes?
    double instantaneousThroughput; // for cross reference purpose
};

struct StreamerPairState {
    int epoch;
    int finishedCount;
    int activeCount;
    int failedCount;
    uint64_t totalDuration;  // in seconds
    uint64_t totalTransferredMB;
    uint64_t totalFileSizeMB;
    uint64_t totalFileSizeSquaredMB;
     
/* Note: Inspired by Optimizer.h's PairState */
    StreamerPairState(): epoch(0), finishedCount(0), activeCount(0), failedCount(0),
        totalDuration(0), totalTransferredMB(0), totalFileSizeMB(0), totalFileSizeSquaredMB(0) {}
    StreamerPairState(int epoch, int finishedCount, int activeCount, int failedCount,
        uint64_t totalDuration, uint64_t totalTransferredMB, uint64_t totalFileSizeMB, uint64_t totalFileSizeSquaredMB):
        epoch(epoch), finishedCount(finishedCount), activeCount(activeCount), failedCount(failedCount),
        totalDuration(totalDuration), totalTransferredMB(totalTransferredMB), totalFileSizeMB(totalFileSizeMB),
        totalFileSizeSquaredMB(totalFileSizeSquaredMB) {}
};

class CyclicPerformanceBuffer {
    /**
     * @class CyclicPerformanceBuffer
     * @brief Represents a cyclic performance buffer for streamer data.
     *
     * This class provides a cyclic performance buffer implementation for storing streamer data.
     * It allows efficient insertion and retrieval of data in a cyclic manner.
     */
    static const uint64_t bucketWidth = 20; // width of time intervals for which statistics are bundled.
    static const uint64_t baseTime = 0;

    int numBuckets;
    uint64_t epochSize;
    // Pairs of <StreamerPairState, Epoch>

protected:
    uint64_t getIndex(uint64_t t) {
        uint64_t epochTime = (t-baseTime)%(epochSize);
        return epochTime/bucketWidth;
    }
    uint64_t getEpoch(uint64_t t) {
        return (t-baseTime)/epochSize;
    }

public:
    std::vector<std::pair<StreamerPairState*, int>> pairStateArray;

    CyclicPerformanceBuffer(int nb): numBuckets(nb), epochSize(nb*bucketWidth) {
        pairStateArray.resize(numBuckets); // Resize the vector to the specified number of buckets
        for (int i = 0; i < numBuckets; i++) {
            pairStateArray[i] = std::make_pair(new StreamerPairState(), 0); // Construct an empty StreamerPairState at each index
        }
    }

    // For use by the Streamer Service (Southbound)
    StreamerPairState* getStreamerState(uint64_t t) {
        return pairStateArray[getIndex(t)].first;
    }

    // For use by the Streamer Data Source (Northbound)
};

struct StreamerFileState {
    uint64_t lastTimestamp;
    uint64_t lastTransferredBytes;
};

class StreamerDataSource: public OptimizerDataSource {
public:
    //TODO: Need to make the following private
    static const uint64_t T = 17*1000; // Period of time in milliseconds.

    uint64_t t0 = 0; // Timestamp in milliseconds after Epoch Time.
    std::map<Pair, StreamerPairState> m_sd;  
    std::map<std::string, StreamerFileState> m_sdf; // Maps the concatenated src+dst+jobid+fileid to its State.
    std::set<Pair> s_activePairs;
    CyclicPerformanceBuffer cyclicPerfBuffer; // a cyclic buffer saving array of StreamerPairState
    int numPM;

//public:
    StreamerDataSource();
    ~StreamerDataSource();
    friend class StreamerService;

    // Return a list of pairs with active or submitted transfers
    std::list<Pair> getActivePairs(void);

    // Return the optimizer configuration value
    OptimizerMode getOptimizerMode(const std::string &source, const std::string &dest);

    // Get configured limits
    void getPairLimits(const Pair &pair, Range *range, StorageLimits *limits);

    // Get the stored optimizer value (current value)
    int getOptimizerValue(const Pair& pair);

    // Get the weighted throughput for the pair
    void getThroughputInfo(const Pair &pair, const boost::posix_time::time_duration &interval,
        double *throughput, double *filesizeAvg, double *filesizeStdDev);

    time_t getAverageDuration(const Pair &pair, const boost::posix_time::time_duration &interval);

    // Get the success rate for the pair
    double getSuccessRateForPair(const Pair &pair, const boost::posix_time::time_duration &interval,
        int *retryCount);

    // Get the number of transfers in the given state
    int getActive(const Pair &pair);
    int getSubmitted(const Pair &pair);

    // Get current throughput
    double getThroughputAsSource(const std::string &se);
    double getThroughputAsDestination(const std::string &se);

    // Permanently register the optimizer decision
    void storeOptimizerDecision(const Pair &pair, int activeDecision,
        const PairState &newState, int diff, const std::string &rationale);
    // Permanently register the number of streams per active
    void storeOptimizerStreams(const Pair &pair, int streams);
};
//}
//}
#endif // StreamerDataSource_H_ // Updated endif directive