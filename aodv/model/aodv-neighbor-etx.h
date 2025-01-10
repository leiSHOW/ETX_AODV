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
  
  uint8_t GetLppTimeStamp () {return m_lppTimeStamp; }
  void GotoNextTimeStampAndClearOldest ();
  void FillLppCntData (LppHeader &lppHeader);
  bool UpdateNeighborEtx (Ipv4Address addr, uint8_t lppTimeStamp, uint8_t lppReverse);
  uint32_t GetEtxForNeighbor (Ipv4Address addr);
  static uint32_t EtxMaxValue () { return UINT32_MAX; };
private:
  std::map<Ipv4Address, Etx> m_neighborEtx;
  uint8_t m_lppTimeStamp; 
  
  uint32_t CalculateBinaryShiftedEtx (struct Etx etxStruct);
  uint8_t Lpp10bMapToCnt (uint16_t lpp10bMap);
  void GotoNextLppTimeStamp ();
  static uint8_t CalculateNextLppTimeStamp (uint8_t currTimeStamp);
};

} // namespace aodv
} // namespace ns3


#endif /* AODVNEIGHBORETX_H */

