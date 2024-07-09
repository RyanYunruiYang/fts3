#include "StreamerDataSource.h"
#include "common/Logger.h"
using namespace fts3::common;

StreamerDataSource::StreamerDataSource(): numPM(0) {
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "StreamerDataSource created" << commit;
}

StreamerDataSource::~StreamerDataSource() {
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "StreamerDataSource destroyed" << commit;
}


// Return a list of pairs with active or submitted transfers
std::list<Pair> StreamerDataSource::getActivePairs(void) {
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "DEV: StreamerDataSource getActivePairs numPM=" << numPM << commit;
    return std::list<Pair>();
}

// Return the optimizer configuration value
OptimizerMode StreamerDataSource::getOptimizerMode(const std::string &, const std::string &) {
    return kOptimizerDisabled;
}

// Get configured limits
void StreamerDataSource::getPairLimits(const Pair &, Range *, StorageLimits *) {
    return;
}

// Get the stored optimizer value (current value)
int StreamerDataSource::getOptimizerValue(const Pair&) {
    return 0;
}

// Get the weighted throughput for the pair
void StreamerDataSource::getThroughputInfo(const Pair &, const boost::posix_time::time_duration &,
    double *, double *, double *) {
    return;
}

time_t StreamerDataSource::getAverageDuration(const Pair &, const boost::posix_time::time_duration &) {
    return 0;
}

// Get the success rate for the pair
double StreamerDataSource::getSuccessRateForPair(const Pair &, const boost::posix_time::time_duration &,
    int *) {
    return 0;
}

// Get the number of transfers in the given state
int StreamerDataSource::getActive(const Pair &) {
    return 0;
}
int StreamerDataSource::getSubmitted(const Pair &) {
    return 0;
}

// Get current throughput
double StreamerDataSource::getThroughputAsSource(const std::string &) {
    return 0;
}
double StreamerDataSource::getThroughputAsDestination(const std::string &) {
    return 0;
}

// Permanently register the optimizer decision
void StreamerDataSource::storeOptimizerDecision(const Pair &, int,
    const PairState &, int, const std::string &) {
    return;
}
// Permanently register the number of streams per active
void StreamerDataSource::storeOptimizerStreams(const Pair &, int) {
    return;
}