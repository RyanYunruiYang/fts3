#include "PerformanceInterval.h"
#include "CyclicPerformanceBuffer.h"
#include "common/Logger.h"


std::shared_ptr<PerformanceInterval>& CyclicPerformanceBuffer::operator[](uint64_t t) {
    uint64_t currentEpoch = getEpoch(t);
    int idx = getIndex(t);
    if (currentEpoch > pairStateArray[idx]->epoch) {
        //TODO: copy all the intermediate ones
        int preIdx = (idx - 1 + numBuckets) % numBuckets;
        pairStateArray[idx]->inherit(*pairStateArray[preIdx], currentEpoch);
    }
    return pairStateArray[idx];
}

void CyclicPerformanceBuffer::updateTransferred(uint64_t t1, uint64_t t2, double deltaTransferred) {
    /* Update the total transferred bytes in the epochs in the range [t1, t2]
        * Steps
        * 1. Get the indices corresponding to t1 and t2
        * 2. For each index-interval from index1 to index2, increment the totalTransferredMB by:
        *   deltaTransferred * (time spent in index-interval) / (t2-t1)
        */
    uint64_t index1 = getIndex(t1);
    uint64_t index2 = getIndex(t2);

    for (uint64_t i = index1; i != index2; i = (i+1)%numBuckets) {
        uint64_t timeInInterval = std::min((i+1)*bucketWidth, t2) - std::max(i*bucketWidth, t1);
        pairStateArray[i]->totalTransferredMB += deltaTransferred * timeInInterval / (t2-t1);
    }
}

// For use by the Streamer Data Source (Northbound)
void CyclicPerformanceBuffer::getPairThroughputInfo(const boost::posix_time::time_duration &interval, double *throughput) {
    // Compute the start and end time of the interval
    // TODO: Support generic [startime, endtime] (currently endtime = curtime always)
    // TODO: Handle when interval > epochSize (when data has been thrown out, change denominator)
    // boost::posix_time::ptime curTime = boost::posix_time::microsec_clock::universal_time();
    // boost::posix_time::ptime startTime = curTime - interval;
    // uint64_t curTimeMillis = (curTime - boost::posix_time::from_time_t(0)).total_milliseconds();
    // uint64_t startTimeMillis = (startTime - boost::posix_time::from_time_t(0)).total_milliseconds();

    time_t now = time(nullptr); // integer seconds.
    uint64_t startIndex = getIndex(now - interval.total_seconds());
    uint64_t curIndex = getIndex(now);

    // Compute the total transferred in range [curTime - interval, curTime].
    double totalTransferred = 0.0;
    for (uint64_t i = startIndex; i != curIndex; i = (i + 1) % numBuckets) {
        if (i==startIndex || i==curIndex) {
            uint64_t timeInInterval = std::min((i + 1) * bucketWidth, static_cast<uint64_t>(now)) - std::max(i * bucketWidth, static_cast<uint64_t>(now - interval.total_seconds()));
            totalTransferred += pairStateArray[i]->totalTransferredMB * static_cast<double>(timeInInterval) / static_cast<double>(bucketWidth);
        } else {
            totalTransferred += pairStateArray[i]->totalTransferredMB;
        }
    }

    // Compute the average file size and standard deviation
    // TODO: Implement the computation

    // Compute the throughput
    *throughput = totalTransferred / interval.total_seconds();

    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Throughput: " << *throughput << fts3::common::commit;


}


// get throughput for current interval
void CyclicPerformanceBuffer::getPairThroughputInfo(double *throughput, double *filesizeAvg, double *filesizeStdDev) {
    // get current interval
    int curr = getCurrentIndex();
    double currTransfer = pairStateArray[curr]->getTransferredBytes();
    int pre = getPreIndex(curr);
    double preTransfer = pairStateArray[pre]->getActiveTransferredBytes();
    *throughput = (currTransfer - preTransfer) / (pairStateArray[curr]->updateTimestamp - pairStateArray[pre]->updateTimestamp);
    *filesizeAvg = pairStateArray[curr]->getAvgFileSize();
    *filesizeStdDev = 1.2345;

    double preThroughput = pairStateArray[pre]->totalTransferredMB / bucketWidth;

    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Throughput: " << *throughput << fts3::common::commit;
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Average File Size: " << *filesizeAvg << fts3::common::commit;
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Standard Deviation: " << *filesizeStdDev << fts3::common::commit;
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "Previous Throughput: " << preThroughput << fts3::common::commit;
}