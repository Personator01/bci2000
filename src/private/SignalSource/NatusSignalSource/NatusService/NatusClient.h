#ifndef __NatusClient_H_
#define __NatusClient_H_
#include "NatusPackageInterface.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

#ifndef USE_CONCURRENT_QUEUE
# define USE_CONCURRENT_QUEUE 0
#endif

#if USE_CONCURRENT_QUEUE
#include <concurrent_queue.h>
using namespace concurrency;
#else
template <class T> class concurrent_queue
{
public:
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mQueue.empty();
    }
    size_t unsafe_size() const // this is not unsafe
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mQueue.size();
    }
    void push(const T& t)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.push(t);
    }
    bool try_pop(T& dest)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mQueue.empty())
            return false;
        dest = mQueue.front();
        mQueue.pop();
        return true;
    }
private:
    std::queue<T> mQueue;
    mutable std::mutex mMutex;
};
#endif

#define TIMEOUT_UDP 1
class NatusClient :public  NatusCommunication
{
public:
	NatusClient ();
	~NatusClient ();
	NatusDeviceInformation GetInformation ();
	void Connect (std::string server_name, int port);
	bool IsConnected();
	bool TryConnect (std::string server_name, int port);
	void Disconnect ();
	void StartStream ();
	void StopStream ();
	uint8_t Ping (uint8_t ping);
	bool GetSamples(void* buffer, uint32_t numSamples, uint32_t buffersize);
	bool waitForPackage();
	NatusSampleBlock * create_natus_sampleblock ( uint8_t * payload);
	void DeleteSampleBlock (NatusSampleBlock*);
	void SetSampleBlockSize (uint32_t blocksize);
	void SetDecimationFactor(uint32_t decimationFactor);
	NatusChannelInformation GetChannelInfo ();
	int GetDecimationFactor();
	int SelectChannelsToStream(std::vector<uint32_t>);


protected:
	NatusSampleBlock* ReadStream();
	void data_received (CmdStruct * payload) override;
	CmdStruct* blockForPackage (uint32_t timeout);
	std::queue<NatusSampleBlock*> _receivequeue;
	bool init_udp (int port);
	NatusSampleBlock _prevblock;
	void interpolateMissingBlocks(NatusSampleBlock *);
	void emptyReceiveQueue();
	void updatePreviousBlock(NatusSampleBlock *);
	void aquisitionWorker();

private:
	std::thread* _acqThread;
	std::atomic_bool _doAquire;
	std::atomic_int32_t _numChannels;
	concurrent_queue<NatusSampleBlock*> _acqQueue;
	std::atomic_int _numSamples;

	std::condition_variable mConditionVariable;
	std::mutex mConditionMutex;
	bool mSyncCondition;
};




#endif
