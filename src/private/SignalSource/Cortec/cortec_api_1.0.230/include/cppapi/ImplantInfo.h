/**********************************************************************
* Copyright 2015-2023, CorTec GmbH
* All rights reserved.
*
* Redistribution, modification, adaptation or translation is not permitted.
*
* CorTec shall be liable a) for any damage caused by a willful, fraudulent or grossly 
* negligent act, or resulting in injury to life, body or health, or covered by the 
* Product Liability Act, b) for any reasonably foreseeable damage resulting from 
* its breach of a fundamental contractual obligation up to the amount of the 
* licensing fees agreed under this Agreement. 
* All other claims shall be excluded. 
* CorTec excludes any liability for any damage caused by Licensee's 
* use of the Software for regular medical treatment of patients.
**********************************************************************/
#ifndef IMPLANTAPI_IMPLANT_INFO_H
#define IMPLANTAPI_IMPLANT_INFO_H
#include "cppapi/ChannelInfo.h"
#include <vector>
#include <string>
#include <stdint.h>


namespace cortec { namespace implantapi {

/**
* @brief Information on the implant including channel info.
*/
class CImplantInfo
{
public:
    /**
    * @param[in] deviceType Type of the implant.
    * @param[in] hardwareRevision Version of the implant's hardware.
    * @param[in] deviceId The unique identifier of the implant.
    * @param[in] firmwareVersion Version of the implant's firmware.
    * @param[in] samplingRate Sampling rate of the measurements.
    * @param[in] channelInfo Provides information of the capabilities of each channel.
    *            The ownership is passed to CImplantInfo.
    *
    * @throws CInvalidArgumentException if channelInfo contains one or more nullptr elements.
    */
    CImplantInfo(const std::string& deviceType
        , const std::string& hardwareRevision
        , const std::string& deviceId
        , const std::string& firmwareVersion
        , const uint32_t samplingRate
        , const std::vector<CChannelInfo*>& channelInfo
    );

    /**
    * Copy constructor.
    *
    * @param o Object to copy from.
    */
    CImplantInfo(const CImplantInfo& o);

    virtual ~CImplantInfo();

    /// @return Firmware version.
    virtual std::string getFirmwareVersion() const;

    /// @return Type of the implant.
    virtual std::string getDeviceType() const;

    /// @return Hardware revision of the implant.
    virtual std::string getHardwareRevision() const;

    /// @return Unique implant identifier.
    virtual std::string getDeviceId() const;

    /// @return Information of the capabilities of each channel.
    virtual const std::vector<CChannelInfo*>& getChannelInfo() const;

    /// @return Total number of channels.
    virtual size_t getChannelCount() const;

    /// @return Number of channels with measurement capabilities.
    virtual size_t getMeasurementChannelCount() const;

    /// @return Number of channels with stimulation capabilities.
    virtual size_t getStimulationChannelCount() const;

    /// @return Measurement sampling rate.
    virtual uint32_t getSamplingRate() const;

    CImplantInfo& operator=(const CImplantInfo& other);

protected:
    class CPimpl;
    CPimpl* m_pimpl;

private:
    CImplantInfo();
};

}}

#endif //IMPLANTAPI_IMPLANT_INFO_H