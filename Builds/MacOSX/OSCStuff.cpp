#include <iostream>
#include <cstring>
#include "OSCStuff.h"
#include <math.h>

#if defined(__BORLANDC__) // workaround for BCB4 release build intrinsics bug
namespace std {
    using ::__strcmp__;  // avoid error: E2316 '__strcmp__' is not a member of 'std'.
}
#endif

#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"

void GesturePacketListener::ProcessMessage( const osc::ReceivedMessage& m) {
    try{
        if( std::strcmp( m.AddressPattern(), "/0/raw" ) == 0 ){
            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
            int battery = (arg++)->AsInt32();
            
            this->switchState = (arg++)->AsInt32();
            int accX = (arg++)->AsInt32();
            this->accX = (arg++)->AsInt32();
            int accZ =(arg++)->AsInt32();
            
            accelMag = sqrt(pow(accX,2) + pow(this->accX, 2) + pow(accZ, 2));
            
            int gyroY = (arg++)->AsInt32();
            int gyroP = (arg++)->AsInt32();
            int gyroR =(arg++)->AsInt32();
            
            int magnetX = (arg++)->AsInt32();
            int magnetY = (arg++)->AsInt32();
            int magnetZ =(arg++)->AsInt32();
        }
        if( std::strcmp( m.AddressPattern(), "/0/euler" ) == 0 ){
            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
            this->yaw = (arg++)->AsFloat();
            this->pitch = (arg++)->AsFloat();
            this->roll = (arg++)->AsFloat();
        }
        if( std::strcmp( m.AddressPattern(), "/0/quat" ) == 0 ){
            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
            this->q1 = (arg++)->AsFloat();
            this->q2 = (arg++)->AsFloat();
            this->q3 = (arg++)->AsFloat();
            this->q3 = (arg++)->AsFloat();
        }
    }catch( osc::Exception& e ){
        std::cout << "error while parsing message: "
        << m.AddressPattern() << ": " << e.what() << "\n";
    }
}


float standard_deviation(float data[], int n){
    float mean=0.0, sum_deviation=0.0;
    int i;
    for(i=0; i<n;++i) {
        mean+=data[i];
    }
    mean=mean/n;
    for(i=0; i<n;++i)
        sum_deviation+=(data[i]-mean)*(data[i]-mean);
    return sqrt(sum_deviation/n);
}

float minMax(float data[], int n){
    float min=200000, max = 0.0;
    int i;
    for(i=0; i<n;++i)
    {
        min= std::min(data[i], min);
        max = std::max(data[i], max);
    }

    return std::abs(max-min)/10;
}

void BitalinoPacketListener::ProcessMessage( const osc::ReceivedMessage& m )  {
    
    try{        
        if( std::strcmp( m.AddressPattern(), "/wek/inputs" ) == 0 ){
            
            osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
            float emg = (arg++)->AsFloat();
            float emgFilt = (arg++)->AsFloat();
            float eda = (arg++)->AsFloat();
            
            this->emgBuffer[this->curEmgBuffIdx++] = emgFilt;
            if (this->curEmgBuffIdx >= BUFFER_SIZE) {
                this->curEmgBuffIdx = 0;
            }
            
            this->emgStd = minMax(this->emgBuffer, BUFFER_SIZE);
            
            float ecg = (arg++)->AsFloat();
            this->accelMean =(arg++)->AsFloat();
            this->lux = (arg++)->AsFloat();
            
            //std::cout << "emg: " << emg << " emgfilt: " << emgFilt << "  eda: " << eda << "  ecg: " << ecg << "  accel: " << accel << "  lux: " << lux << '\n';
            
        }
    }catch( osc::Exception& e ){
        // any parsing errors such as unexpected argument types, or
        // missing arguments get thrown as exceptions.
        std::cout << "error while parsing message: "
        << m.AddressPattern() << ": " << e.what() << "\n";
    }
}

SocketListener::SocketListener(osc::OscPacketListener * listener, int port):Thread("SocketListenerThread"){
    this->port = port;
    this->listener = listener;
}

void SocketListener::startListening() {
    socket = new DatagramSocket (this->port, false);
    startThread();
}

void SocketListener::stopListening() {
    signalThreadShouldExit();
    socket = nullptr;
    waitForThreadToExit(0);
    
}

void SocketListener::run() {
    char buffer[1024];
    
    while (! threadShouldExit()) {
        if (socket->waitUntilReady(true, -1) != 1)
            break;
        
        if (threadShouldExit())
            break;
        
        int len = socket->read (buffer, 1024, false);
        if (len < 1)
            break;
        
        listener->ProcessPacket(buffer, len);
        
    }
}