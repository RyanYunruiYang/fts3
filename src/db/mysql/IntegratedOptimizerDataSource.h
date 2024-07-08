#pragma once

// #include <numeric>
// #include "MySqlAPI.h"
// #include "common/Exceptions.h"
// #include "common/Logger.h"
// #include "sociConversions.h"

// #include "db/generic/DbUtils.h"
#include "server/services/optimizer/Optimizer.h"
#include "server/services/streamer/StreamerDataSource.h"
#include "MySqlOptimizerDataSource.h"

// using namespace db;
using namespace fts3;
using namespace fts3::common;
using namespace fts3::optimizer;

// using namespace fts3::common;


class IntegratedOptimizerDataSource: public OptimizerDataSource {
protected:
    OptimizerDataSource *mySqlData; // assume mySql as default, streamData is a cache in front for optimization.
    StreamerDataSource *streamData;

private:
    IntegratedOptimizerDataSource() {};

public:
    // connectionPool
    IntegratedOptimizerDataSource(OptimizerDataSource *mySqlData, StreamerDataSource *streamData);

    ~IntegratedOptimizerDataSource() {
        // RY: Assume that both are singletons
    }

    // Get the MySqlOptimizerDataSource
    OptimizerDataSource *getOptimizerDataSource() {
        return mySqlData;
    }

    // Get the StreamerDataSource
    StreamerDataSource *getStreamerDataSource() {
        return streamData;
    }
    
    // static OptimizerDataSource *getOptimizerDataSource(StreamerDataSource *dataSource);

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
