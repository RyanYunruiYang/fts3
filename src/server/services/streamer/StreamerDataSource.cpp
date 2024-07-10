#include "StreamerDataSource.h"
#include "common/Logger.h"
using namespace fts3::common;

StreamerDataSource::StreamerDataSource(): numPM(0), cyclicPerfBuffer(5) {
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "StreamerDataSource created" << commit;
}

StreamerDataSource::~StreamerDataSource() {
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "StreamerDataSource destroyed" << commit;
}


// Return a list of pairs with active or submitted transfers
std::list<Pair> StreamerDataSource::getActivePairs(void) {
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "DEV: StreamerDataSource getActivePairs numPM=" << numPM << commit;
    // Print the set of active pairs
    for (const auto& pair : s_activePairs) {
        FTS3_COMMON_LOGGER_NEWLOG(INFO) << "\tAAB Active Pair: " << pair << commit;
    }
    return std::list<Pair>(s_activePairs.begin(), s_activePairs.end());
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
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "Streamer version of getThroughputInfo called" << commit;
    // print out the totalFileSizeMB for each of the StreamerPairStates
    for (const auto& pairState : cyclicPerfBuffer.pairStateArray) {
        FTS3_COMMON_LOGGER_NEWLOG(INFO) << "AAD: TotalFileSizeMB: " << pairState.first->totalFileSizeMB << commit;
    }
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
int StreamerDataSource::getActive(const Pair &pair) {
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "AAD getActive: " << m_sd[pair].activeCount << commit;
    return m_sd[pair].activeCount;
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