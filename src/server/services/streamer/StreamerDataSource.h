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

// Used for sending performance markers.
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

// Used in m_sd.
struct StreamerPairState {
    int submittedCount;
    int activeCount;
    int finishedCount;
    int failedCount;

    StreamerPairState(): submittedCount(0), activeCount(0), finishedCount(0), failedCount(0) {}
    StreamerPairState(int submittedCount, int activeCount, int finishedCount, int failedCount):
        submittedCount(submittedCount), activeCount(activeCount), finishedCount(finishedCount), failedCount(failedCount) {}
};

// Used in m_sds.
struct PerformanceInterval {
    uint64_t epoch;
    double   totalTransferredMB;

    PerformanceInterval(): epoch(0), totalTransferredMB(0) {}
    PerformanceInterval(int epoch, double totalTransferredMB):
        epoch(epoch), totalTransferredMB(totalTransferredMB) {}
};


struct StreamerTransientStateStats { //keep recent history for a (src,dst) pair
    std::deque<std::pair<uint64_t, uint64_t>> fileTransfers; // history data (starttime, fileSize)
    uint64_t totalDuration;   // stat data for fileTransfers, valid period [start,end] is hidden in fileTransfers
    uint64_t totalFileSizeMB; // stat data for fileTransfers
    uint64_t totalFileSizeSquaredMB; // stat data for fileTransfers
    uint64_t interval;        // default query interval, in seconds
    uint64_t maxInterval;     // in seconds, purge period

    StreamerTransientStateStats()
        : totalDuration(0), totalFileSizeMB(0), totalFileSizeSquaredMB(0), interval(100), maxInterval(100) {}

    StreamerTransientStateStats(uint64_t interval, uint64_t maxInterval)
        : totalDuration(0), totalFileSizeMB(0), totalFileSizeSquaredMB(0), interval(interval), maxInterval(maxInterval) {}


    //timestamp is starttime, fileSize in bytes
    void processNewTransfer(uint64_t timestamp, uint64_t fileSize) {
        FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXF Arguments (timestamp, fileSize) = " << timestamp << " " << fileSize << fts3::common::commit;
        fileTransfers.emplace_back(timestamp, fileSize);
        FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXF. fileTransfers Current List of size " << fileTransfers.size() << fts3::common::commit;
        for (const auto& x : fileTransfers) {
            FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << x.first << " " << x.second << fts3::common::commit;
        }

        totalFileSizeMB        += fileSize / (1024 * 1024);
        totalFileSizeSquaredMB += (fileSize / (1024 * 1024)) * (fileSize / (1024 * 1024));
        // purgeOldTransfers();
    }

    /*
    double getAvgDuration() const {
        boost::posix_time::ptime curTime        = boost::posix_time::microsec_clock::universal_time();
        boost::posix_time::ptime validStartTime = curTime - interval;

        uint64_t durationSum = 0;
        int count = 0;

        for (const auto& transfer : fileTransfers) {
            if (transfer.second >= validStartTime) {
                durationSum += transfer.second;  // Assuming transfer.second is the duration for finished transfers
                count++;
            }
        }
        return count > 0 ? static_cast<double>(durationSum) / count : 0.0;
    }*/

    // get average file size during [currTime-interval, currTime]
    void getPairFileSizeInfo(uint64_t interval, double* avg, double* stdDev) const {
        std::time_t curTime     = std::time(nullptr);
        uint64_t validStartTime = (uint64_t)curTime - interval;

        uint64_t validTotalFileSizeMB        = 0;
        uint64_t validTotalFileSizeSquaredMB = 0;
        int count = 0;

        FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXG. getPairFileSizeInfo Called " << fileTransfers.size() << fts3::common::commit;

        for (const auto& transfer : fileTransfers) {
            FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXE. Transfer: " << transfer.first << "," << transfer.second << fts3::common::commit;
            if (transfer.first >= validStartTime) { // transfer type (fileSizeInBytes, startTime)
                uint64_t fileSizeMB   = transfer.second / (1024 * 1024);
                validTotalFileSizeMB += fileSizeMB;
                validTotalFileSizeSquaredMB += fileSizeMB * fileSizeMB;
                count++;
                FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXE. File size: " << fileSizeMB << " MB" << fts3::common::commit;
                FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXE. Valid total file size: " << validTotalFileSizeMB << " MB" << fts3::common::commit;
            }
        }
        
        if (count == 0) {
            *avg = 0.0;
            *stdDev = 0.0;
            return;
        }

        *avg = static_cast<double>(validTotalFileSizeMB) / count;
        *stdDev = count > 1 
            ? sqrt((validTotalFileSizeSquaredMB - validTotalFileSizeMB * validTotalFileSizeMB / count) / (count - 1))
            : 0.0;
    }

private:
    void purgeOldTransfers() {
        uint64_t currentTime     = (uint64_t)std::time(nullptr);
        while (!fileTransfers.empty() && currentTime - fileTransfers.front().first > maxInterval) {
            auto [oldTimestamp, oldFileSize] = fileTransfers.front();
            totalFileSizeMB        -= oldFileSize  / (1024 * 1024);
            totalFileSizeSquaredMB -= (oldFileSize / (1024 * 1024)) * (oldFileSize / (1024 * 1024));
            fileTransfers.pop_front();
        }
    }
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
public:
    std::vector<std::shared_ptr<PerformanceInterval>> pairStateArray;

