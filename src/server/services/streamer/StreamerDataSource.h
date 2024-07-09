#pragma once
#ifndef StreamerDataSource_H_ // Updated ifndef directive
#define StreamerDataSource_H_ // Updated define directive

#include <map>
#include <unordered_map>
#include "../BaseService.h"
#include "server/services/optimizer/Optimizer.h"

using namespace fts3::optimizer;
using namespace fts3::server;

class StreamerService;

//namespace fts3 {
//namespace streamer {

struct StreamerPairState {
    int totalDuration;
    int finishedCount;
    int activeCount;
    int failedCount;
    long long totalTransferredMB;
    long long totalFileSizeMB;
    long long totalFileSizeSquaredMB;
     
/*
    time_t timestamp;
    double throughput;
    time_t avgDuration;
    double successRate;
    int retryCount;
    int activeCount;
    int queueSize;
    // Exponential Moving Average
    double ema;
    // Filesize statistics
    double filesizeAvg;
    double filesizeStdDev;
    // Optimizer last decision
    int connections;
*/
    StreamerPairState(): totalDuration(0), finishedCount(0), activeCount(0), failedCount(0),
        totalTransferredMB(0), totalFileSizeMB(0), totalFileSizeSquaredMB(0) {}
    StreamerPairState(int totalDuration, int finishedCount, int activeCount, int failedCount,
        long long totalTransferredMB, long long totalFileSizeMB, long long totalFileSizeSquaredMB):
        totalDuration(totalDuration), finishedCount(finishedCount), activeCount(activeCount),
        failedCount(failedCount), totalTransferredMB(totalTransferredMB), totalFileSizeMB(totalFileSizeMB),
        totalFileSizeSquaredMB(totalFileSizeSquaredMB) {}
};

class StreamerDataSource: public OptimizerDataSource {
public:
    //TODO: Need to make the following private
    // std::stirng src, dst;
    time_t t0;
    std::map<Pair, StreamerPairState> m_sd;  
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