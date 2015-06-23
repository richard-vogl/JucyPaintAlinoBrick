/*
 Example of two different ways to process received OSC messages using oscpack.
 Receives the messages from the SimpleSend.cpp example.
 */
#pragma once

#include <iostream>
#include <cstring>
#include "../JuceLibraryCode/JuceHeader.h"

#if defined(__BORLANDC__) // workaround for BCB4 release build intrinsics bug
namespace std {
    using ::__strcmp__;  // avoid error: E2316 '__strcmp__' is not a member of 'std'.
}
#endif

#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"


#define PORT_GESTURE 8888
#define PORT_BITALINO 6448

#define BUFFER_SIZE 100

class GesturePacketListener : public osc::OscPacketListener {
public:
    
    float getPitch() {
        return pitch;
    }
    float getYaw() {
        return yaw;
    }
    float getRoll() {
        return roll;
    }
    float getAccelMag() {
        return accelMag;
    }
    float getSwitchState() {
        return switchState;
    }
    
    float getAccX() {
        return accX;
    }
    
    float getQ1() {
        return q1;
    }
    
    float getQ2() {
        return q2;
    }
    
    float getQ3() {
        return q3;
    }
    
    float getQ4() {
        return q4;
    }
protected:
    
    virtual void ProcessMessage( const osc::ReceivedMessage& m);
    
private:
    float pitch = 0;
    float yaw = 0;
    float roll = 0;
    float accX = 0;
    
    float accelMag=0;
    
    float switchState=1;
    
    float q1=0;
    float q2=0;
    float q3=0;
    float q4=0;
};

class BitalinoPacketListener : public osc::OscPacketListener {
public:
    
    float getEmgStd(){return emgStd;};
    float getAccelMean(){return accelMean;};
    float getLux(){return lux;};
    float * getEmgBuffer() {
        return emgBuffer;
    }
    
    int getCurEmgBuffIdx() { return curEmgBuffIdx;}
protected:

    
    virtual void ProcessMessage( const osc::ReceivedMessage& m);
    
private:
    float emgStd=0;
    float accelMean=0;
    float lux=10;
    
    float emgBuffer[BUFFER_SIZE];
    int curEmgBuffIdx = 0;
};


class SocketListener : public Thread {
private:
    ScopedPointer<DatagramSocket> socket;
    int port;
    osc::OscPacketListener * listener;
    
    void run() override;    
public:
    SocketListener(osc::OscPacketListener * listener, int port);
    void startListening();
    void stopListening();

};