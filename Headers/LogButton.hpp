#pragma once

#include "LogKey.hpp"

#include <memory>
#include <array>
#include <vector>


class LogButton
{
    public:
        LogButton(const unsigned idx, LogKey &key);

        void processRealtimeInput();

        static void moveIndex();
        bool isButtonPressed() const;
        static const std::vector<unsigned>& getPressedIndices() { return sPressedIndices; }
        static bool isOutOfOrder(unsigned idx) { return sOutOfOrder[idx] != 0; }
        static void setNumKeys(size_t num);

		void accumulateBeatsPerMinute();
        void reset();

        static float getKeysPerSecond();
        static float getMaxKeysPerSecond();
        static unsigned getTotal();
        static float getBeatsPerMinute();
    

    protected:
        float getLocalBeatsPerMinute() const;


    public:
        static float statKeysPerSecond;
        static float statMaxKeysPerSecond;
        static unsigned statTotal;
        static float statBeatsPerMinute;


    protected:
        LogKey mKey;
        float mKeysPerSecond;
        unsigned mTotal;


    private:
        const unsigned mBtnIdx;
        bool mState;
		unsigned mLastAccumulateBpmBufferIndex;
        
        // Calculation related
        std::array<unsigned, 60lu> mBuffer;
        std::array<float, 7lu> mPrevKpsBuffer;
        static unsigned mBufferIndex;
        static unsigned mPrevKpsBufferIndex;

        static std::vector<unsigned> sPressedIndices;
        static unsigned sExpectedIndex;
        static std::vector<char> sOutOfOrder;
        static size_t sNumKeys;
        static bool sResetNext;
    
};