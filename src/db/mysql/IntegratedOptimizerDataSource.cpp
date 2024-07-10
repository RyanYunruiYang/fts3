#include "IntegratedOptimizerDataSource.h"
// #include "MySqlOptimizerDataSource.h"
using namespace fts3::common;


// @param connectionPool: parameter used to connect to the MySQL database
// @param 
IntegratedOptimizerDataSource::IntegratedOptimizerDataSource(OptimizerDataSource *msds,
                                                            StreamerDataSource *sds): 
                                                        mySqlData(msds), streamData(sds) {
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "DEV: IntegratedOptimizerDataSource created" << commit;
}

// Return a list of pairs with active or submitted transfers
std::list<Pair> IntegratedOptimizerDataSource::getActivePairs(void) {
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "DEV: IODS Dispatching getActivePairs" << commit;
    std::list<Pair> msPairs = mySqlData->getActivePairs();
    std::list<Pair> sPairs = streamData->getActivePairs();

    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "AAC: msPairs:" << commit;
    for (const auto& pair : msPairs) {
        FTS3_COMMON_LOGGER_NEWLOG(INFO) << pair << commit;
    }
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "AAC: sPairs:" << commit;
    for (const auto& pair : sPairs) {
        FTS3_COMMON_LOGGER_NEWLOG(INFO) << pair << commit;
    }
    
    return sPairs;
}

// Return the optimizer configuration value
OptimizerMode IntegratedOptimizerDataSource::getOptimizerMode(const std::string &source, const std::string &dest) {
    return mySqlData->getOptimizerMode(source, dest);
}

// Get configured limits
void IntegratedOptimizerDataSource::getPairLimits(const Pair &pair, Range *range, StorageLimits *limits) {
    return mySqlData->getPairLimits(pair, range, limits);
}

// Get the stored optimizer value (current value)
int IntegratedOptimizerDataSource::getOptimizerValue(const Pair& pair) {
    return mySqlData->getOptimizerValue(pair);
}

// Get the weighted throughput for the pair
void IntegratedOptimizerDataSource::getThroughputInfo(const Pair &pair, const boost::posix_time::time_duration &interval,
    double *throughput, double *filesizeAvg, double *filesizeStdDev) {
    streamData->getThroughputInfo(pair, interval, throughput, filesizeAvg, filesizeStdDev);
    return mySqlData->getThroughputInfo(pair, interval, throughput, filesizeAvg, filesizeStdDev);
}

time_t IntegratedOptimizerDataSource::getAverageDuration(const Pair &pair, const boost::posix_time::time_duration &interval) {
    return mySqlData->getAverageDuration(pair, interval);
}

// Get the success rate for the pair
double IntegratedOptimizerDataSource::getSuccessRateForPair(const Pair &pair, const boost::posix_time::time_duration &interval,
    int *retryCount) {
    return mySqlData->getSuccessRateForPair(pair, interval, retryCount);
}

// Get the number of transfers in the given state
int IntegratedOptimizerDataSource::getActive(const Pair &pair) {
    return streamData->getActive(pair);
}
int IntegratedOptimizerDataSource::getSubmitted(const Pair &pair) {
    return mySqlData->getSubmitted(pair);
}

// Get current throughput
double IntegratedOptimizerDataSource::getThroughputAsSource(const std::string &se) {
    return mySqlData->getThroughputAsSource(se);
}
double IntegratedOptimizerDataSource::getThroughputAsDestination(const std::string &se) {
    return mySqlData->getThroughputAsDestination(se);
}

// Permanently register the optimizer decision
void IntegratedOptimizerDataSource::storeOptimizerDecision(const Pair &pair, int activeDecision,
    const PairState &newState, int diff, const std::string &rationale) {
    return mySqlData->storeOptimizerDecision(pair, activeDecision, newState, diff, rationale);
}
// Permanently register the number of streams per active
void IntegratedOptimizerDataSource::storeOptimizerStreams(const Pair &pair, int streams) {
    return mySqlData->storeOptimizerStreams(pair, streams);
}