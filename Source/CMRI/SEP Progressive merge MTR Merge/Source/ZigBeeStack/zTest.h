/*

Home Control, Lighting Profile Definition File

This ZigBee Profile header file provides all of the constants needed to
implement a device that is conforms to the ZigBee Home Control, Lighting
profile.

Mandatory Clusters shall be used in all devices and require all attributes
to be full implemented with the designated data type.

Optional Clusters may be unsupported.  However, if a cluster is supported,
then all of that cluster's attributes shall be supported.

Refer to the ZigBee Specification for the Mandatory and Optional clusters
for each device.


NOTES:
(1) Simple Bind must be supported. See [APS] End Device Bind sections for more details.
(2) Simple Binding required on Light Sensor Monochromatic and Occupancy Sensor Inputs

 *********************************************************************
 * FileName:        zHCLighting.h
 * Dependencies:
 * Processor:       PIC18
 * Complier:        MCC18 v3.00 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PIC® microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PIC microcontroller products.
 *
 * You may not modify or create derivatives works of the Software.
 *
 * If you intend to use the software in a product for sale, then you must
 * be a member of the ZigBee Alliance and obtain any licenses required by
 * them.  For more information, go to www.zigbee.org.
 *
 * The software is owned by the Company and/or its licensor, and is
 * protected under applicable copyright laws. All rights are reserved.
 *
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *
 * Author               Date    Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * DF/KO                04/29/05 Microchip ZigBee Stack v1.0-2.0
 * DF/KO                07/18/05 Microchip ZigBee Stack v1.0-3.0
 * DF/KO                07/27/05 Microchip ZigBee Stack v1.0-3.1
 * DF/KO                08/19/05 Microchip ZigBee Stack v1.0-3.2
 * DF/KO                01/09/06 Microchip ZigBee Stack v1.0-3.5
 * DF/KO                08/31/06 Microchip ZigBee Stack v1.0-3.6
*/

#ifndef _ZTEST_H_
#define _ZTEST_H_

#include "zStack_Profile.h"
#include "zStack_Configuration.h"

#endif
