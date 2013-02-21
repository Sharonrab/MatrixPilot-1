#ifndef _MYIPADSB_C_
#define _MYIPADSB_C_

#include "defines.h"
#if (USE_NETWORK == 1) && (NETWORK_USE_ADSB == 1)

#include "TCPIP_Stack/TCPIP.h"
#include "MyIpData.h"
#include "MyIpADSB.h"
#include "euler_angles.h"
#include "MyIpHelpers.h"

//////////////////////////
// Module Variables
DWORD taskTimer1_ADSB[MAX_NUM_INSTANCES_OF_MODULES];
DWORD taskTimer2_ADSB[MAX_NUM_INSTANCES_OF_MODULES];

static ROM BYTE CALLSIGN[] = "ADS-B CallSign";

typedef struct {
    char callSign[sizeof (CALLSIGN)];
    INT32 gpsLat;
    INT32 gpsLong;
    INT32 heading;
    INT32 altitude;
    INT32 groundSpeed;
    INT32 climbRate;
} MyIpADSBtype;

void MyIpOnConnect_ADSB(BYTE s) {
    // Print any one-time connection annoucement text
    StringToSocket(s, "\r\nYou've connected to ADSB on "); // 33 chars
    StringToSocket(s, ID_LEAD_PILOT); // 15ish chars
    StringToSocket(s, "'s aircraft. More info at "); // 26 chars
    StringToSocket(s, ID_DIY_DRONES_URL); // 45ish chars
    StringToSocket(s, "\r\n"); // 2 chars
    MyIpData[s].sendPacket = TRUE; // send right away
}

void MyIpInit_ADSB(BYTE s) {
    // This gets called once for every socket we're configured to use for this module.
    BYTE i = MyIpData[s].instance;
    taskTimer1_ADSB[i] = GenerateRandomDWORD() % (TICK_SECOND);
    taskTimer2_ADSB[i] = GenerateRandomDWORD() % (TICK_SECOND);
}

void MyIpService_ADSB(BYTE s) {
    // don't bother queuing data if no one is listening
    if (FALSE == MyIpIsConnectedSocket(s))
        return;

    BYTE i = MyIpData[s].instance;
    MyIpADSBtype data;

    memcpy(data.callSign, CALLSIGN, sizeof (CALLSIGN));
    data.gpsLat = lat_gps.WW;
    data.gpsLong = long_gps.WW;
    data.heading = get_geo_heading_angle();
    data.altitude = alt_sl_gps.WW;
    data.groundSpeed = air_speed_3DIMU;
    data.climbRate = 156; //climb_gps.BB;

    // PACKET # 1 (even)
    if (MyIpData[s].port == 3001) {
        // Option 1, sending a structure
        if ((TickGet() - taskTimer1_ADSB[i]) > ((TICK_SECOND) / 2)) // 2Hz
        {
            taskTimer1_ADSB[i] = TickGet();
            ArrayToSocket(s, (BYTE*) & data, sizeof (data));
            MyIpData[s].sendPacket = TRUE;
        }
    } else if (MyIpData[s].port == 3002) {
        // Option 2, sending a packet organized however we'd like
        if ((TickGet() - taskTimer1_ADSB[i]) > ((TICK_SECOND) / 2) && // 2Hz
            (TCPIsPutReady(MyIpData[s].socket) >= (sizeof(data) + 12)))
        {
            taskTimer1_ADSB[i] = TickGet();

            // sending an encoded bitstream (example)
            ByteToSocket(s, 0xAA); // some sort of header
            ByteToSocket(s, 0xAB); // some sort of header
            ByteToSocket(s, 0xAC); // some sort of header
            ByteToSocket(s, sizeof (data)); // data length

            ArrayToSocket(s, (BYTE*) data.callSign, sizeof (data.callSign));
            /*
            ByteToSocket(s, data.gpsLat.v[0]);
            ByteToSocket(s, data.gpsLat.v[1]);
            ByteToSocket(s, data.gpsLat.v[2]);
            ByteToSocket(s, data.gpsLat.v[3]);
            ByteToSocket(s, data.gpsLong.v[0]);
            ByteToSocket(s, data.gpsLong.v[1]);
            ByteToSocket(s, data.gpsLong.v[2]);
            ByteToSocket(s, data.gpsLong.v[3]);
            ByteToSocket(s, data.heading.v[0]);
            ByteToSocket(s, data.heading.v[1]);
            ByteToSocket(s, data.altitude.v[0]);
            ByteToSocket(s, data.altitude.v[1]);
            ByteToSocket(s, data.groundSpeed.v[0]);
            ByteToSocket(s, data.groundSpeed.v[1]);
            ByteToSocket(s, data.climbRate);
            */

            ByteToSocket(s, 0xAD); // some sort of footer, maybe CRC or end-of-packet flag?
            ByteToSocket(s, 0xAE); // some sort of footer, maybe CRC or end-of-packet flag?
            MyIpData[s].sendPacket = TRUE;
        }
    } else if (MyIpData[s].port == 3003) {
        // Option 3, ASCII data which is human readable (great for debugging)
        if ((TickGet() - taskTimer1_ADSB[i]) > ((TICK_SECOND) / 2)) // 2Hz
        {
            taskTimer1_ADSB[i] = TickGet();

            //ByteToSocket(s, 12); //12 is a form feed
            //StringToSocket(s, "even packet\r\n\r\n");

            StringToSocket(s, data.callSign); ByteToSocket(s, ',');
            ltoaSocket(s, data.gpsLat); ByteToSocket(s, ',');
            ltoaSocket(s, data.gpsLong); ByteToSocket(s, ',');
            ltoaSocket(s, data.heading);  ByteToSocket(s, ',');
            ltoaSocket(s, data.altitude); ByteToSocket(s, ',');
            ltoaSocket(s, data.groundSpeed); ByteToSocket(s, ',');
            ltoaSocket(s, data.climbRate); StringToSocket(s, "\r\n");
            
        }
    }


    // Packet #2 (odd)
    /*
    if ((TickGet() - taskTimer2_ADSB[i]) > (TICK_SECOND)) // 1Hz
    {
        taskTimer2_ADSB[i] = TickGet();
        // generate other data in a different packet at a different time interval
        StringToSocket(s, "odd packet\r\n");
    }
    */
}

BOOL MyIpThreadSafeSendPacketCheck_ADSB(BYTE s, BOOL doClearFlag) {
    // since this data comes from, and goes to, the idle thread we
    // don't need to deal with any thread issues
    BOOL sendpacket = MyIpData[s].sendPacket;
    if (doClearFlag) {
        MyIpData[s].sendPacket = FALSE;
    }
    return sendpacket;
}

int MyIpThreadSafeReadBufferHead_ADSB(BYTE s) {
    // since this data comes from, and goes to, the idle thread we
    //  don't need to deal with any thread issues
    return MyIpData[s].buffer_head;
}

void MyIpProcessRxData_ADSB(BYTE s) {
    BYTE rxData;
    BOOL successfulRead;

    do {
        if (eTCP == MyIpData[s].type) {
            successfulRead = TCPGet(MyIpData[s].socket, &rxData);
        } else //if (eUDP == MyIpData[s].type)
        {
            successfulRead = UDPGet(&rxData);
        }

        if (successfulRead) {
            // do something with rxData
            // No Rx data parsing implemented
        }
    } while (successfulRead);
}


#endif // #if (USE_NETWORK == 1) && (NETWORK_USE_ADSB == 1)
#endif // _MYIPADSB_C_
