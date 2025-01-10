#ifndef AODVNEIGHBORETX_H
#define AODVNEIGHBORETX_H

#include <map>
#include "ns3/ipv4-address.h"
#include "ns3/aodv-packet.h"

namespace ns3
{
namespace aodv
{

class NeighborEtx
{
public:
  NeighborEtx ();
  struct Etx
  {
    uint16_t m_lppMyCnt10bMap;
    uint8_t m_lppReverse;
    Etx () : m_lppMyCnt10bMap (0), m_lppReverse (0) {}
  };
  
  //Returns current time stamp (it is needed for sending LPP packet, used as LPP ID)
  uint8_t GetLppTimeStamp () {return m_lppTimeStamp; }
  // This function is used to prepare for new cicle of sending LPP packets.
  // It clears oldest LPP count data and moves to the new time stamp.
  // Tese two, the oldest and this new current time stamp are not used for
  // calculation of ETX metrix (previous 10 time stamps is used for ETX calculations) 
  void GotoNextTimeStampAndClearOldest ();
  // Fills all ETX data from the neighbors map in the LPP packet header 
  void FillLppCntData (LppHeader &lppHeader);
  // When receive LPP from neighbor node updates my lpp count for that neigbor, and
  // also reads all data from lpp packet header and if finds his IP address then also
  // updates lpp reverse count
  bool UpdateNeighborEtx (Ipv4Address addr, uint8_t lppTimeStamp, uint8_t lppReverse);
  // Look for neighbor and return its ETX, return etx ->oo (max of uint32_t) if there
  // is no neigbor in the map (this is unlikely since it will receive at least
  // one LPP packet from this neighbor and therefore neighbor will be in the map).
  uint32_t GetEtxForNeighbor (Ipv4Address addr);
  // Return max posible ETX value, it is max (uint32_t)
  static uint32_t EtxMaxValue () { return UINT32_MAX; };
private:
  std::map<Ipv4Address, Etx> m_neighborEtx;
  uint8_t m_lppTimeStamp; // has to be incremented every llp time period; holds last 10 events  
  
  uint32_t CalculateBinaryShiftedEtx (struct Etx etxStruct);
  uint8_t Lpp10bMapToCnt (uint16_t lpp10bMap);
  void GotoNextLppTimeStamp ();
  static uint8_t CalculateNextLppTimeStamp (uint8_t currTimeStamp);
};

} // namespace aodv
} // namespace ns3


#endif /* AODVNEIGHBORETX_H */

