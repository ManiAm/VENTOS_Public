#ifndef SIGNAL_H_
#define SIGNAL_H_

#include <list>
#include <omnetpp.h>
#include <boost/serialization/access.hpp>

#include "global/MiXiMDefs.h"
#include "Mapping.h"


/**
 * @brief The signal class stores the physical representation of the
 * signal of an AirFrame.
 *
 * This includes start, duration and propagation delay of the signal,
 * Mappings which represent the transmission power, bitrate, attenuation
 * caused by effects of the channel on the signal during its transmission
 * and the receiving power.
 *
 * Note: Although the Signal itself has a signalSendingStart parameter the
 * Mappings it contains should use absolute time positions to store the values
 * at (NOT relative to the start time of the signal).
 *
 * The Signal is created at the senders MAC layer which has to define
 * the TX-power- and the bitrate Mapping.
 * Sending start time and duration is added at the sender's physical layer.
 * Attenuation Mappings are added to the Signal by the
 * AnalogueModels of the receiver's physical layer.
 * The RX-power Mapping is calculated on demand by multiplying the
 * TX-power Mapping with every attenuation Mapping of the signal.
 *
 * @ingroup phyLayer
 */

class MIXIM_API Signal
{
public:

    /**
     * @brief Shortcut type for a concatenated Mapping using multiply operator.
     *
     * Used to define the receiving power mapping.
     */
    typedef ConcatConstMapping<std::multiplies<double> > MultipliedMapping;

    /** @brief Shortcut type for a list of ConstMappings.*/
    typedef std::list<ConstMapping*> ConstMappingList;

protected:

    /** @brief The propagation delay of the transmission. */
    double propagationDelay;

    /** @brief Stores the function which describes the power of the signal*/
    ConstMapping* power;

    /** @brief Stores the function which describes the bitrate of the signal*/
    Mapping* bitrate;

    /** @brief If propagation delay is not zero this stores the undelayed bitrate*/
    Mapping* txBitrate;

    /** @brief Stores the functions describing the attenuation of the signal*/
    ConstMappingList attenuations;

    /** @brief Stores the mapping defining the receiving power of the signal.*/
    MultipliedMapping* rcvPower;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        archive & propagationDelay;
        archive & power;
        archive & bitrate;
        archive & txBitrate;
        archive & attenuations;
        archive & rcvPower;
    }

