//
// Generated file, do not edit! Created by nedtool 4.6 from messages/MacPkt.msg.
//

#ifndef _MACPKT_M_H_
#define _MACPKT_M_H_

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0406
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
#include "SimpleAddress.h"
// }}

/**
 * Class generated from <tt>messages/MacPkt.msg:36</tt> by nedtool.
 * <pre>
 * // A basic MAC (Media Access Control) packet format definition
 * // 
 * // subclass if you want to create your own MAC layer packet class
 * //
 * // The basic MAC packet only provides source and destination address
 * //
 * // @author Daniel Willkomm
 * packet MacPkt
 * {
 *     LAddress::L2Type destAddr; // destination mac address
 *     LAddress::L2Type srcAddr;  // source mac address
 *     long sequenceId; // Sequence Number to detect duplicate messages
 * }
 * </pre>
 */
class MacPkt : public ::cPacket
{
  protected:
    LAddress::L2Type destAddr_var;
    LAddress::L2Type srcAddr_var;
    long sequenceId_var;

  private:
    void copy(const MacPkt& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const MacPkt&);

  public:
    MacPkt(const char *name=NULL, int kind=0);
    MacPkt(const MacPkt& other);
    virtual ~MacPkt();
    MacPkt& operator=(const MacPkt& other);
    virtual MacPkt *dup() const {return new MacPkt(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual LAddress::L2Type& getDestAddr();
    virtual const LAddress::L2Type& getDestAddr() const {return const_cast<MacPkt*>(this)->getDestAddr();}
    virtual void setDestAddr(const LAddress::L2Type& destAddr);
    virtual LAddress::L2Type& getSrcAddr();
    virtual const LAddress::L2Type& getSrcAddr() const {return const_cast<MacPkt*>(this)->getSrcAddr();}
    virtual void setSrcAddr(const LAddress::L2Type& srcAddr);
    virtual long getSequenceId() const;
    virtual void setSequenceId(long sequenceId);
};

inline void doPacking(cCommBuffer *b, MacPkt& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, MacPkt& obj) {obj.parsimUnpack(b);}


#endif // ifndef _MACPKT_M_H_

