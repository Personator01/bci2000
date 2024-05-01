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
#ifndef IMPLANTAPI_IIMPLANT_FACTORY_H
#define IMPLANTAPI_IIMPLANT_FACTORY_H
#include "cppapi/ExternalUnitInfo.h"
#include "cppapi/ImplantInfo.h"
#include "cppapi/IImplant.h"
#include <vector>


namespace cortec { namespace implantapi {

/**
* @brief Device discovery and factory interface creating instances of IImplant.
*
* As BIP is intended as a platform technology, there will be several different hardware
* abstraction layer implementations. The implementations of this factory interface are capable to
* - discover all external units and implants that are connected to the system and are recognized by the factory
* - create an instance of a suitable hardware abstraction layer implementation for a specified
*   external unit and implant combination.
*/
class IImplantFactory
{
public:
    virtual ~IImplantFactory() {}

    /**
    * Method for discovering the external units that are connected to this system and recognized by the concrete
    * IImplantFactory object. The ownership of CExternalUnitInfo objects are passed to the callee.
    *
    * @return List of all connected external units, empty list if no external units are currently connected.
    */
    virtual std::vector<CExternalUnitInfo*> getExternalUnitInfos() = 0;

    /**
    * Method for discovering the connected implant of an external unit.
    * The ownership of CImplantInfo is passed to the callee.
    *
    * @param[in] externalUnitInfo External unit information of the device of interest.
    *
    * @return ImplantInfo Details on the implant connected to the given external unit.
    *
    * @throws CInvalidArgumentException if the factory is not responsible for implant type.
    * @throws CRuntimeException if the hardware revision or firmware version are not supported by this factory or
    *         if hardware info was out of date.
    */
    virtual CImplantInfo* getImplantInfo(const CExternalUnitInfo& externalUnitInfo) = 0;

    /**
    * Determine if the factory is responsible for a certain external unit.
    *
    * @param[in] externalUnitInfo Information describing the external unit.
    *
    * @return True if the factory can handle the type of the external unit, false otherwise.
    */
    virtual bool isResponsibleFactory(const implantapi::CExternalUnitInfo& externalUnitInfo) const = 0;

    /**
    * Creates an object that provides the interfaces of IImplant for the devices specified by implant ID (in implantInfo)
    * and external unit ID (in externalUnitInfo). This method fully initializes the system, so that when the call 
    * is finished, the system is ready for commands.
    * The ownership of the IImplant object is passed to the callee.
    *
    * @param[in] externalUnitInfo Information that specifies the external unit that shall be initiated.
    * @param[in] implantInfo Information that specifies the implant that shall be initiated.
    *
    * @return An object that provides the interfaces of IImplant. The ownership of the object is passed to the callee.
    *
    * @throws CRuntimeException If no external units or implants are found with the respective Info, or if no suitable
    *         hardware abstraction layer exists.
    */
    virtual IImplant* create(const CExternalUnitInfo& externalUnitInfo, const CImplantInfo& implantInfo) = 0;

    /**
    * Retrieve and return the version of the implant API.
    *
    * @return Version string of the implant API.
    */
    virtual std::string getLibraryVersion() const = 0;
};

}}


#endif //IMPLANTAPI_IIMPLANT_FACTORY_H