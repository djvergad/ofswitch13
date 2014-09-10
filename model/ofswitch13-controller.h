/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Luciano Chaves <luciano@lrc.ic.unicamp.br>
 */

#ifndef OFSWITCH13_CONTROLLER_H
#define OFSWITCH13_CONTROLLER_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/tcp-socket-factory.h"
#include "ofswitch13-interface.h"
#include "ofswitch13-net-device.h"

namespace ns3 {

class OFSwitch13NetDevice;
class OFSwitch13Helper;

/**
 * \ingroup ofswitch13
 * \brief An OpenFlow 1.3 controller for OFSwitch13NetDevice devices
 * \attention Currently, It is not full-compliant with the protocol
 * specification. 
 */
class OFSwitch13Controller : public Application
{
public:
  OFSwitch13Controller ();
  virtual ~OFSwitch13Controller ();

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * \briefRegister the OFSwitch13Helper used to create the network.
   * \param helper The helper pointer
   */
  void SetOFSwitch13Helper (Ptr<OFSwitch13Helper> helper);

  /**
   * \brief Create a flow_mod message using the same syntax from dpctl, and
   * send it to the switch.
   * \param swtch The Ptr<OFSwitch13NetDevice> switch to register.
   * \param textCmd The dpctl flow_mod command to create the message.
   */
  void SendFlowModMsg (Ptr<OFSwitch13NetDevice> sw, const char* textCmd);

private:
  // inherited from Application
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \internal
   * Get the packet type on the buffer, which can then be used
   * to determine how to handle the buffer.
   *
   * \param buffer The packet in OpenFlow buffer format.
   * \return The packet type, as defined in the ofp_type struct.
   */
  uint8_t GetPacketType (ofpbuf* buffer);

  /**
   * \internal
   * \brief Handles any ofpbuf received from switch
   * \param swtch The switch the message was received from.
   * \param buffer The pointer to the buffer containing the message.
   */
  virtual void ReceiveFromSwitch (Ptr<OFSwitch13NetDevice> swtch, ofpbuf* buffer);

  /**
   * \internal
   * Send a message to a registered switch. It will encapsulate the ofl_msg
   * format into an ofpbuf wire format and send it over a TCP socekt to the
   * proper switch IP address.
   * \param swtch The switch NetDevice to receive the message.
   * \param msg The message to send.
   */
  void SendToSwitch (Ptr<OFSwitch13NetDevice> swtch, void *msg);

  /**
   * Handlers used as socket callbacks to TCP communication between this
   * controller and the switches.
   */
  //\{
  void HandleRead       (Ptr<Socket> socket);                       //!< Receive packet from switch
  bool HandleRequest    (Ptr<Socket> s, const Address& from);       //!< TCP request from switch
  void HandleAccept     (Ptr<Socket> socket, const Address& from);  //!< TCP handshake succeeded
  void HandlePeerClose  (Ptr<Socket> socket);                       //!< TCP connection closed
  void HandlePeerError  (Ptr<Socket> socket);                       //!< TCP connection error
  //\}
  
  static const uint32_t m_global_xid = 0xf0ff00f0;  //!< Global transaction idx
  uint16_t              m_port;                     //!< Local controller tcp port
  
  Ptr<Socket>           m_serverSocket;             //!< Listening server socket
  Ptr<OFSwitch13Helper> m_helper;                   //!< OpenFlow helper
  
  typedef std::map<uint32_t, Ptr<Socket> > SocketsMap_t;
  SocketsMap_t m_socketsMap;                        //!< Map of accepted sockets from switches
};

} // namespace ns3
#endif /* OFSWITCH13_CONTROLLER_H */
