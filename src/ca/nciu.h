/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
*     National Laboratory.
* Copyright (c) 2002 The Regents of the University of California, as
*     Operator of Los Alamos National Laboratory.
* EPICS BASE Versions 3.13.7
* and higher are distributed subject to a Software License Agreement found
* in file LICENSE that is included with this distribution. 
\*************************************************************************/
/*  
 *  $Id$
 *
 *                              
 *                    L O S  A L A M O S
 *              Los Alamos National Laboratory
 *               Los Alamos, New Mexico 87545
 *                                  
 *  Copyright, 1986, The Regents of the University of California.
 *                                  
 *           
 *	Author Jeffrey O. Hill
 *	johill@lanl.gov
 *	505 665 1831
 */

#ifndef nciuh  
#define nciuh

#ifdef epicsExportSharedSymbols
#   define nciuh_restore_epicsExportSharedSymbols
#   undef epicsExportSharedSymbols
#endif

#include "resourceLib.h"
#include "tsDLList.h"
#include "tsFreeList.h"
#include "epicsMutex.h"
#include "compilerDependencies.h"

#ifdef nciuh_restore_epicsExportSharedSymbols
#   define epicsExportSharedSymbols
#   include "shareLib.h"
#endif

#define CA_MINOR_PROTOCOL_REVISION 11
#include "caProto.h"

#include "cacIO.h"

class cac;
class netiiu;

// The node and the state which tracks the list membership
// are in the channel, but belong to the circuit.
// Protected by the callback mutex
class channelNode : public tsDLNode < class nciu >
{
public:
    channelNode ();
    bool isConnected ( epicsGuard < epicsMutex > & ) const;
private:
    enum channelState {
        cs_none,
        cs_disconnGov,
        cs_serverAddrResPend,
        cs_createReqPend,
        cs_createRespPend,
        cs_subscripReqPend,
        cs_connected,
        cs_unrespCircuit,
        cs_subscripUpdateReqPend
    } listMember;
    friend class tcpiiu;
    friend class tcpSendThread;
    friend class udpiiu;
};

class privateInterfaceForIO {
public:
    virtual void ioCompletionNotify ( 
        epicsGuard < epicsMutex > &, class baseNMIU & ) = 0;
    virtual arrayElementCount nativeElementCount ( 
        epicsGuard < epicsMutex > & ) const = 0;
    virtual bool connected ( epicsGuard < epicsMutex > & ) const = 0;
};

