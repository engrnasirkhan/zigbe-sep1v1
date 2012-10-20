/*********************************************************************
 *
 *                  ZCL Power Configuration Header File
 *
 *********************************************************************
 * FileName        : ZCL_Power_Configuration.h
 * Dependencies    :
 * Processor       : PIC18 / PIC24 / dsPIC33
 * Complier        : MCC18 v3.00 or higher
 *                   MCC30 v2.05 or higher
 * Company         : Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright © 2004-2007 Microchip Technology Inc.  All rights reserved.
 *
 * Microchip licenses to you the right to use, copy and distribute Software
 * only when embedded on a Microchip microcontroller or digital signal
 * controller and used with a Microchip radio frequency transceiver, which
 * are integrated into your product or third party product (pursuant to the
 * sublicense terms in the accompanying license agreement).  You may NOT
 * modify or create derivative works of the Software.
 *
 * If you intend to use this Software in the development of a product for
 * sale, you must be a member of the ZigBee Alliance.  For more information,
 * go to www.zigbee.org.
 *
 * You should refer to the license agreement accompanying this Software for
 * additional information regarding your rights and obligations.
 *
 * SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY
 * OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR
 * PURPOSE. IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED
 * UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF
 * WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR
 * EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT,
 * PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY
 * THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER
 * SIMILAR COSTS.
 *
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 ********************************************************************/
#ifndef _ZCL_POWER_CONFIGURATION_H
#define _ZCL_POWER_CONFIGURATION_H

/*****************************************************************************
                                Includes
 *****************************************************************************/
 /* None */

/*****************************************************************************
                          Constants and Enumerations
 *****************************************************************************/
#define ZCL_POWER_CONFIGURATION_CLUSTER                     0x0001

/* Mains Information Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_MAINS_VOLTAGE                               0x0000
    #define ZCL_MAINS_FREQUENCY                             0x0001
#endif
/* Mains Settings Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_MAINS_ALARM_MASK                            0x0010
    #define ZCL_MAINS_VOLTAGE_MIN_THRESHOLD                 0x0011
    #define ZCL_MAINS_VOLTAGE_MAX_THRESHOLD                 0x0012
    #define ZCL_MAINS_VOLTAGE_DWELL_TRIP_POINT              0x0013
#endif

/* Battery Information Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_BATTERY_VOLTAGE                             0x0020
#endif

/* Battery Settings Attribute Set */
#if (I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 0x01 )
    #define ZCL_BATTERY_MANUFACTURER                        0x0030
    #define ZCL_BATTERY_SIZE                                0x0031
    #define ZCL_BATTERY_A_HR_RATING                         0x0032
    #define ZCL_BATTERY_QUANTITY                            0x0033
    #define ZCL_BATTERY_RATED_VOLTAGE                       0x0034
    #define ZCL_BATTERY_ALARM_MASK                          0x0035
    #define ZCL_BATTERY_VOLTAGE_MINIMUM_THRESHOLD           0x0036
#endif

 enum PowerConfiguration_AlarmCodes
 {
    MainsVoltageTooLow                                      = 0x00,
    MainsVoltageTooHigh                                     = 0x01,
    BatteryVoltageTooLow                                    = 0x10

 };

/*****************************************************************************
                          Customizable Macros
 *****************************************************************************/
 /* None */

 /*****************************************************************************
                          Variable definitions
 *****************************************************************************/
 extern ROM ZCL_ClusterDefinition zcl_PowerConfigurationClusterServerDefinition;
 extern ROM ZCL_ClusterDefinition zcl_PowerConfigurationClusterClientDefinition;

 extern ROM ZCL_ClusterInfo zcl_PowerConfigurationClusterServerInfo;
 extern ROM ZCL_ClusterInfo zcl_PowerConfigurationClusterClientInfo;

 /*****************************************************************************
                            Data Structures
 *****************************************************************************/
/* Structure for holding basic cluster attributes */
typedef struct __attribute__((packed,aligned(1))) _ZCL_PowerConfigurationClusterAttributes
{
#if I_SUPPORT_OPTIONAL_ZCL_ATTRIBUTES == 1
    WORD mainsVoltage;
    BYTE mainsFrequency;
    BYTE mainsAlarmMask;
    WORD mainsVoltageMinThreshold;
    WORD mainsVoltageMaxThreshold;
    WORD mainsVoltageDwellTripPoint;
    BYTE batteryVoltage;
    BYTE batteryManfacturer[16];
    BYTE batterySize;
    WORD batteryAHrRating;
    BYTE batteryQuantity;
    BYTE batteryRatedVoltage;
    BYTE batteryAlarmMask;
    BYTE batteryVoltageMinThreshold;
#endif
} ZCL_PowerConfigurationClusterAttributes;
  /*****************************************************************************
                          Function Prototypes
 *****************************************************************************/
 /******************************************************************************
 * Function:        BYTE ZCL_HandlePowerConfigurationClusterCommand
 *                  (
 *                      BYTE endPointId,
 *                      BYTE* pReceivedAsdu,
 *                      BYTE* pResponseAsduStartLocation,
 *                      ZCL_ActionHandler* pActionHandler,
 *                      ROM ZCL_ClusterInfo* pZCLClusterInfo
 *                  );
 *
 * PreCondition:    None
 *
 * Input:           endPointId - EndPoint on which the command is received
 *                  pReceivedAsdu - This point to the actual data received.
 *                                  This consists of ZCL header and Payload.
 *                  pZCLClusterInfo - Pointer pointing to the Power
 *                                    configuration Cluster info.
 *
 * Output:          pResponseAsduStartLocation - Pointer pointing to the start
 *                                               location of asdu for which the
 *                                               response frame needs to be
 *                                               created.
 *                  pActionHandler -   This is used to notify the application
 *                                     that a response frame is generated or not.
 *                                     It also holds the information of the type
 *                                     event to be handled by the application
 *                                     and has the payload of the recevied data.
 *
 * Return :         Returns zero data since there is no commands in Power
 *                  configuration cluster
 *
 * Side Effects:    None
 *
 * Overview:        This function is called by the ZCL module if a cluster
 *                  specific command with power configuration cluster is
 *                  received. There are no commands in power configuration
 *                  cluster. For error handling, this function is implemented.
 *                  This creates default response command with status Unsupported
 *                  cluster command.
 *
 * Note:            None
 *****************************************************************************/
BYTE ZCL_HandlePowerConfigurationClusterCommand
(
    BYTE endPointId,
    BYTE asduLength,
    BYTE* pReceivedAsdu,
    BYTE* pResponseAsduStartLocation,
    ZCL_ActionHandler *pActionHandler,
    ROM ZCL_ClusterInfo *pZCLClusterInfo
);
#endif /* _ZCL_POWER_CONFIGURATION_H */
