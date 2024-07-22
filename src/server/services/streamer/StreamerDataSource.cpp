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
void StreamerDataSource::getThroughputInfo(const Pair &pair, const boost::posix_time::time_duration &interval,
    double *throughput, double *filesizeAvg, double *filesizeStdDev) {
    FTS3_COMMON_LOGGER_NEWLOG(INFO) << "Streamer version of getThroughputInfo called" << commit;
    // // print out the totalFileSizeMB for each of the StreamerPairStates
    // for (const auto& pairState : pairToCyclicBuffer[pair].pairStateArray) {
    //     FTS3_COMMON_LOGGER_NEWLOG(INFO) << "AAD: TotalFileSizeMB: " << pairstate << commit;
    // }

    CyclicPerformanceBuffer &cpb = pairToCyclicBuffer[pair];
    cpb.getPairThroughputInfo(interval, throughput);  // read throughput for the interval 
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXG. filesizeavg, filesizestddev: " << *filesizeAvg << " " << *filesizeStdDev << commit;
    m_sds[pair].getPairFileSizeInfo(1000, filesizeAvg, filesizeStdDev);
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXE. filesizeavg, filesizestddev: " << *filesizeAvg << " " << *filesizeStdDev << commit;

    // m_sds[pair].getPairFileSizeInfo(static_cast<uint64_t>(interval.total_microseconds()), filesizeAvg, filesizeStdDev);
}

time_t StreamerDataSource::getAverageDuration(const Pair &pair, const boost::posix_time::time_duration &interval) {
    //return m_sds[pair].getAvgDuration();
    return 0;
}

// Get the success rate for the pair
double StreamerDataSource::getSuccessRateForPair(const Pair &pair, const boost::posix_time::time_duration &interval,
    int *retryCount) {
    int nFailedLastHour = m_sd[pair].failedCount;
    int nFinishedLastHour = m_sd[pair].finishedCount;
    
    // TODO: Figure out which signal to use for the retryCount
    retryCount = 0;
    
    if (nFailedLastHour + nFinishedLastHour == 0) {
        return 1.0;
    }
    return static_cast<double>(nFinishedLastHour) / (nFailedLastHour + nFinishedLastHour);
}

// Get the number of transfers in the given state
int StreamerDataSource::getActive(const Pair &pair) {
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "AAD getActive: " << m_sd[pair].activeCount << commit;
    return m_sd[pair].activeCount;
}
int StreamerDataSource::getSubmitted(const Pair &pair) {
    return m_sd[pair].submittedCount;
}

// Get current throughput
double StreamerDataSource::getThroughputAsSource(const std::string &se) {
    double totalThroughput = 0.0;
    for (const auto& pair : s_activePairs) {
        if (pair.source == se) {
            totalThroughput += pairToCyclicBuffer[pair].getPairThroughputInfo();
        }
    }
    return totalThroughput;
}

double StreamerDataSource::getThroughputAsDestination(const std::string &se) {
    double totalThroughput = 0.0;
    for (const auto& pair : s_activePairs) {
        if (pair.destination == se) {
            totalThroughput += pairToCyclicBuffer[pair].getPairThroughputInfo();
        }
    }
    return totalThroughput;
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