class nciu : 
    public cacChannel,
    public chronIntIdRes < nciu >, 
    public channelNode,
    private privateInterfaceForIO {
public:
    nciu ( cac &, netiiu &, cacChannelNotify &, 
        const char * pNameIn, cacChannel::priLev );
    void destructor ( 
        epicsGuard < epicsMutex > & cbGuard,
        epicsGuard < epicsMutex > & guard );
    void connect ( unsigned nativeType, 
        unsigned nativeCount, unsigned sid, 
        epicsGuard < epicsMutex > & cbGuard, 
        epicsGuard < epicsMutex > & guard );
    void connect ( epicsGuard < epicsMutex > & cbGuard, 
        epicsGuard < epicsMutex > & guard );
    void unresponsiveCircuitNotify ( 
        epicsGuard < epicsMutex > & cbGuard, 
        epicsGuard < epicsMutex > & guard );
    void circuitHangupNotify ( class udpiiu &, 
        epicsGuard < epicsMutex > & cbGuard, 
        epicsGuard < epicsMutex > & guard );
    void setServerAddressUnknown ( 
        udpiiu & newiiu, epicsGuard < epicsMutex > & guard );
    bool searchMsg ( class udpiiu & iiu, unsigned & retryNoForThisChannel );
    void beaconAnomalyNotify ();
    void serviceShutdownNotify ();
    void accessRightsStateChange ( const caAccessRights &, 
        epicsGuard < epicsMutex > & cbGuard, 
        epicsGuard < epicsMutex > & guard );
    ca_uint32_t getSID () const;
    ca_uint32_t getCID () const;
    netiiu * getPIIU ();
    const netiiu * getConstPIIU () const;
    cac & getClient ();
    void searchReplySetUp ( netiiu &iiu, unsigned sidIn, 
        ca_uint16_t typeIn, arrayElementCount countIn,
        epicsGuard < epicsMutex > & );
    void show ( unsigned level ) const;
    const char * pName () const;
    unsigned nameLen () const;
    const char * pHostName () const; // deprecated - please do not use
    void writeException ( 
        epicsGuard < epicsMutex > &, epicsGuard < epicsMutex > &,
        int status, const char *pContext, unsigned type, arrayElementCount count );
    cacChannel::priLev getPriority () const;
    void * operator new ( 
        size_t size, tsFreeList < class nciu, 1024, epicsMutexNOOP > & );
    epicsPlacementDeleteOperator (
        ( void *, tsFreeList < class nciu, 1024, epicsMutexNOOP > & ))
    arrayElementCount nativeElementCount ( epicsGuard < epicsMutex > & ) const;
    void resubscribe ( epicsGuard < epicsMutex > & );
    void sendSubscriptionUpdateRequests ( epicsGuard < epicsMutex > & );
    void disconnectAllIO ( 
        epicsGuard < epicsMutex > &, epicsGuard < epicsMutex > & );
    bool connected ( epicsGuard < epicsMutex > & ) const;

protected:
    ~nciu ();

private:
    tsDLList < class baseNMIU > eventq;
    caAccessRights accessRightState;
    cac & cacCtx;
    char * pNameStr;
    netiiu * piiu;
    ca_uint32_t sid; // server id
    unsigned count;
    unsigned retry; // search retry number
    unsigned short nameLength; // channel name length
    ca_uint16_t typeCode;
    ca_uint8_t priority; 
    virtual void destroy (
        epicsGuard < epicsMutex > & callbackControlGuard, 
        epicsGuard < epicsMutex > & mutualExclusionGuard );
    void initiateConnect (
        epicsGuard < epicsMutex > & );
    ioStatus read ( 
        epicsGuard < epicsMutex > &,
        unsigned type, arrayElementCount count, 
        cacReadNotify &, ioid * );
    void write ( 
        epicsGuard < epicsMutex > &,
        unsigned type, arrayElementCount count, 
        const void *pValue );
    ioStatus write ( 
        epicsGuard < epicsMutex > &,
        unsigned type, arrayElementCount count, 
        const void *pValue, cacWriteNotify &, ioid * );
    void subscribe ( 
        epicsGuard < epicsMutex > & guard, 
        unsigned type, arrayElementCount nElem, 
        unsigned mask, cacStateNotify &notify, ioid * );
    virtual void ioCancel ( 
        epicsGuard < epicsMutex > & callbackControlGuard, 
        epicsGuard < epicsMutex > & mutualExclusionGuard,
        const ioid & );
    void ioShow ( 
        const ioid &, unsigned level ) const;
    short nativeType () const;
    caAccessRights accessRights () const;
    unsigned searchAttempts () const;
    double beaconPeriod () const;
    double receiveWatchdogDelay () const;
    bool ca_v42_ok () const;
    void hostName ( char *pBuf, unsigned bufLength ) const;
    arrayElementCount nativeElementCount () const;
    static void stringVerify ( const char *pStr, const unsigned count );
    virtual void ioCompletionNotify ( 
        epicsGuard < epicsMutex > &, class baseNMIU & );
	nciu ( const nciu & );
	nciu & operator = ( const nciu & );
    void * operator new ( size_t );
    void operator delete ( void * );
};

inline void * nciu::operator new ( size_t size, 
    tsFreeList < class nciu, 1024, epicsMutexNOOP > & freeList )
{ 
    return freeList.allocate ( size );
}

#ifdef CXX_PLACEMENT_DELETE
inline void nciu::operator delete ( void * pCadaver,
    tsFreeList < class nciu, 1024, epicsMutexNOOP > & freeList )
{ 
    freeList.release ( pCadaver, sizeof ( nciu ) );
}
#endif

inline ca_uint32_t nciu::getSID () const
{
    return this->sid;
}

inline ca_uint32_t nciu::getCID () const
{
    return this->id;
}

// this is to only be used by early protocol revisions
inline void nciu::connect ( epicsGuard < epicsMutex > & cbGuard, 
                epicsGuard < epicsMutex > & guard )
{
    this->connect ( this->typeCode, this->count, 
        this->sid, cbGuard, guard );
}

inline void nciu::searchReplySetUp ( netiiu &iiu, unsigned sidIn, 
    ca_uint16_t typeIn, arrayElementCount countIn,
    epicsGuard < epicsMutex > & )
{
    this->piiu = & iiu;
    this->typeCode = typeIn;      
    this->count = countIn;
    this->sid = sidIn;
}

inline netiiu * nciu::getPIIU ()
{
    return this->piiu;
}

inline void nciu::writeException ( 
    epicsGuard < epicsMutex > & cbGuard, 
    epicsGuard < epicsMutex > & guard, 
    int status, const char * pContext, 
    unsigned typeIn, arrayElementCount countIn )
{
    this->notify().writeException ( guard, 
        status, pContext, typeIn, countIn );
}

inline const netiiu * nciu::getConstPIIU () const
{
    return this->piiu;
}

inline void nciu::serviceShutdownNotify ()
{
    this->notify().serviceShutdownNotify ();
}

inline cac & nciu::getClient ()
{
    return this->cacCtx;
}

inline cacChannel::priLev nciu::getPriority () const
{
    return this->priority;
}

inline channelNode::channelNode () :
    listMember ( cs_none )
{
}

inline bool channelNode::isConnected ( epicsGuard < epicsMutex > & ) const
{
    return 
        this->listMember == cs_connected || 
        this->listMember == cs_subscripReqPend ||
        this->listMember == cs_subscripUpdateReqPend;
}

#endif // ifdef nciuh
