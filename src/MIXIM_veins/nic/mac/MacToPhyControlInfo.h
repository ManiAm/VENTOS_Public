#ifndef MACTOPHYCONTROLINFO_H_
#define MACTOPHYCONTROLINFO_H_

#include "global/MiXiMDefs.h"
#include "MIXIM_veins/nic/phy/Signal_.h"
#include "ConstsPhy.h"


/**
 * @brief Stores information which is needed by the physical layer
 * when sending a MacPkt.
 *
 * @ingroup phyLayer
 * @ingroup macLayer
 */

class MIXIM_API MacToPhyControlInfo: public omnetpp::cObject
{
protected:

    uint64_t bitrate;
    double power;
    double freq;
    enum Veins::PHY_MCS mcs;

public:

    /**
     * @brief Initialize the MacToPhyControlInfo.
     */
    MacToPhyControlInfo() {
        this->bitrate = 0;
        this->power = -1;
        this->freq = -1;
        this->mcs = Veins::MCS_DEFAULT;
    }

    virtual ~MacToPhyControlInfo()
    {

    }

    uint64_t getBitrate() {return this->bitrate;};
    double getPower() {return this->power;};
    double getFreq() {return this->freq;};
    enum Veins::PHY_MCS getMCS() {return this->mcs;};

    void setBitrate(uint64_t bitrate) {this->bitrate = bitrate;};
    void setPower(double power) {this->power = power;};
    void setFreq(double freq) {this->freq = freq;};
    void setMCS(enum Veins::PHY_MCS mcs) {this->mcs = mcs;};
};

#endif /*MACTOPHYCONTROLINFO_H_*/
