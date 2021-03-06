/*
 * File name   : UAS_comm.cpp
 *
 * Created on  : Jul 31, 2012
 *
 * Author      : Xichen Shi
 */

#include <UAS_comm.hpp>

// Defining Serial classes and mavlink_system for global use
UAS_serial Serial_apm;
UAS_serial Serial_gcs;
mavlink_system_t mavlink_system;
pthread_cond_t condition_apm_send_var = PTHREAD_COND_INITIALIZER;
pthread_cond_t condition_gcs_send_var = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_apm_send_lock   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_gcs_send_lock   = PTHREAD_MUTEX_INITIALIZER;
uint8_t  apm_tx_buf[MAVLINK_MAX_PACKET_LEN]; // Mem space for holding tx buffer for apm 
uint8_t  gcs_tx_buf[MAVLINK_MAX_PACKET_LEN]; // Mem space for holding rx buffer for gcs
uint16_t apm_tx_buf_len = 0; // length of buffer to be written for apm
uint16_t gcs_tx_buf_len = 0; // length of buffer to be written for gcs

/**
 * @brief -- Default Constructor for comm system
 */
UAS_comm::UAS_comm() {
}

/**
 * @brief -- Constructor for comm system
 * @param -- two serial classes for comm with APM and GCS
 */
UAS_comm::UAS_comm(UAS_serial* serial_apm, UAS_serial* serial_gcs) {
    _comm_apm = serial_apm; 
    _comm_gcs = serial_gcs;
}

/**
 * @brief -- initialize comm system by opening two serial ports
 * @param serial_apm -- pointer to apm serial
 * @param serial_gcs -- pointer to gcs serial
 */
void UAS_comm::init(UAS_serial* serial_apm, UAS_serial* serial_gcs){
    // Designate Ports
    _comm_apm = serial_apm; 
    _comm_gcs = serial_gcs;

    // open ports with baudrate specified
    _apm_initialised = _comm_apm->isOpened();
    _gcs_initialised = _comm_gcs->isOpened(); 

    if (_apm_initialised) {
        printf("Port to APM is initialised at %s\n",_comm_apm->getDeviceName() );
    } else {
        printf("Port to APM failed to initialised at %s\n", _comm_apm->getDeviceName());
    }
    if (_gcs_initialised) {
        printf("Port to GCS is initialised at %s\n", _comm_gcs->getDeviceName() );
    } else {
        printf("Port to GCS failed to initialised at %s\n", _comm_gcs->getDeviceName());
    }

    // assign mavlink_channels to each link
    chan_apm = MAVLINK_COMM_0; 
    chan_gcs = MAVLINK_COMM_1;
}

/**
 * @brief -- initialize comm system by opening two serial ports with defined baudrate
 * @param baudrate_apm --
 * @param baudrate_gcs --
 */
void UAS_comm::init(uint32_t baudrate_apm, uint32_t baudrate_gcs){
    // open ports with baudrate specified
    _apm_initialised = _comm_apm->beginPort(baudrate_apm);
    _gcs_initialised = _comm_gcs->beginPort(baudrate_gcs); 

    if (_apm_initialised) {
        printf("Port to APM is initialised at %s\n",_comm_apm->getDeviceName() );
    } else {
        printf("Port to APM failed to initialised at %s\n", _comm_apm->getDeviceName());
    }
    if (_gcs_initialised) {
        printf("Port to GCS is initialised at %s\n", _comm_gcs->getDeviceName() );
    } else {
        printf("Port to GCS failed to initialised at %s\n", _comm_gcs->getDeviceName());
    }

    // assign mavlink_channels to each link
    chan_apm = MAVLINK_COMM_0; 
    chan_gcs = MAVLINK_COMM_1;
}

/**
 * @brief -- initialize comm system by opening two serial ports with defined name and baudrate
 * @param serial_name_apm -- device name for apm serial link
 * @param serial_name_gcs -- device name for gcs serial link
 * @param baudrate_apm --
 * @param baudrate_gcs --
 */
void UAS_comm::init(const char* serial_name_apm, const char* serial_name_gcs, uint32_t baudrate_apm, uint32_t baudrate_gcs){
    // open ports with baudrate specified
    _apm_initialised = _comm_apm->beginPort(serial_name_apm, baudrate_apm);
    _gcs_initialised = _comm_gcs->beginPort(serial_name_gcs, baudrate_gcs); 

    if (_apm_initialised) {
        printf("Port to APM is initialised at %s\n",_comm_apm->getDeviceName() );
    } else {
        printf("Port to APM failed to initialised at %s\n", _comm_apm->getDeviceName());
    }
    if (_gcs_initialised) {
        printf("Port to GCS is initialised at %s\n", _comm_gcs->getDeviceName() );
    } else {
        printf("Port to GCS failed to initialised at %s\n", _comm_gcs->getDeviceName());
    }

    // assign mavlink_channels to each link
    chan_apm = MAVLINK_COMM_0; 
    chan_gcs = MAVLINK_COMM_1;
}

