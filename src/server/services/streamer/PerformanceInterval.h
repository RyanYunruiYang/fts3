#include <map>
#include <memory>
#include <cstdint>

#ifndef PerformanceInterval_H_
#define PerformanceInterval_H_

class CyclicPerformanceBuffer;
class StreamerPerfMarker;

// Ownership Model: Corresponds to one (src, dst, fileid) tuple.
struct StreamerFileState {
    uint64_t fileId;
    uint64_t fileSize;
    uint64_t startTime;
    uint64_t lastTimestamp;
    uint64_t transferredBytes;
    uint64_t fileDuration;

    double instThroughput;

    StreamerFileState() : fileId(0), fileSize(0), lastTimestamp(0), transferredBytes(0), fileDuration(0), instThroughput(0.0) {}

    StreamerFileState(uint64_t fileID, uint64_t fileSize, uint64_t lastTimestamp, uint64_t lastTransferredBytes, uint64_t fileDuration, double instThroughput)
        : fileId(fileID), fileSize(fileSize), lastTimestamp(lastTimestamp), transferredBytes(lastTransferredBytes), fileDuration(fileDuration), instThroughput(instThroughput) {}

    StreamerFileState(const StreamerPerfMarker &pm);

    StreamerFileState(const StreamerFileState &orig)
        : fileId(orig.fileId), fileSize(orig.fileSize), startTime(orig.startTime),
          lastTimestamp(orig.lastTimestamp), transferredBytes(orig.transferredBytes),
          fileDuration(orig.fileDuration), instThroughput(orig.instThroughput) {}

    void update(const StreamerPerfMarker &pm);
};


// Used in m_sds.
struct PerformanceInterval {
    uint64_t epoch;
    uint64_t updateTimestamp;

    // stat for the interval
    int submittedCount;
    int activeCount;
    int finishedCount;
    int failedCount;
    
    uint64_t totalDuration;   // stat data for fileTransfers, valid period [start,end] is hidden in fileTransfers
    uint64_t totalFileSizeMB; // stat data for fileTransfers
    uint64_t totalFileSizeSquaredMB; // stat data for fileTransfers
    double   totalTransferredMB; // from start of Streaming service

    // records, may be replaced later by only keeping streaming stats such as totalTransferredMG
    std::map<uint64_t, std::shared_ptr<StreamerFileState>> activeFiles;
    std::map<uint64_t, std::shared_ptr<StreamerFileState>> finishFiles;

    PerformanceInterval(): epoch(0), updateTimestamp(0),totalDuration(0), totalFileSizeMB(0), totalFileSizeSquaredMB(0) {}
    PerformanceInterval(int epoch, double totalTransferredMB):
        epoch(epoch), totalTransferredMB(totalTransferredMB) {}

    void inherit(const PerformanceInterval &orig, int newepoch) {
        epoch = newepoch;
        updateTimestamp = orig.updateTimestamp;
        activeCount = orig.activeCount;
        finishedCount = 0;
        totalDuration = 0;
        totalFileSizeMB = 0;
        totalFileSizeSquaredMB = 0;

        activeFiles.clear();
        finishFiles.clear();
        for (auto& file : orig.activeFiles) {
            activeFiles[file.first] = std::make_shared<StreamerFileState>(*file.second);
        }
    }

    void processNewTransfer(const StreamerPerfMarker &pm);                             
    void processPerformanceMarker(const StreamerPerfMarker& pm, CyclicPerformanceBuffer& cyclicBuffer);
    void processComplete(const StreamerPerfMarker &pm, CyclicPerformanceBuffer& cyclicBuffer);


    uint64_t getActiveTransferredBytes() {
        uint64_t total = 0;
        for (const auto&pair : activeFiles) {
            total += pair.second->transferredBytes;
        }
        return total;
    }

    uint64_t getTransferredBytes() {
        uint64_t total = getActiveTransferredBytes();
        for (const auto&pair : finishFiles) {
            total += pair.second->transferredBytes;
        }
        return total;
    }

    double getAvgFileSize() {
        uint64_t total = 0;
        for (const auto&pair : finishFiles) {
            total += pair.second->fileSize;
        }
        return double(total)/size(finishFiles);
    }

};

#endif // PerformanceInterval_H