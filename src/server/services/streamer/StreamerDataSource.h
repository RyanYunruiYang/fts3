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
class CyclicPerformanceBuffer;

enum EventType {
    TRANSFER_START,
    TRANSFER_CALLBACK_PM,
    TRANSFER_COMPLETE,
    UNDEFINED
};

// Used for messages only (sending performance markers over ZMQ).
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


class StreamerDataSource: public OptimizerDataSource {
public:
    static const uint64_t T = 17*1000; // Period of time in milliseconds.

    uint64_t t0 = 0; // Timestamp in milliseconds after Epoch Time.
    // std::map<Pair, StreamerPairState> m_sd;  // stat from the service start time (no expiration), answer query getActive, getSubmitted
    // std::map<Pair, StreamerTransientStateStats> m_sds; //transient stat for last k intervals, answer query getThroughputInfo (filesize statistics), getAverageDuration
    std::map<Pair, CyclicPerformanceBuffer> pairToCyclicBuffer; // a cyclic buffer saving array of PerformanceInterval
    // std::map<Pair, std::map<uint64_t, StreamerFileState>> m_sdf; // Maps the pair to a map of fileid to its State., file specific stat
    std::set<Pair> s_activePairs; // current active (src, dst)
    int numPM; // number of PM received

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