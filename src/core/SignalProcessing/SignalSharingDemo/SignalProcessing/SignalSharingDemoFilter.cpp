////////////////////////////////////////////////////////////////////////////////
// $Id: SignalSharingDemoFilter.cpp 8013 2024-04-09 18:35:19Z mellinger $
//
// $BEGIN_BCI2000_LICENSE$
//
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2023: BCI2000 team and many external contributors ]
//
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
//
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "SignalSharingDemoFilter.h"
#include "MessageChannel.h"
#include "Sockets.h"
#include "Streambuf.h"
#include "Thread.h"
#include "WaitableEvent.h"

struct SignalSharingDemoFilter::Private
{
    // A socket for a connection to the client application.
    ClientTCPSocket mSocket;
    // A signal to be shared.
    GenericSignal mSharedSignal;
    // An event to notify the sender thread when new signal data is available.
    WaitableEvent mSignalAvailable;
    // An object to wrap a call to SenderThreadFunc().
    MemberCall<void(Private *)> mSenderThreadCall;
    // The sender thread.
    Thread mSenderThread;

    Private();
    void SenderThreadFunc();
};

SignalSharingDemoFilter::Private::Private()
    : mSenderThreadCall(&Private::SenderThreadFunc, this),
      mSenderThread(&mSenderThreadCall, "Signal Sharing Demo Sender"), mSignalAvailable(true)
{
    mSocket.SetTCPNodelay(true);
    mSocket.SetFlushAfterWrite(true);
}

void SignalSharingDemoFilter::Private::SenderThreadFunc()
{
    if (mSocket.Connected())
    {
        UnbufferedIO buf;
        buf.SetOutput(&mSocket.Output());
        // A message channel wraps up BCI2000 objects into BCI2000 messages.
        MessageChannel messageChannel(buf);
        messageChannel.Send(mSharedSignal.Properties()); // send signal properties first
        while (mSignalAvailable.Wait()) // will return false when Thread::Terminate() has been called
        {
            if (!messageChannel.Send(mSharedSignal)) // will transmit memory reference, or signal, depending on
                                                     // whether ShareAcrossModules() has been called
            {
                bciwarn << "Could not send signal, giving up";
                mSenderThread.Terminate();
            }
        }
        messageChannel.Send(GenericSignal()); // empty signal to indicate end of data
    }
}

// Main class

SignalSharingDemoFilter::SignalSharingDemoFilter() : p(new Private)
{
}

SignalSharingDemoFilter::~SignalSharingDemoFilter()
{
    delete p;
}

void SignalSharingDemoFilter::Publish()
{
    BEGIN_PARAMETER_DEFINITIONS
    "Filtering string SignalSharingDemoClientAddress= localhost:1879 localhost:1879 % % "
    "// port client is listening on, empty to disable",
        END_PARAMETER_DEFINITIONS
}

void SignalSharingDemoFilter::Preflight(const SignalProperties &Input, SignalProperties &Output) const
{
    Parameter("SignalSharingDemoClientAddress");
    Output = Input;
}

void SignalSharingDemoFilter::Initialize(const SignalProperties &Input, const SignalProperties &Output)
{
    p->mSocket.Close();
    p->mSocket.Input().ClearIOState();
    p->mSocket.Output().ClearIOState();
    std::string addr = Parameter("SignalSharingDemoClientAddress");
    if (!addr.empty())
    { // connect to the client side
        p->mSocket.Open(addr);
        // initialize shared signal from input signal properties
        p->mSharedSignal = GenericSignal(Input);
        if (!p->mSocket.Connected())
        {
            bciwarn << "Cannot connect to " << addr;
        }
        else if (p->mSocket.Connected() == Socket::local) // only use shared memory when connected locally
        {
            p->mSharedSignal.ShareAcrossModules();
            bciwarn << "Locally connected to " << addr << ", using shared memory";
        }
        else // otherwise, full data will be transmitted over network (slower)
        {
            bciwarn << "Remotely connected to " << addr << ", transmitting data through network";
        }
    }
}

void SignalSharingDemoFilter::Halt()
{
    p->mSocket.Close();
    p->mSenderThread.Terminate();
}

void SignalSharingDemoFilter::StartRun()
{
    p->mSenderThread.Start();
}

void SignalSharingDemoFilter::StopRun()
{
    p->mSenderThread.Terminate();
}

void SignalSharingDemoFilter::Process(const GenericSignal &Input, GenericSignal &Output)
{
    Output = Input;
    p->mSharedSignal.AssignValues(Input);
    p->mSignalAvailable.Set();
}
