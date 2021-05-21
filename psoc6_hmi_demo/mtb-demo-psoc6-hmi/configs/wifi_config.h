/*
 * wifi_config.h
 *
 *  Created on: 16-May-2021
 *      Author: KVVaisakh
 */

#ifndef SOURCE_WIFI_CONFIG_H_
#define SOURCE_WIFI_CONFIG_H_

#define WIFI_INTERFACE_TYPE                   CY_WCM_INTERFACE_TYPE_STA

/* Wi-Fi Credentials: Modify WIFI_SSID, WIFI_PASSWORD, and WIFI_SECURITY_TYPE
 * to match your Wi-Fi network credentials.
 * Note: Maximum length of the Wi-Fi SSID and password is set to
 * CY_WCM_MAX_SSID_LEN and CY_WCM_MAX_PASSPHRASE_LEN as defined in cy_wcm.h file.
 */
#define WIFI_SSID                             "My_WiFi"
#define WIFI_PASSWORD                         "pswd@123"

/* Security type of the Wi-Fi access point. See 'cy_wcm_security_t' structure
 * in "cy_wcm.h" for more details.
 */
#define WIFI_SECURITY_TYPE                    CY_WCM_SECURITY_WPA2_AES_PSK
/* Maximum number of connection retries to a Wi-Fi network. */
#define MAX_WIFI_CONN_RETRIES                 (10u)

/* Wi-Fi re-connection time interval in milliseconds */
#define WIFI_CONN_RETRY_INTERVAL_MSEC         (1000u)


#endif /* SOURCE_WIFI_CONFIG_H_ */
