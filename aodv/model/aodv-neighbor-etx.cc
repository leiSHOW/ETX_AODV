#include "aodv-neighbor-etx.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <math.h>
#include <stdint.h>

namespace ns3
{
  
NS_LOG_COMPONENT_DEFINE ("AodvNeighborEtx");

namespace aodv
{

//将LPP时间戳初始化为0，时间戳是0-11循环的
NeighborEtx::NeighborEtx () : m_lppTimeStamp (0) {}

uint8_t
NeighborEtx::CalculateNextLppTimeStamp (uint8_t currTimeStamp)
{
  uint8_t nextTimeStamp = currTimeStamp + 1;
  if (nextTimeStamp > 11)
    {
      nextTimeStamp = 0;
    }
  return nextTimeStamp;
}


//根据当前的时间戳 currTimeStamp 计算下一个时间戳
void NeighborEtx::GotoNextLppTimeStamp ()
{
  m_lppTimeStamp = CalculateNextLppTimeStamp (m_lppTimeStamp);
}

//将LPP包头中的10位映射（lpp10bMap）转换为计数值【通过检查每个位，计算在当前时间戳和下一个时间戳之外的有效LPP计数】
uint8_t NeighborEtx::Lpp10bMapToCnt (uint16_t lpp10bMap)
{
  uint8_t lpp = 0;
  for (int j=0; j<12; ++j)
    {
      if ((j!=m_lppTimeStamp) && (j!=(m_lppTimeStamp+1)))
        {
          lpp += (lpp10bMap & ( (uint16_t)0x0001 << j) ) ? 1 : 0;
        }
    }
  return lpp;
}

//更新到下一个时间戳，并清除最旧的时间戳的LPP计数数据
//CalculateNextLppTimeStamp获取下一个LPP时间戳
void NeighborEtx::GotoNextTimeStampAndClearOldest ()
{
  GotoNextLppTimeStamp (); // go to  next timestamp which becomes current timestamp
  for (std::map<Ipv4Address, Etx>::iterator i = m_neighborEtx.begin (); i != m_neighborEtx.end (); ++i)
    {
      Etx etx = i->second;
      uint16_t lppMyCnt10bMap = etx.m_lppMyCnt10bMap;
      // Delete oldest timestamp lpp count (this is next time stamp compared to current)
      // Only lower 12 bits are used
      lppMyCnt10bMap &= (uint16_t)(~((uint16_t)0x0001 << CalculateNextLppTimeStamp (m_lppTimeStamp)) & (uint16_t)0x0FFF);
      etx.m_lppMyCnt10bMap = lppMyCnt10bMap;
      i->second = etx;
    }  
}

//将邻居的LPP计数数据填充到LPP包头中
//对于每个邻居，计算其有效的LPP计数，并将其添加到LPP包头中的邻居列表
void NeighborEtx::FillLppCntData (LppHeader &lppHeader)
{
  for (std::map<Ipv4Address, Etx>::iterator i = m_neighborEtx.begin (); i != m_neighborEtx.end (); ++i)
        {
          uint8_t lpp = Lpp10bMapToCnt (i->second.m_lppMyCnt10bMap);
          if (lpp > 0) lppHeader.AddToNeighborsList (i->first, lpp);
        }
}


//更新邻居的ETX数据
//如果该邻居不存在，则插入新数据。如果已存在，则更新其LPP计数和反向计数
bool NeighborEtx::UpdateNeighborEtx (Ipv4Address addr, uint8_t lppTimeStamp, uint8_t lppReverse)
{
  std::map<Ipv4Address, Etx>::iterator i = m_neighborEtx.find (addr);
  if (i == m_neighborEtx.end ())
    {
      // No address, insert new entry
      Etx etx;
      etx.m_lppReverse = lppReverse;
      etx.m_lppMyCnt10bMap = (uint16_t)0x0001 << lppTimeStamp;
      std::pair<std::map<Ipv4Address, Etx>::iterator, bool> result = m_neighborEtx.insert (std::make_pair (addr, etx));
      return result.second;
    }
  else
    {
      // Address found, update existing entry
      i->second.m_lppReverse = lppReverse;
      (i->second.m_lppMyCnt10bMap) |= ((uint16_t)0x0001 << lppTimeStamp);
      return true;
    }
}

//计算ETX值，基于链路的LPP计数和反向计数
//ETX的计算使用LPP计数和反向计数的乘积来评估链路的质量：链路成功传输的成功率越高，ETX值越低
uint32_t NeighborEtx::CalculateBinaryShiftedEtx (Etx etxStruct)
{
  uint32_t etx = UINT32_MAX;
  if ((Lpp10bMapToCnt (etxStruct.m_lppMyCnt10bMap)!=0) && (etxStruct.m_lppReverse!=0))
    {
      etx = (uint32_t) (round (1000000.0 / (Lpp10bMapToCnt (etxStruct.m_lppMyCnt10bMap) * etxStruct.m_lppReverse)));
    }
  //NS_LOG_UNCOND ("ETX binary: " << etx);
  return etx;
}


//获取指定邻居的ETX值
//如果该邻居的ETX数据已存储在邻居表中，则返回对应的ETX值；如果该邻居不存在，则返回最大值UINT32_MAX，表示链路质量极差或不可用
uint32_t NeighborEtx::GetEtxForNeighbor (Ipv4Address addr)
{
  uint32_t etx;
  std::map<Ipv4Address, Etx>::iterator i = m_neighborEtx.find (addr);
  if (i == m_neighborEtx.end ())
    {
      // No address, ETX -> oo (= UINT32_MAX)
      etx = UINT32_MAX;
      return etx;
    }
  else
    {
      // Address found, calculate and return current ETX value
      return CalculateBinaryShiftedEtx (i->second);
    }
}



} // namespace aodv
} // namespace ns3