/**
 * @brief -- check the incoming byte from APM
 * @return -- NULL
 */
void UAS_comm::updateApm(){
    mavlink_message_t msg;
    mavlink_status_t status;
    unsigned char c;

    while(_comm_apm->getIncomingBytes()>0){
        _comm_apm->fetch(&c);
        if (mavlink_parse_char(chan_apm, c, &msg, &status)){
            //printf("APM: message received: %d\n",msg.msgid );
            parseApmMessage(&msg);
        }
    }

    _apm_packet_drops += status.packet_rx_drop_count;
}

/**
 * @brief -- check the incoming byte from GCS
 * @return -- NULL
 */
void UAS_comm::updateGcs(){
    mavlink_message_t msg;
    mavlink_status_t status;
    unsigned char c;

    while(_comm_gcs->getIncomingBytes()>0){
        _comm_gcs->fetch(&c);
        if (mavlink_parse_char(chan_gcs, c, &msg, &status)){
            //printf("GCS: message received: %d\n",msg.msgid );
            parseGcsMessage(&msg);
        }
    }

    _gcs_packet_drops += status.packet_rx_drop_count;
}

/**
 * @brief -- pass-through non-needed message to specified channel 
 * @param chan -- channel to send (apm or gcs)
 * @param msg -- the msg to be sent
 * @return -- NULL
 */
void UAS_comm::bypassMessage(uint8_t chan, mavlink_message_t* pMsg){
    uint8_t buf[MAVLINK_MAX_PACKET_LEN]; 
    uint16_t len = mavlink_msg_to_send_buffer(buf, pMsg);

    if (chan == chan_apm){
        pthread_mutex_lock(&mutex_apm_send_lock);
        for (int i = 0; i < len; ++i)
        {
            apm_tx_buf[i] = buf[i];
        }
        apm_tx_buf_len = len;
        pthread_cond_signal(&condition_apm_send_var);
        pthread_mutex_unlock(&mutex_apm_send_lock);
    }
    if (chan == chan_gcs){
        pthread_mutex_lock(&mutex_gcs_send_lock);
        for (int i = 0; i < len; ++i)
        {
            gcs_tx_buf[i] = buf[i];
        }
        gcs_tx_buf_len = len;
        pthread_cond_signal(&condition_gcs_send_var);
        pthread_mutex_unlock(&mutex_gcs_send_lock);
    }
}

/**
 * @brief -- parse mavlink_msg after the char is processed
 * @param msg -- the msg to be parsed
 * @return -- NULL
 */
void UAS_comm::parseApmMessage(mavlink_message_t* pMsg){
    switch (pMsg->msgid){
    case MAVLINK_MSG_ID_HEARTBEAT:
        mavlink_heartbeat_t heartbeat;
        mavlink_msg_heartbeat_decode(pMsg, &heartbeat);
        bypassMessage(chan_gcs, pMsg);
        break;

    case MAVLINK_MSG_ID_ATTITUDE:
        mavlink_attitude_t attitude;
        mavlink_msg_attitude_decode(pMsg, &attitude);
        updateAttitude(attitude.roll,
                       attitude.pitch,
                       attitude.yaw, 
                       attitude.rollspeed,
                       attitude.pitchspeed, 
                       attitude.yawspeed);
        bypassMessage(chan_gcs, pMsg);
        break;

    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
        mavlink_global_position_int_t global_position;
        mavlink_msg_global_position_int_decode(pMsg, &global_position);
        updateGlobalPosition(global_position.lat, 
                             global_position.lon, 
                             global_position.alt, 
                             global_position.relative_alt, 
                             global_position.vx, 
                             global_position.vy, 
                             global_position.vz,  
                             global_position.hdg);
        bypassMessage(chan_gcs, pMsg);
        break;

    default:
        if (isGcsOpen()) { 
            bypassMessage(chan_gcs, pMsg); // pass the message to gcs
        }
        break;
    } // end switch
}

/**
 * @brief -- parse mavlink_msg after the char is processed
 * @param msg -- the msg to be parsed
 * @return -- NULL
 */
void UAS_comm::parseGcsMessage(mavlink_message_t* pMsg){
    switch (pMsg->msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT:
        mavlink_heartbeat_t heartbeat;
        mavlink_msg_heartbeat_decode(pMsg, &heartbeat);
        bypassMessage(chan_apm, pMsg);
        break;

    default:
        if (isApmOpen()) {
            bypassMessage(chan_apm, pMsg); // pass the msg to apm
        }
        break;
    } // end switch
} 

/**
 * @brief -- if apm port is opened
 */
bool UAS_comm::isApmOpen(){
    return _apm_initialised;
}

/**
 * @brief -- if gcs port is opened
 */
bool UAS_comm::isGcsOpen(){
    return _gcs_initialised;
}