public:

    /**
     * @brief Initializes a signal with the specified start and length.
     */
    Signal() {

        this->propagationDelay = 0;
        this->power = NULL;
        this->bitrate = NULL;
        this->txBitrate = NULL;
        this->attenuations = ConstMappingList();
        this->rcvPower = NULL;
    }

    /**
     * @brief Overwrites the copy constructor to make sure that the
     * mappings are cloned correct.
     */
    Signal(const Signal& o)
    {
        this->propagationDelay = o.propagationDelay;
        this->power = NULL;
        this->bitrate = NULL;
        this->txBitrate = NULL;
        this->attenuations = ConstMappingList();
        this->rcvPower = NULL;

        if (o.power)
            power = o.power->constClone();

        if (o.bitrate)
            bitrate = o.bitrate->clone();

        if (o.txBitrate)
            txBitrate = o.txBitrate->clone();

        for(ConstMappingList::const_iterator it = o.attenuations.begin(); it != o.attenuations.end(); it++)
            attenuations.push_back((*it)->constClone());
    }

    /**
     * @brief Delete the functions of this signal.
     */
    ~Signal()
    {
        if(rcvPower)
        {
            if(propagationDelay != 0)
            {
                assert(rcvPower->getRefMapping() != power);
                delete rcvPower->getRefMapping();
            }

            delete rcvPower;
        }

        if(power)
            delete power;

        if(bitrate)
            delete bitrate;

        if(txBitrate)
            delete txBitrate;

        for(ConstMappingList::iterator it = attenuations.begin(); it != attenuations.end(); it++)
            delete *it;
    }

    /**
     * @brief Overwrites the copy operator to make sure that the
     * mappings are cloned correct.
     */
    const Signal& operator=(const Signal& o)
    {
        propagationDelay = o.propagationDelay;

        markRcvPowerOutdated();

        if(power)
        {
            delete power;
            power = NULL;
        }

        if(bitrate)
        {
            delete bitrate;
            bitrate = NULL;
        }

        if(txBitrate)
        {
            delete txBitrate;
            txBitrate = NULL;
        }

        if(o.power)
            power = o.power->constClone();

        if(o.bitrate)
            bitrate = o.bitrate->clone();

        if(o.txBitrate)
            txBitrate = o.txBitrate->clone();

        for(ConstMappingList::const_iterator it = attenuations.begin(); it != attenuations.end(); ++it)
            delete(*it);

        attenuations.clear();

        for(ConstMappingList::const_iterator it = o.attenuations.begin(); it != o.attenuations.end(); ++it)
            attenuations.push_back((*it)->constClone());

        return *this;
    }

    void swap(Signal& s)
    {
        std::swap(propagationDelay,  s.propagationDelay);
        std::swap(power,             s.power);
        std::swap(bitrate,           s.bitrate);
        std::swap(txBitrate,         s.txBitrate);
        std::swap(attenuations,      s.attenuations);
        std::swap(rcvPower,          s.rcvPower);
    }

    omnetpp::simtime_t getPropagationDelay() const
    {
        return propagationDelay;
    }

    /**
     * @brief Returns the function representing the transmission power
     * of the signal.
     *
     * Be aware that the transmission power mapping is not yet affected
     * by the propagation delay!
     */
    ConstMapping* getTransmissionPower()
    {
        return power;
    }

    /**
     * @brief Returns the function representing the transmission power
     * of the signal.
     *
     * Be aware that the transmission power mapping is not yet affected
     * by the propagation delay!
     */
    const ConstMapping* getTransmissionPower() const
    {
        return power;
    }

    /**
     * @brief Returns the function representing the bitrate of the
     * signal.
     */
    Mapping* getBitrate()
    {
        return bitrate;
    }

    /**
     * @brief Returns a list of functions representing the attenuation
     * of the signal.
     */
    const ConstMappingList& getAttenuation() const
    {
        return attenuations;
    }

    /**
     * @brief Calculates and returns the receiving power of this Signal.
     * Ownership of the returned mapping belongs to this class.
     *
     * The receiving power is calculated by multiplying the transmission
     * power with the attenuation of every receiving phys AnalogueModel.
     */
    MultipliedMapping* getReceivingPower()
    {
        if(!rcvPower)
        {
            ConstMapping* tmp = power;
            if(propagationDelay != 0)
            {
                tmp = new ConstDelayedMapping(power, propagationDelay);
            }

            rcvPower = new MultipliedMapping(tmp, attenuations.begin(), attenuations.end(), false, Argument::MappedZero());
        }

        return rcvPower;
    }

    /**
     * @brief Sets the propagation delay of the signal.
     *
     * This should be only set by the sending physical layer.
     */
    void setPropagationDelay(omnetpp::simtime_t_cref delay)
    {
        assert(propagationDelay == 0);
        assert(!txBitrate);

        markRcvPowerOutdated();

        propagationDelay = delay.dbl();

        if(bitrate)
        {
            txBitrate = bitrate;
            bitrate = new DelayedMapping(txBitrate, propagationDelay);
        }
    }

    /**
     * @brief Sets the function representing the transmission power
     * of the signal.
     *
     * The ownership of the passed pointer goes to the signal.
     */
    void setTransmissionPower(ConstMapping* power)
    {
        if(this->power)
        {
            markRcvPowerOutdated();
            delete this->power;
        }

        this->power = power;
    }

    /**
     * @brief Sets the function representing the bitrate of the signal.
     *
     * The ownership of the passed pointer goes to the signal.
     */
    void setBitrate(Mapping* bitrate)
    {
        assert(!txBitrate);

        if(this->bitrate)
            delete this->bitrate;

        this->bitrate = bitrate;
    }

    /**
     * @brief Adds a function representing an attenuation of the signal.
     *
     * The ownership of the passed pointer goes to the signal.
     */
    void addAttenuation(ConstMapping* att)
    {
        //assert the attenuation wasn't already added to the list before
        assert(std::find(attenuations.begin(), attenuations.end(), att) == attenuations.end());

        attenuations.push_back(att);

        if(rcvPower)
            rcvPower->addMapping(att);
    }

protected:

    /**
     * @brief Deletes the rcvPower mapping member because it became
     * out-dated.
     *
     * This happens when transmission power or propagation delay changes.
     */
    void markRcvPowerOutdated()
    {
        if(rcvPower)
        {
            if(propagationDelay != 0)
            {
                assert(rcvPower->getRefMapping() != power);
                delete rcvPower->getRefMapping();
            }

            delete rcvPower;
            rcvPower = 0;
        }
    }
};

#endif /*SIGNAL_H_*/