    CyclicPerformanceBuffer(): CyclicPerformanceBuffer(10) {}
    CyclicPerformanceBuffer(int nb): numBuckets(nb), epochSize(nb*bucketWidth) {
        pairStateArray.resize(numBuckets); // Resize the vector to the specified number of buckets
        for (int i = 0; i < numBuckets; i++) {
            // Construct a shared_ptr to a PerformanceInterval at each index
            pairStateArray[i] = std::make_shared<PerformanceInterval>();
        }
    }

    // For use by the Streamer Service (Southbound)
    // Note: It is the job of the user to write in a new epoch.
    std::shared_ptr<PerformanceInterval> getStreamerState(uint64_t t) {
        uint64_t currentEpoch = getEpoch(t);
        if (currentEpoch != pairStateArray[getIndex(t)]->epoch) {
            pairStateArray[getIndex(t)] = std::make_shared<PerformanceInterval>(); // Create a new shared_ptr to a StreamerTransientState at the current index
        }
        return pairStateArray[getIndex(t)];
    }

    void updateTransferred(uint64_t t1, uint64_t t2, uint64_t deltaTransferred) {
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
    void getPairThroughputInfo(const boost::posix_time::time_duration &interval, double *throughput) {
        // Compute the start and end time of the interval
        // TODO: Support generic [startime, endtime] (currently endtime = curtime always)
        // TODO: Handle when interval > epochSize (when data has been thrown out, change denominator)
        boost::posix_time::ptime curTime = boost::posix_time::microsec_clock::universal_time();
        boost::posix_time::ptime startTime = curTime - interval;
        uint64_t curTimeMillis = (curTime - boost::posix_time::from_time_t(0)).total_milliseconds();
        uint64_t startTimeMillis = (startTime - boost::posix_time::from_time_t(0)).total_milliseconds();
        uint64_t startIndex = getIndex(startTimeMillis);
        uint64_t curIndex = getIndex(curTimeMillis);

        // Compute the total transferred in range [curTime - interval, curTime].
        double totalTransferred = 0.0;
        for (uint64_t i = startIndex; i != curIndex; i = (i + 1) % numBuckets) {
            if (i==startIndex || i==curIndex) {
                uint64_t timeInInterval = std::min((i + 1) * bucketWidth, curTimeMillis) - std::max(i * bucketWidth, startTimeMillis);
                totalTransferred += pairStateArray[i]->totalTransferredMB * static_cast<double>(timeInInterval) / static_cast<double>(bucketWidth);
            } else {
                totalTransferred += pairStateArray[i]->totalTransferredMB;
            }
        }

        // Compute the average file size and standard deviation
        // TODO: Implement the computation

        // Compute the throughput
        *throughput = totalTransferred / interval.total_seconds();
    }

    double getPairThroughputInfo() {
        // Set default interval to 1 minute
        boost::posix_time::time_duration interval = boost::posix_time::minutes(1);
        double throughput;
        getPairThroughputInfo(interval, &throughput);
        return throughput;
    }

protected:
    uint64_t getIndex(uint64_t t) {
        uint64_t epochTime = (t-baseTime)%(epochSize);
        return epochTime/bucketWidth;
    }
    uint64_t getEpoch(uint64_t t) {
        return (t-baseTime)/epochSize;
    }

};

struct StreamerFileState {
    uint64_t lastTimestamp;
    uint64_t lastTransferredBytes;
    double instThroughput;
};

class StreamerDataSource: public OptimizerDataSource {
public:
    //TODO: Need to make the following private
    static const uint64_t T = 17*1000; // Period of time in milliseconds.

    uint64_t t0 = 0; // Timestamp in milliseconds after Epoch Time.
    std::map<Pair, StreamerPairState> m_sd;  // stat from the service start time (no expiration), answer query getActive, getSubmitted
    std::map<Pair, StreamerTransientStateStats> m_sds; //transient stat for last k intervals, answer query getThroughputInfo (filesize statistics), getAverageDuration
    std::map<Pair, CyclicPerformanceBuffer> pairToCyclicBuffer; // a cyclic buffer saving array of PerformanceInterval
    std::map<Pair, std::map<uint64_t, StreamerFileState>> m_sdf; // Maps the concatenated src+dst+jobid+fileid to its State.
    std::set<Pair> s_activePairs;
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