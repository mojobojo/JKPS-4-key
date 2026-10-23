#include "../Headers/LogButton.hpp"
#include "../Headers/Settings.hpp"

#include <cassert>
#include <iostream>

float LogButton::statMaxKeysPerSecond(0);
unsigned LogButton::statTotal(0);
float LogButton::statBeatsPerMinute(0);
float LogButton::statKeysPerSecond(0);
unsigned LogButton::mBufferIndex(0);
unsigned LogButton::mPrevKpsBufferIndex(0);
size_t LogButton::sNumKeys = 4;
std::vector<char> LogButton::sOutOfOrder(4, 0);
unsigned LogButton::sExpectedIndex = 0;
std::vector<unsigned> LogButton::sPressedIndices;
bool LogButton::sResetNext = false;

void LogButton::setNumKeys(size_t num) { 
    sNumKeys = num; 
    sOutOfOrder.assign(num, 0);
}

LogButton::LogButton(const unsigned idx, LogKey &key)
: mState(false)
, mLastAccumulateBpmBufferIndex(mBuffer.size())
, mKey(key)
, mKeysPerSecond(0)
, mTotal(0)
, mBtnIdx(idx)
{
    statMaxKeysPerSecond = Settings::MaxKPS;
    statTotal = Settings::Total;
    for (auto &elem : mBuffer)
        elem = 0u;
    for (auto &elem : mPrevKpsBuffer)
        elem = 0.f;
}

void LogButton::processRealtimeInput()
{
    if (sResetNext) {
        sExpectedIndex = 0;
        sOutOfOrder.assign(sNumKeys, 0);
        sResetNext = false;
        std::cout << "Reset to 0" << std::endl;
    }

    auto &bufferElem = mBuffer[mBufferIndex];
    const auto prevState = mState;
    mState = mKey.isPressed();

    if (!prevState && mState)
    {
        //std::cout << "Pressed " << mBtnIdx << ", expected " << sExpectedIndex << ", out of order: " << (mBtnIdx != sExpectedIndex) << std::endl;
        if (mBtnIdx != sExpectedIndex) {
            sOutOfOrder[mBtnIdx] = 1;
        } else {
            sOutOfOrder[mBtnIdx] = 0;
            sExpectedIndex = (sExpectedIndex + 1) % sNumKeys;
        }
        sPressedIndices.push_back(mBtnIdx);
    }
    if (prevState && !mState)
    {
        sPressedIndices.erase(std::remove(sPressedIndices.begin(), sPressedIndices.end(), mBtnIdx), sPressedIndices.end());
    }

    if (bufferElem > 0u)
    {
        mKeysPerSecond -= static_cast<float>(bufferElem);
        bufferElem = 0u;
        --statKeysPerSecond;
        assert(bufferElem == 0u);
    }
    if (!prevState && mState)
    {
        ++mKeysPerSecond;
        ++bufferElem;
        ++statKeysPerSecond;
        if (statKeysPerSecond > statMaxKeysPerSecond)
            statMaxKeysPerSecond = Settings::MaxKPS = statKeysPerSecond;

        const auto amtToAdd = 1u * Settings::ButtonPressMultiplier;
        mTotal += amtToAdd;
        statTotal += amtToAdd;
        Settings::Total += amtToAdd;
        Settings::KeysTotal[mBtnIdx] += amtToAdd;
    }

    mPrevKpsBuffer[mPrevKpsBufferIndex] = mKeysPerSecond;
	accumulateBeatsPerMinute();
}

bool LogButton::isButtonPressed() const
{
    return mState;
}

void LogButton::moveIndex()
{
    if (++mBufferIndex == 60)
        mBufferIndex = 0;
    if (++mPrevKpsBufferIndex == 7)
        mPrevKpsBufferIndex = 0;
    
	statBeatsPerMinute = 0;
}

void LogButton::accumulateBeatsPerMinute()
{
	if (mBufferIndex != mLastAccumulateBpmBufferIndex)
	{
		statBeatsPerMinute += getLocalBeatsPerMinute();
		mLastAccumulateBpmBufferIndex = mBufferIndex;
	}
}

void LogButton::reset()
{
    mKeysPerSecond = mTotal = statKeysPerSecond = statTotal = statBeatsPerMinute = statMaxKeysPerSecond = Settings::MaxKPS = Settings::Total = Settings::KeysTotal[mBtnIdx] = 0;
    for (auto &elem : mBuffer)
        elem = 0;
    for (auto &elem : mPrevKpsBuffer)
        elem = 0;
}

float LogButton::getKeysPerSecond()
{
    return statKeysPerSecond;
}

float LogButton::getMaxKeysPerSecond()
{
    return statMaxKeysPerSecond;
}

unsigned LogButton::getTotal()
{
    return statTotal;
}

float LogButton::getBeatsPerMinute()
{
    return statBeatsPerMinute;
}

float LogButton::getLocalBeatsPerMinute() const
{
    float prevKpsSum = 0;
    for (const auto &elem : mPrevKpsBuffer)
        prevKpsSum += elem;

    // 15 = 60 (sec) / 4 (1/4 time signature for streams)
    return prevKpsSum / mPrevKpsBuffer.size() * 15;
}
