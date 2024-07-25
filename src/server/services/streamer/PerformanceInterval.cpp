#include "StreamerDataSource.h"
#include "PerformanceInterval.h"
#include "CyclicPerformanceBuffer.h"

StreamerFileState::StreamerFileState(const StreamerPerfMarker &pm)
    : fileId(pm.fileId), fileSize(pm.userFileSize), startTime(pm.timestamp), lastTimestamp(pm.timestamp), transferredBytes(pm.transferred), fileDuration(0), instThroughput(0.0) {
}

void StreamerFileState::update(const StreamerPerfMarker &pm) {
    lastTimestamp = pm.timestamp;
    transferredBytes = pm.transferred;
}

void PerformanceInterval::processNewTransfer(const StreamerPerfMarker &pm) {
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXF Arguments (timestamp, fileSize) = " << pm.timestamp << " " << pm.userFileSize << fts3::common::commit;

    updateTimestamp = pm.timestamp;

    // update stat info
    activeCount += 1;
    uint64_t tmp = pm.userFileSize / (1024 * 1024);
    totalFileSizeMB        += tmp;
    totalFileSizeSquaredMB += tmp * tmp;
    // totalDuration is modified only in finish event

    // update records 
    std::shared_ptr<StreamerFileState> newFile = std::make_shared<StreamerFileState>(pm);
    activeFiles[pm.fileId] = newFile;
    FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXF. fileTransfers Current List of size " << activeFiles.size() << fts3::common::commit;
    for (const auto& x : activeFiles) {
        FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << x.first << " " << x.second << fts3::common::commit;
    }

    // TODO: add state to deal with state ARCHIVING
}

void PerformanceInterval::processComplete(const StreamerPerfMarker &pm, CyclicPerformanceBuffer& cyclicBuffer) {
    updateTimestamp = pm.timestamp;

    // deal with performance data
    processPerformanceMarker(pm, cyclicBuffer);

    // update stat info
    activeCount   -= 1;
    finishedCount += 1;

    // update records
    // move from activeFiles to finishedFiles
    auto it = activeFiles.find(pm.fileId);
    if (it != activeFiles.end()) {
        finishFiles.insert(std::move(*it));
        activeFiles.erase(it);
        finishFiles[pm.fileId]->fileDuration = pm.timestamp - finishFiles[pm.fileId]->startTime; 
    }

    // TODO: add state to deal with state ARCHIVING
}



void PerformanceInterval::processPerformanceMarker(const StreamerPerfMarker& pm, CyclicPerformanceBuffer& cyclicBuffer)
{
    updateTimestamp = pm.timestamp;

    // update stat info
    //transferredBytes = pm.transferred;

    double delta = pm.transferred;
    if (activeFiles.find(pm.fileId)==activeFiles.end()) {
        FTS3_COMMON_LOGGER_NEWLOG(DEBUG) << "XXH ERROR: Missing new transfer event for file " << pm.fileId << fts3::common::commit;
    } else {
        delta -= activeFiles[pm.fileId]->transferredBytes;
    }
    //cross check previous intervals to update totalTransferredMB in current and also previous intervals if necessary
    cyclicBuffer.updateTransferred(activeFiles[pm.fileId]->lastTimestamp, pm.timestamp, delta);
    // update records
    activeFiles[pm.fileId]->update(pm);
}