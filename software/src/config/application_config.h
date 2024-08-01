#ifndef CE86C311_A664_4DCC_A0AF_E526343F9478
#define CE86C311_A664_4DCC_A0AF_E526343F9478

/* IP to lora LNS (e.g TTN)*/
#define LORA_LNS_IP "52.212.223.226"
/* Port to the lora LNS (e.g TTN )*/
#define LORA_LNS_PORT 1700
/* How often send the gateway status to the lora lns*/
#define LORA_STATUS_INTERVAL_MINUTES 5


/* Intervall to sync the pico time with sntp time*/
#define SNTP_INTERVAL_MINUTES 120


/* IP to send SIM status information */
#define STATUS_SERVER_IP "45.83.104.74"
/* Port to send SIM status information */
#define STATUS_SERVER_PORT 7074
/* Intervall how often send SIM status information*/
#define STATUS_INTERVAL_MINUTES 2



#endif /* CE86C311_A664_4DCC_A0AF_E526343F9478 */
