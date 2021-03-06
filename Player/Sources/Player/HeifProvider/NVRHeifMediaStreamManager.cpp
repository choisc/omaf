
/**
 * This file is part of Nokia OMAF implementation
 *
 * Copyright (c) 2018-2019 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: omaf@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */
#include "NVRHeifMediaStreamManager.h"
#include "Media/NVRMediaPacket.h"
#include "NVRHeifParser.h"
#include "Foundation/NVRLogger.h"

OMAF_NS_BEGIN
OMAF_LOG_ZONE(HeifMediaStreamManager);

    HeifMediaStreamManager::HeifMediaStreamManager()
        : mAllocator(*MemorySystem::DefaultHeapAllocator())
        , mMediaFormatParser(OMAF_NULL)
        , mAudioStreams()
        , mVideoStreams()
    {
    }

    HeifMediaStreamManager::~HeifMediaStreamManager()
    {
        OMAF_DELETE(mAllocator, mMediaFormatParser);
    }

    Error::Enum HeifMediaStreamManager::openInput(const PathName& mediaUri)
    {
        // free old mp4reader
        if (mMediaFormatParser != OMAF_NULL)
        {
            OMAF_DELETE(mAllocator, mMediaFormatParser);
            mMediaFormatParser = OMAF_NULL;
        }

        // create new mp4reader
        mMediaFormatParser = OMAF_NEW(mAllocator, HeifParser)();

        Error::Enum result = mMediaFormatParser->openInput(mediaUri);

        selectNextImage();

        return result;
    }

    Error::Enum HeifMediaStreamManager::selectNextImage() {
        resetStreams();
        
        mMediaFormatParser->selectNextImage(mVideoStreams);
        for (MP4VideoStreams::ConstIterator it = mVideoStreams.begin(); it != mVideoStreams.end(); ++it)
        {
            (*it)->setMode(VideoStreamMode::BASE);
        }
        return Error::OK;
    }

    void_t HeifMediaStreamManager::resetStreams()
    {
        while (!mVideoStreams.isEmpty())
        {
            MP4VideoStream* videoStream = mVideoStreams[0];
            videoStream->resetPackets();
            mVideoStreams.removeAt(0);
            OMAF_DELETE(mAllocator, videoStream);
        }
        mVideoStreams.clear();

        while (!mAudioStreams.isEmpty())
        {
            MP4AudioStream* audioStream = mAudioStreams[0];
            audioStream->resetPackets();
            mAudioStreams.removeAt(0);
            OMAF_DELETE(mAllocator, audioStream);
        }
        mAudioStreams.clear();
    }

    const MP4AudioStreams& HeifMediaStreamManager::getAudioStreams()
    {
        return mAudioStreams;
    }

    const MP4VideoStreams& HeifMediaStreamManager::getVideoStreams()
    {
        return mVideoStreams;
    }
    const MP4MetadataStreams& HeifMediaStreamManager::getMetadataStreams()
    {
        return mMetadataStreams;
    }
    int32_t HeifMediaStreamManager::getNrStreams()
    {
        return static_cast<int32_t>(mAudioStreams.getSize() + mVideoStreams.getSize());
    }

    MP4VRMediaPacket* HeifMediaStreamManager::getNextAudioFrame(MP4MediaStream& stream)
    {
        return OMAF_NULL;
    }

    MP4VRMediaPacket* HeifMediaStreamManager::getNextVideoFrame(MP4MediaStream& stream, int64_t currentTimeUs)
    {
        if (!stream.hasFilledPackets())
        {
            bool_t segmentChanged = false;  // the value is not used in local file playback
            mMediaFormatParser->readFrame(stream);
        }
        return stream.peekNextFilledPacket();
    }
    MP4VRMediaPacket* HeifMediaStreamManager::getMetadataFrame(MP4MediaStream& aStream, int64_t aCurrentTimeUs)
    {
        return OMAF_NULL;
    }

    Error::Enum HeifMediaStreamManager::readVideoFrames(int64_t currentTimeUs)
    {
        // Do nothing here on file playback
        return Error::OK;
    }
    Error::Enum HeifMediaStreamManager::readAudioFrames()
    {
        // Do nothing here on file playback
        return Error::OK;
    }
    Error::Enum HeifMediaStreamManager::readMetadata()
    {
        // Do nothing here on file playback
        return Error::OK;
    }

    const CoreProviderSourceTypes& HeifMediaStreamManager::getVideoSourceTypes()
    {
        return mMediaFormatParser->getVideoSourceTypes();
    }

    const CoreProviderSources& HeifMediaStreamManager::getVideoSources()
    {
        return mMediaFormatParser->getVideoSources();
    }

    bool_t HeifMediaStreamManager::isBuffering()
    {
        return false;
    }

    bool_t HeifMediaStreamManager::isEOS() const
    {
        return false;
    }

    bool_t HeifMediaStreamManager::isReadyToSignalEoS(MP4MediaStream& aStream) const
    {
        return true;
    }

    bool_t HeifMediaStreamManager::seekToUs(uint64_t& seekPosUs, SeekDirection::Enum direction, SeekAccuracy::Enum accuracy)
    {
        return false;
    }

    bool_t HeifMediaStreamManager::seekToFrame(int32_t seekFrameNr, uint64_t& seekPosUs)
    {
        return false;
    }

    uint64_t HeifMediaStreamManager::getNextVideoTimestampUs()
    {
        return 0;
    }

    // this is not a full seek as it is done for video only. It is used in alternate tracks case to find the time when the next track is ready to switch and pre-seek the next track for the switch
    int64_t HeifMediaStreamManager::findAndSeekSyncFrameMs(uint64_t currentTimeMs, MP4MediaStream* videoStream, SeekDirection::Enum direction)
    {
        return 0;
    }

    MediaInformation HeifMediaStreamManager::getMediaInformation()
    {
        if (mMediaFormatParser != OMAF_NULL)
        {
            return mMediaFormatParser->getMediaInformation();
        }
        else
        {
            return MediaInformation();
        }
    }

OMAF_NS_END
