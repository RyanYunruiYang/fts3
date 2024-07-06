#include "sociConversions.h"
#include 

class IntegratedDataSource: public OptimizerDataSource {
protected:
    MySqlOptimizerDataSource mySqlData;
    AggregatorDataSource aggData;
public:
    IntegratedDataSource(soci::connection_pool* connectionPool): mySqlData(*connectionPool), aggData() {

    }

    // Return a list of pairs with active or submitted transfers
    std::list<Pair> getActivePairs(void) {
        return mySqlData.getActivePairs();
    }

    // Return the optimizer configuration value
    OptimizerMode getOptimizerMode(const std::string &source, const std::string &dest) {
        return mySqlData.getOptimizerMode(source, dest);
    }

    // Get configured limits
    void getPairLimits(const Pair &pair, Range *range, StorageLimits *limits) {
        return mySqlData.getPairLimits(pair, range, limits);
    }

    // Get the stored optimizer value (current value)
    int getOptimizerValue(const Pair& pair) {
        return mySqlData.getOptimizerValue(pair);
    }

    // Get the weighted throughput for the pair
    void getThroughputInfo(const Pair &pair, const boost::posix_time::time_duration &interval,
        double *throughput, double *filesizeAvg, double *filesizeStdDev) {
        return mySqlData.getThroughputInfo(pair, interval, throughput, filesizeAvg, filesizeStdDev);
    }

    time_t getAverageDuration(const Pair &pair, const boost::posix_time::time_duration &interval) {
        return mySqlData.getAverageDuration(pair, interval);
    }

    // Get the success rate for the pair
    double getSuccessRateForPair(const Pair &pair, const boost::posix_time::time_duration &interval,
        int *retryCount) {
        return mySqlData.getSuccessRateForPair(pair, interval, retryCount);
    }

    // Get the number of transfers in the given state
    int getActive(const Pair &pair) {
        return mySqlData.getActive(pair);
    }
    int getSubmitted(const Pair &pair) {
        return mySqlData.getSubmitted(pair);
    }

    // Get current throughput
    double getThroughputAsSource(const std::string &se) {
        return mySqlData.getThroughputAsSource(se);
    }
    double getThroughputAsDestination(const std::string &se) {
        return mySqlData.getThroughputAsDestination(se);
    }

    // Permanently register the optimizer decision
    void storeOptimizerDecision(const Pair &pair, int activeDecision,
        const PairState &newState, int diff, const std::string &rationale) {
        return mySqlData.storeOptimizerDecision(pair, activeDecision, newState, diff, rationale);
    }
    // Permanently register the number of streams per active
    void storeOptimizerStreams(const Pair &pair, int streams) {
        return mySqlData.storeOptimizerStreams(pair, streams);
    }
};