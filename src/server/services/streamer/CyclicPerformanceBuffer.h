#ifndef CyclicPerformanceBuffer_H_
#define CyclicPerformanceBuffer_H_

#include <vector>
#include <cstdint>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

class PerformanceInterval;

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
    std::vector<std::shared_ptr<PerformanceInterval>> pairStateArray;
    
public:

    CyclicPerformanceBuffer(): CyclicPerformanceBuffer(10) {}
    CyclicPerformanceBuffer(int nb): numBuckets(nb), epochSize(nb*bucketWidth) {
        pairStateArray.resize(numBuckets); // Resize the vector to the specified number of buckets
        for (int i = 0; i < numBuckets; i++) {
            // Construct a shared_ptr to a PerformanceInterval at each index
            pairStateArray[i] = std::make_shared<PerformanceInterval>();
        }
    }

    std::shared_ptr<PerformanceInterval>& operator[](uint64_t t);

    void updateTransferred(uint64_t t1, uint64_t t2, double deltaTransferred);

    // For use by the Streamer Data Source (Northbound)
    void getPairThroughputInfo(const boost::posix_time::time_duration &interval, double *throughput);

    int getCurrentIndex () {
        time_t now = time(nullptr); // in seconds
        return getIndex(now);
    }

    std::shared_ptr<PerformanceInterval> getCurrentPerformanceInterval() {
        int currIndex = getCurrentIndex();
        return pairStateArray[currIndex];
    }

    int getPreIndex(int idx) {
        return (idx + numBuckets - 1) % numBuckets;
    }

    // get throughput for current interval
    void getPairThroughputInfo(double *throughput, double *filesizeAvg, double *filesizeStdDev);

protected:
    uint64_t getIndex(uint64_t t) {
        uint64_t epochTime = (t-baseTime)%(epochSize);
        return epochTime/bucketWidth;
    }
    uint64_t getEpoch(uint64_t t) {
        return (t-baseTime)/epochSize;
    }

};
#endif // CyclicPerformanceBuffer_H