/*******************************************************************************
* File Name: cycfg_capsense.c
* Version: 3.0
*
* Description:
* CapSense Middleware configuration
* This file should not be modified. It was automatically generated by
* CapSense Configurator 3.0.0.2188
*
********************************************************************************
* Copyright 2020 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include "cycfg_capsense.h"

#if !defined(CY_DISABLE_CAPSENSE)

#if (CY_CAPSENSE_CORE == __CORTEX_M)

#ifndef CSD_CMODPADS_PORT
#define CSD_CMODPADS_PORT 0xFF
#endif
#ifndef CSD_CMODPADS_PIN
#define CSD_CMODPADS_PIN 0xFF
#endif
#ifndef CSD_CSH_TANKPADS_PORT
#define CSD_CSH_TANKPADS_PORT 0xFF
#endif
#ifndef CSD_CSH_TANKPADS_PIN
#define CSD_CSH_TANKPADS_PIN 0xFF
#endif
#ifndef CSD_CSHIELDPADS_PORT
#define CSD_CSHIELDPADS_PORT 0xFF
#endif
#ifndef CSD_CSHIELDPADS_PIN
#define CSD_CSHIELDPADS_PIN 0xFF
#endif
#ifndef CSD_VREF_EXT_PORT
#define CSD_VREF_EXT_PORT 0xFF
#endif
#ifndef CSD_VREF_EXT_PIN
#define CSD_VREF_EXT_PIN 0xFF
#endif

#define CY_CAPSENSE_WIDGET_COUNT                                                 (3u)
#define CY_CAPSENSE_SENSOR_COUNT                                                 (7u)
#define CY_CAPSENSE_ELTD_COUNT                                                   (7u)
#define CY_CAPSENSE_PIN_COUNT                                                    (7u)
#define CY_CAPSENSE_SHIELD_PIN_COUNT                                             (0u)
#define CY_CAPSENSE_POSITION_SIZE                                                (1u)
#define CY_CAPSENSE_DEBOUNCE_SIZE                                                (3u)
#define CY_CAPSENSE_NOISE_ENVELOPE_SIZE                                          (7u)
#define CY_CAPSENSE_RAW_HISTORY_SIZE                                             (0u)
#define CY_CAPSENSE_IIR_HISTORY_LOW_SIZE                                         (0u)
#define CY_CAPSENSE_POSITION_FILTER_HISTORY_SIZE                                 (0u)
#define CY_CAPSENSE_TOUCH_FILTER_HISTORY_SIZE                                    (0u)
#define CY_CAPSENSE_DIPLEX_SIZE                                                  (0u)
#define CY_CAPSENSE_CSD_TOUCHPAD_MAX_SENSORS_SIZE                                (0u)
#define CY_CAPSENSE_CSX_TOUCH_BUFFER_ENABLE                                      (0u)
#define CY_CAPSENSE_CSX_TOUCH_HISTORY_SIZE                                       (0u)
#define CY_CAPSENSE_BALLISTIC_WIDGET_COUNT                                       (0u)
#define CY_CAPSENSE_GESTURE_WIDGET_COUNT                                         (1u)

static cy_stc_capsense_internal_context_t cy_capsense_internalContext;
static cy_stc_active_scan_sns_t cy_capsense_activeScanSns;

#if (CY_CAPSENSE_DEBOUNCE_SIZE > 0)
    static uint8_t cy_capsense_debounce[CY_CAPSENSE_DEBOUNCE_SIZE] = {0};
#endif

#if (CY_CAPSENSE_NOISE_ENVELOPE_SIZE > 0)
    static cy_stc_capsense_smartsense_csd_noise_envelope_t cy_capsense_noiseEnvelope[CY_CAPSENSE_NOISE_ENVELOPE_SIZE];
#endif

#if (CY_CAPSENSE_RAW_HISTORY_SIZE > 0)
    static uint16_t cy_capsense_rawFilterHistory[CY_CAPSENSE_RAW_HISTORY_SIZE] = {0};
#endif

#if (CY_CAPSENSE_IIR_HISTORY_LOW_SIZE > 0)
    static uint8_t cy_capsense_iirHistoryLow[CY_CAPSENSE_IIR_HISTORY_LOW_SIZE] = {0};
#endif

#if (CY_CAPSENSE_POSITION_FILTER_HISTORY_SIZE > 0)
    static cy_stc_capsense_position_t cy_capsense_positionFilterHistory[CY_CAPSENSE_POSITION_FILTER_HISTORY_SIZE];
#endif

#if (CY_CAPSENSE_TOUCH_FILTER_HISTORY_SIZE > 0)
    static cy_stc_capsense_touch_t cy_capsense_touchFilterHistory[CY_CAPSENSE_TOUCH_FILTER_HISTORY_SIZE] =
    {
    };
#endif

#if (CY_CAPSENSE_DIPLEX_SIZE > 0)
    static const uint8_t cy_capsense_diplexTable[CY_CAPSENSE_DIPLEX_SIZE] =
    {
    };
#endif

#if (CY_CAPSENSE_CSD_TOUCHPAD_MAX_SENSORS_SIZE > 0)
    static uint16 cy_capsense_csdTouchBuffer[CY_CAPSENSE_CSD_TOUCHPAD_MAX_SENSORS_SIZE] = {0};
#endif

#if (CY_CAPSENSE_CSX_TOUCH_BUFFER_ENABLE > 0)
    static cy_stc_capsense_csx_touch_buffer_t cy_capsense_csxTouchBuffer;
#endif

#if (CY_CAPSENSE_CSX_TOUCH_HISTORY_SIZE > 0)
    static cy_stc_capsense_csx_touch_history_t cy_capsense_csxTouchHistory[CY_CAPSENSE_CSX_TOUCH_HISTORY_SIZE] =
    {
    };
#endif

#if (CY_CAPSENSE_BALLISTIC_WIDGET_COUNT > 0)
    static cy_stc_capsense_ballistic_context_t cy_capsense_ballisticContext[CY_CAPSENSE_BALLISTIC_WIDGET_COUNT];
#endif

#if (CY_CAPSENSE_GESTURE_WIDGET_COUNT > 0)
    static cy_stc_capsense_gesture_context_t cy_capsense_gestureContext[CY_CAPSENSE_GESTURE_WIDGET_COUNT];
#endif

static const cy_stc_capsense_common_config_t cy_capsense_commonConfig =
{
    .cpuClkHz = CY_CAPSENSE_CPU_CLK,
    .periClkHz = CY_CAPSENSE_PERI_CLK,
    .vdda = CY_CAPSENSE_VDDA_MV,
    .numPin = CY_CAPSENSE_PIN_COUNT,
    .numSns = CY_CAPSENSE_SENSOR_COUNT,
    .numWd = CY_CAPSENSE_WIDGET_COUNT,
    .csdEn = CY_CAPSENSE_ENABLE,
    .csxEn = CY_CAPSENSE_DISABLE,
    .mfsEn = CY_CAPSENSE_DISABLE,
#if (CY_CAPSENSE_BIST_SUPPORTED)
    .bistEn = CY_CAPSENSE_DISABLE,
#endif
    .positionFilterEn = CY_CAPSENSE_DISABLE,
    .periDividerType = CY_CAPSENSE_PERI_DIV_TYPE,
    .periDividerIndex = CY_CAPSENSE_PERI_DIV_INDEX,
    .analogWakeupDelay = 25u,
    .ssIrefSource = CY_CAPSENSE_IREF_SRSS,
    .ssVrefSource = CY_CAPSENSE_VREF_SRSS,
    .proxTouchCoeff = 300u,
    .swSensorAutoResetEn = CY_CAPSENSE_DISABLE,
    .portCmodPadNum = CSD_CMODPADS_PORT,
    .pinCmodPad = CSD_CMODPADS_PIN,
    .portCshPadNum = CSD_CSH_TANKPADS_PORT,
    .pinCshPad = CSD_CSH_TANKPADS_PIN,
    .portShieldPadNum = CSD_CSHIELDPADS_PORT,
    .pinShieldPad = CSD_CSHIELDPADS_PIN,
    .portVrefExtPadNum = CSD_VREF_EXT_PORT,
    .pinVrefExtPad = CSD_VREF_EXT_PIN,
    .portCmodNum = Cmod_PORT_NUM,
    .idacGainTable = {
        {0x01000000u,   37500u},
        {0x03000000u,   75000u},
        {0x01400000u,  300000u},
        {0x03400000u,  600000u},
        {0x01800000u, 2400000u},
        {0x03800000u, 4800000u},
    },
    .ptrCsdBase = CSD0,
    .ptrCsdContext = &cy_csd_0_context,
    .portCmod = Cmod_PORT,
    .portCsh = NULL,
    .portCintA = NULL,
    .portCintB = NULL,
    .pinCmod = Cmod_PIN,
    .portCshNum = 0u,
    .pinCsh = 0u,
    .pinCintA = 0u,
    .pinCintB = 0u,
    .csdShieldEn = CY_CAPSENSE_DISABLE,
    .csdInactiveSnsConnection = CY_CAPSENSE_SNS_CONNECTION_GROUND,
    .csdShieldDelay = CY_CAPSENSE_SH_DELAY_0NS,
    .csdVref = 0u,
    .csdRConst = 1000u,
    .csdCTankShieldEn = CY_CAPSENSE_DISABLE,
    .csdShieldNumPin = 0u,
    .csdShieldSwRes = CY_CAPSENSE_SHIELD_SW_RES_MEDIUM,
    .csdInitSwRes = CY_CAPSENSE_INIT_SW_RES_MEDIUM,
    .csdChargeTransfer = CY_CAPSENSE_IDAC_SOURCING,
    .csdRawTarget = 85u,
    .csdAutotuneEn = CY_CAPSENSE_CSD_SS_HWTH_EN,
    .csdIdacAutocalEn = CY_CAPSENSE_ENABLE,
    .csdIdacAutoGainEn = CY_CAPSENSE_ENABLE,
    .csdCalibrationError = 10u,
    .csdIdacGainInitIndex = 4u,
    .csdIdacMin = 20u,
    .csdIdacCompEn = CY_CAPSENSE_ENABLE,
    .csdFineInitTime = 10u,
    .csdIdacRowColAlignEn = CY_CAPSENSE_ENABLE,
    .csdMfsDividerOffsetF1 = 1u,
    .csdMfsDividerOffsetF2 = 2u,
    .csxRawTarget = 40u,
    .csxIdacGainInitIndex = 2u,
    .csxIdacAutocalEn = CY_CAPSENSE_DISABLE,
    .csxCalibrationError = 20u,
    .csxFineInitTime = 10u,
    .csxInitSwRes = CY_CAPSENSE_INIT_SW_RES_MEDIUM,
    .csxScanSwRes = CY_CAPSENSE_INIT_SW_RES_LOW,
    .csxInitShieldSwRes = CY_CAPSENSE_SHIELD_SW_RES_MEDIUM,
    .csxScanShieldSwRes = CY_CAPSENSE_SHIELD_SW_RES_LOW,
    .csxMfsDividerOffsetF1 = 1u,
    .csxMfsDividerOffsetF2 = 2u,
};

static const cy_stc_capsense_fptr_config_t cy_capsense_fptrConfig =
{
#if (CY_CAPSENSE_CSD_EN == 0)
    .fptrCSDSetupWidget = NULL,
    .fptrCSDScan = NULL,
    .fptrDpProcessCsdWidgetRawCounts = NULL,
    .fptrDpProcessCsdWidgetStatus = NULL,
    .fptrCSDDisableMode = NULL,
    .fptrCSDInitialize = NULL,
    .fptrCSDScanISR = NULL,
#else
    .fptrCSDSetupWidget = &Cy_CapSense_CSDSetupWidget,
    .fptrCSDScan = &Cy_CapSense_CSDScan,
    .fptrDpProcessCsdWidgetRawCounts = &Cy_CapSense_DpProcessCsdWidgetRawCounts,
    .fptrDpProcessCsdWidgetStatus = &Cy_CapSense_DpProcessCsdWidgetStatus,
    .fptrCSDDisableMode = &Cy_CapSense_CSDDisableMode,
    .fptrCSDInitialize = &Cy_CapSense_CSDInitialize,
    .fptrCSDScanISR = &Cy_CapSense_CSDScanISR,
#endif

#if (CY_CAPSENSE_CSX_EN == 0)
    .fptrCSXSetupWidget = NULL,
    .fptrCSXScan = NULL,
    .fptrDpProcessCsxWidgetRawCounts = NULL,
    .fptrDpProcessCsxWidgetStatus = NULL,
    .fptrCSXInitialize = NULL,
    .fptrCSXDisableMode = NULL,
    .fptrCSXScanISR = NULL,
#else
    .fptrCSXSetupWidget = &Cy_CapSense_CSXSetupWidget,
    .fptrCSXScan = &Cy_CapSense_CSXScan,
    .fptrDpProcessCsxWidgetRawCounts = &Cy_CapSense_DpProcessCsxWidgetRawCounts,
    .fptrDpProcessCsxWidgetStatus = &Cy_CapSense_DpProcessCsxWidgetStatus,
    .fptrCSXInitialize = &Cy_CapSense_CSXInitialize,
    .fptrCSXDisableMode = &Cy_CapSense_CSXDisableMode,
    .fptrCSXScanISR = &Cy_CapSense_CSXScanISR,
#endif

#if (CY_CAPSENSE_ADAPTIVE_FILTER_EN == 0)
    .fptrAdaptiveFilterInitializeLib = NULL,
    .fptrAdaptiveFilterRunLib = NULL,
#else
    .fptrAdaptiveFilterInitializeLib = &Cy_CapSense_AdaptiveFilterInitialize_Lib,
    .fptrAdaptiveFilterRunLib = &Cy_CapSense_AdaptiveFilterRun_Lib,
#endif

#if (CY_CAPSENSE_BALLISTIC_MULTIPLIER_EN == 0)
    .fptrBallisticMultiplierLib = NULL,
#else
    .fptrBallisticMultiplierLib = &Cy_CapSense_BallisticMultiplier_Lib,
#endif

#if (CY_CAPSENSE_RAWCOUNT_FILTER_EN == 0)
    .fptrInitializeAllFilters = NULL,
    .fptrFtRunEnabledFiltersInternal = NULL,
#else
    .fptrInitializeAllFilters = &Cy_CapSense_InitializeAllFilters,
    .fptrFtRunEnabledFiltersInternal = &Cy_CapSense_FtRunEnabledFiltersInternal,
#endif

#if (CY_CAPSENSE_CSD_POSITION_FILTER_EN == 0)
    .fptrProcessPositionFilters = NULL,
#else
    .fptrProcessPositionFilters = &Cy_CapSense_ProcessPositionFilters,
#endif

#if ((CY_CAPSENSE_CSX_POSITION_FILTER_EN == 0) && (CY_CAPSENSE_CSD_POSITION_FILTER_EN == 0))
    .fptrRunPositionFilters = NULL,
    .fptrInitPositionFilters = NULL,
#else
    .fptrRunPositionFilters = &Cy_CapSense_RunPositionFilters,
    .fptrInitPositionFilters = &Cy_CapSense_InitPositionFilters,
#endif

#if ((CY_CAPSENSE_CSD_BUTTON_EN == 0) && \
     (CY_CAPSENSE_CSX_BUTTON_EN == 0) && (CY_CAPSENSE_CSX_MATRIX_EN == 0))
    .fptrDpProcessButton = NULL,
#else
    .fptrDpProcessButton = &Cy_CapSense_DpProcessButton,
#endif

#if (CY_CAPSENSE_CSD_SLIDER_EN == 0)
    .fptrDpProcessSlider = NULL,
#else
    .fptrDpProcessSlider = &Cy_CapSense_DpProcessSlider,
#endif

#if (CY_CAPSENSE_CSD_MATRIX_EN == 0)
    .fptrDpProcessCsdMatrix = NULL,
#else
    .fptrDpProcessCsdMatrix = &Cy_CapSense_DpProcessCsdMatrix,
#endif

#if (CY_CAPSENSE_CSD_TOUCHPAD_EN == 0)
    .fptrDpProcessCsdTouchpad = NULL,
#else
    .fptrDpProcessCsdTouchpad = &Cy_CapSense_DpProcessCsdTouchpad,
#endif

#if (CY_CAPSENSE_CSD_PROXIMITY_EN == 0)
    .fptrDpProcessProximity = NULL,
#else
    .fptrDpProcessProximity = &Cy_CapSense_DpProcessProximity,
#endif

#if (CY_CAPSENSE_CSX_TOUCHPAD_EN == 0)
    .fptrDpProcessCsxTouchpad = NULL,
#else
    .fptrDpProcessCsxTouchpad = &Cy_CapSense_DpProcessCsxTouchpad,
#endif

#if (CY_CAPSENSE_ADVANCED_CENTROID_5X5_EN == 0)
    .fptrDpAdvancedCentroidTouchpad = NULL,
#else
    .fptrDpAdvancedCentroidTouchpad = &Cy_CapSense_DpAdvancedCentroidTouchpad,
#endif

#if ((CY_CAPSENSE_CSD_CALIBRATION_EN == 0) && (CY_CAPSENSE_SMARTSENSE_FULL_EN == 0))
    .fptrCSDCalibrateWidget = NULL,
    .fptrCalibrateAllCsdWidgets = NULL,
#else
    .fptrCSDCalibrateWidget = &Cy_CapSense_CSDCalibrateWidget,
    .fptrCalibrateAllCsdWidgets = &Cy_CapSense_CalibrateAllCsdWidgets,
#endif

#if (CY_CAPSENSE_CSX_CALIBRATION_EN == 0)
    .fptrCalibrateAllCsxWidgets = NULL,
#else
    .fptrCalibrateAllCsxWidgets = &Cy_CapSense_CalibrateAllCsxWidgets,
#endif

#if (CY_CAPSENSE_SMARTSENSE_FULL_EN == 0)
    .fptrRunNoiseEnvelopeLib = NULL,
    .fptrDpUpdateThresholds = NULL,
    .fptrInitializeNoiseEnvelopeLib = NULL,
#else
    .fptrRunNoiseEnvelopeLib = &Cy_CapSense_RunNoiseEnvelope_Lib,
    .fptrDpUpdateThresholds = &Cy_CapSense_DpUpdateThresholds,
    .fptrInitializeNoiseEnvelopeLib = &Cy_CapSense_InitializeNoiseEnvelope_Lib,
#endif

#if ((CY_CAPSENSE_SMARTSENSE_HW_EN == 0) && (CY_CAPSENSE_SMARTSENSE_FULL_EN == 0))
    .fptrSsAutoTune = NULL,
#else
    .fptrSsAutoTune = &Cy_CapSense_SsAutoTune,
#endif

#if (CY_CAPSENSE_BIST_SUPPORTED)
#if (CY_CAPSENSE_BIST_EN == 0)
    .fptrBistInitialize = NULL,
    .fptrBistDisableMode = NULL,
    .fptrBistDsInitialize = NULL,
#else
    .fptrBistInitialize = &Cy_CapSense_BistInitialize,
    .fptrBistDisableMode = &Cy_CapSense_BistDisableMode,
    .fptrBistDsInitialize = &Cy_CapSense_BistDsInitialize,
#endif
#endif
};

static const cy_stc_capsense_pin_config_t cy_capsense_pinConfig[CY_CAPSENSE_PIN_COUNT] =
{
    { /* Button0_Sns0 */
        Button0_Sns0_PORT,
        Button0_Sns0_PIN,
    },
    { /* Button1_Sns0 */
        Button1_Sns0_PORT,
        Button1_Sns0_PIN,
    },
    { /* LinearSlider0_Sns0 */
        LinearSlider0_Sns0_PORT,
        LinearSlider0_Sns0_PIN,
    },
    { /* LinearSlider0_Sns1 */
        LinearSlider0_Sns1_PORT,
        LinearSlider0_Sns1_PIN,
    },
    { /* LinearSlider0_Sns2 */
        LinearSlider0_Sns2_PORT,
        LinearSlider0_Sns2_PIN,
    },
    { /* LinearSlider0_Sns3 */
        LinearSlider0_Sns3_PORT,
        LinearSlider0_Sns3_PIN,
    },
    { /* LinearSlider0_Sns4 */
        LinearSlider0_Sns4_PORT,
        LinearSlider0_Sns4_PIN,
    },
};

#if (CY_CAPSENSE_SHIELD_PIN_COUNT > 0)
    static const cy_stc_capsense_pin_config_t cy_capsense_shieldPinConfig[CY_CAPSENSE_SHIELD_PIN_COUNT] =
    {
    };
#endif

static const cy_stc_capsense_electrode_config_t cy_capsense_electrodeConfig[CY_CAPSENSE_ELTD_COUNT] =
{
    { /* Button0_Sns0 */
        .ptrPin = &cy_capsense_pinConfig[0u],
        .type = CY_CAPSENSE_ELTD_TYPE_SELF_E,
        .numPins = 1u,
    },
    { /* Button1_Sns0 */
        .ptrPin = &cy_capsense_pinConfig[1u],
        .type = CY_CAPSENSE_ELTD_TYPE_SELF_E,
        .numPins = 1u,
    },
    { /* LinearSlider0_Sns0 */
        .ptrPin = &cy_capsense_pinConfig[2u],
        .type = CY_CAPSENSE_ELTD_TYPE_SELF_E,
        .numPins = 1u,
    },
    { /* LinearSlider0_Sns1 */
        .ptrPin = &cy_capsense_pinConfig[3u],
        .type = CY_CAPSENSE_ELTD_TYPE_SELF_E,
        .numPins = 1u,
    },
    { /* LinearSlider0_Sns2 */
        .ptrPin = &cy_capsense_pinConfig[4u],
        .type = CY_CAPSENSE_ELTD_TYPE_SELF_E,
        .numPins = 1u,
    },
    { /* LinearSlider0_Sns3 */
        .ptrPin = &cy_capsense_pinConfig[5u],
        .type = CY_CAPSENSE_ELTD_TYPE_SELF_E,
        .numPins = 1u,
    },
    { /* LinearSlider0_Sns4 */
        .ptrPin = &cy_capsense_pinConfig[6u],
        .type = CY_CAPSENSE_ELTD_TYPE_SELF_E,
        .numPins = 1u,
    },
};

static const cy_stc_capsense_widget_config_t cy_capsense_widgetConfig[CY_CAPSENSE_WIDGET_COUNT] =
{
    { /* Button0 */
        .ptrWdContext = &cy_capsense_tuner.widgetContext[0u],
        .ptrSnsContext = &cy_capsense_tuner.sensorContext[0u],
        .ptrEltdConfig = &cy_capsense_electrodeConfig[0u],
#if (CY_CAPSENSE_BIST_SUPPORTED)
        .ptrEltdCapacitance = NULL,
        .ptrBslnInv = NULL,
#endif
        .ptrNoiseEnvelope = &cy_capsense_noiseEnvelope[0u],
        .ptrRawFilterHistory = NULL,
        .ptrRawFilterHistoryLow = NULL,
        .iirCoeff = 128u,
        .ptrDebounceArr = &cy_capsense_debounce[0u],
        .ptrDiplexTable = NULL,
        .centroidConfig = 0u,
        .xResolution = 0u,
        .yResolution = 0u,
        .numSns = 1u,
        .numCols = 1u,
        .numRows = 0u,
        .ptrPosFilterHistory = NULL,
        .ptrCsxTouchHistory = NULL,
        .ptrCsxTouchBuffer = NULL,
        .ptrCsdTouchBuffer = NULL,
        .ptrGestureConfig = NULL,
        .ptrGestureContext = NULL,
        .ballisticConfig = {
            .accelCoeff = 9u,
            .speedCoeff = 2u,
            .divisorValue = 4u,
            .speedThresholdX = 3u,
            .speedThresholdY = 4u,
        },
        .ptrBallisticContext = NULL,
        .aiirConfig = {
            .maxK = 60u,
            .minK = 1u,
            .noMovTh = 3u,
            .littleMovTh = 7u,
            .largeMovTh = 12u,
            .divVal = 64u,
        },
        .advConfig = {
            .penultimateTh = 100u,
            .virtualSnsTh = 100u,
            .crossCouplingTh = 5u,
        },
        .posFilterConfig = 0u,
        .rawFilterConfig = 0u,
        .senseMethod = CY_CAPSENSE_SENSE_METHOD_CSD_E,
        .wdType = CY_CAPSENSE_WD_BUTTON_E,
    },
    { /* Button1 */
        .ptrWdContext = &cy_capsense_tuner.widgetContext[1u],
        .ptrSnsContext = &cy_capsense_tuner.sensorContext[1u],
        .ptrEltdConfig = &cy_capsense_electrodeConfig[1u],
#if (CY_CAPSENSE_BIST_SUPPORTED)
        .ptrEltdCapacitance = NULL,
        .ptrBslnInv = NULL,
#endif
        .ptrNoiseEnvelope = &cy_capsense_noiseEnvelope[1u],
        .ptrRawFilterHistory = NULL,
        .ptrRawFilterHistoryLow = NULL,
        .iirCoeff = 128u,
        .ptrDebounceArr = &cy_capsense_debounce[1u],
        .ptrDiplexTable = NULL,
        .centroidConfig = 0u,
        .xResolution = 0u,
        .yResolution = 0u,
        .numSns = 1u,
        .numCols = 1u,
        .numRows = 0u,
        .ptrPosFilterHistory = NULL,
        .ptrCsxTouchHistory = NULL,
        .ptrCsxTouchBuffer = NULL,
        .ptrCsdTouchBuffer = NULL,
        .ptrGestureConfig = NULL,
        .ptrGestureContext = NULL,
        .ballisticConfig = {
            .accelCoeff = 9u,
            .speedCoeff = 2u,
            .divisorValue = 4u,
            .speedThresholdX = 3u,
            .speedThresholdY = 4u,
        },
        .ptrBallisticContext = NULL,
        .aiirConfig = {
            .maxK = 60u,
            .minK = 1u,
            .noMovTh = 3u,
            .littleMovTh = 7u,
            .largeMovTh = 12u,
            .divVal = 64u,
        },
        .advConfig = {
            .penultimateTh = 100u,
            .virtualSnsTh = 100u,
            .crossCouplingTh = 5u,
        },
        .posFilterConfig = 0u,
        .rawFilterConfig = 0u,
        .senseMethod = CY_CAPSENSE_SENSE_METHOD_CSD_E,
        .wdType = CY_CAPSENSE_WD_BUTTON_E,
    },
    { /* LinearSlider0 */
        .ptrWdContext = &cy_capsense_tuner.widgetContext[2u],
        .ptrSnsContext = &cy_capsense_tuner.sensorContext[2u],
        .ptrEltdConfig = &cy_capsense_electrodeConfig[2u],
#if (CY_CAPSENSE_BIST_SUPPORTED)
        .ptrEltdCapacitance = NULL,
        .ptrBslnInv = NULL,
#endif
        .ptrNoiseEnvelope = &cy_capsense_noiseEnvelope[2u],
        .ptrRawFilterHistory = NULL,
        .ptrRawFilterHistoryLow = NULL,
        .iirCoeff = 128u,
        .ptrDebounceArr = &cy_capsense_debounce[2u],
        .ptrDiplexTable = NULL,
        .centroidConfig = 1u,
        .xResolution = 100u,
        .yResolution = 0u,
        .numSns = 5u,
        .numCols = 5u,
        .numRows = 0u,
        .ptrPosFilterHistory = NULL,
        .ptrCsxTouchHistory = NULL,
        .ptrCsxTouchBuffer = NULL,
        .ptrCsdTouchBuffer = NULL,
        .ptrGestureConfig = &cy_capsense_tuner.gestureConfig[0u],
        .ptrGestureContext = &cy_capsense_gestureContext[0u],
        .ballisticConfig = {
            .accelCoeff = 9u,
            .speedCoeff = 2u,
            .divisorValue = 4u,
            .speedThresholdX = 3u,
            .speedThresholdY = 4u,
        },
        .ptrBallisticContext = NULL,
        .aiirConfig = {
            .maxK = 60u,
            .minK = 1u,
            .noMovTh = 3u,
            .littleMovTh = 7u,
            .largeMovTh = 12u,
            .divVal = 64u,
        },
        .advConfig = {
            .penultimateTh = 100u,
            .virtualSnsTh = 100u,
            .crossCouplingTh = 5u,
        },
        .posFilterConfig = 0u,
        .rawFilterConfig = 0u,
        .senseMethod = CY_CAPSENSE_SENSE_METHOD_CSD_E,
        .wdType = CY_CAPSENSE_WD_LINEAR_SLIDER_E,
    },
};

cy_stc_capsense_tuner_t cy_capsense_tuner =
{
    .commonContext = {
        .configId = 0xa8b9,
        .tunerCmd = 0u,
        .scanCounter = 0u,
        .tunerSt = 0u,
        .initDone = 0u,
        .ptrSSCallback = NULL,
        .ptrEOSCallback = NULL,
        .ptrTunerSendCallback = NULL,
        .ptrTunerReceiveCallback = NULL,
        .status = 0u,
        .timestampInterval = 1u,
        .timestamp = 0u,
        .modCsdClk = 2u,
        .modCsxClk = 2u,
        .tunerCnt = 0u,
    },
    .widgetContext = {
        {
            .fingerCap = 160u,
            .sigPFC = 0u,
            .resolution = 12u,
            .maxRawCount = 0u,
            .fingerTh = 100u,
            .proxTh = 200u,
            .lowBslnRst = 30u,
            .snsClk = 16u,
            .rowSnsClk = 16u,
            .gestureDetected = 0u,
            .gestureDirection = 0u,
            .xDelta = 0u,
            .yDelta = 0u,
            .noiseTh = 40u,
            .nNoiseTh = 40u,
            .hysteresis = 10u,
            .onDebounce = 3u,
            .snsClkSource = CY_CAPSENSE_CLK_SOURCE_AUTO_MASK,
            .idacMod = { 32u, 32u, 32u, },
            .idacGainIndex = 4u,
            .rowIdacMod = { 32u, 32u, 32u, },
            .bslnCoeff = 1u,
            .status = 0u,
            .wdTouch = {
                .ptrPosition = NULL,
                .numPosition = 0,
            },
        },
        {
            .fingerCap = 160u,
            .sigPFC = 0u,
            .resolution = 12u,
            .maxRawCount = 0u,
            .fingerTh = 100u,
            .proxTh = 200u,
            .lowBslnRst = 30u,
            .snsClk = 16u,
            .rowSnsClk = 16u,
            .gestureDetected = 0u,
            .gestureDirection = 0u,
            .xDelta = 0u,
            .yDelta = 0u,
            .noiseTh = 40u,
            .nNoiseTh = 40u,
            .hysteresis = 10u,
            .onDebounce = 3u,
            .snsClkSource = CY_CAPSENSE_CLK_SOURCE_AUTO_MASK,
            .idacMod = { 32u, 32u, 32u, },
            .idacGainIndex = 4u,
            .rowIdacMod = { 32u, 32u, 32u, },
            .bslnCoeff = 1u,
            .status = 0u,
            .wdTouch = {
                .ptrPosition = NULL,
                .numPosition = 0,
            },
        },
        {
            .fingerCap = 160u,
            .sigPFC = 0u,
            .resolution = 12u,
            .maxRawCount = 0u,
            .fingerTh = 100u,
            .proxTh = 200u,
            .lowBslnRst = 30u,
            .snsClk = 16u,
            .rowSnsClk = 16u,
            .gestureDetected = 0u,
            .gestureDirection = 0u,
            .xDelta = 0u,
            .yDelta = 0u,
            .noiseTh = 40u,
            .nNoiseTh = 40u,
            .hysteresis = 10u,
            .onDebounce = 3u,
            .snsClkSource = CY_CAPSENSE_CLK_SOURCE_AUTO_MASK,
            .idacMod = { 32u, 32u, 32u, },
            .idacGainIndex = 4u,
            .rowIdacMod = { 32u, 32u, 32u, },
            .bslnCoeff = 1u,
            .status = 0u,
            .wdTouch = {
                .ptrPosition = &cy_capsense_tuner.position[0u],
                .numPosition = 1,
            },
        },
    },
    .sensorContext = {
        {
            .raw = 0u,
            .bsln = 0u,
            .diff = 0u,
            .status = 0u,
            .negBslnRstCnt = 0u,
            .idacComp = 32u,
            .bslnExt = 0u,
        },
        {
            .raw = 0u,
            .bsln = 0u,
            .diff = 0u,
            .status = 0u,
            .negBslnRstCnt = 0u,
            .idacComp = 32u,
            .bslnExt = 0u,
        },
        {
            .raw = 0u,
            .bsln = 0u,
            .diff = 0u,
            .status = 0u,
            .negBslnRstCnt = 0u,
            .idacComp = 32u,
            .bslnExt = 0u,
        },
        {
            .raw = 0u,
            .bsln = 0u,
            .diff = 0u,
            .status = 0u,
            .negBslnRstCnt = 0u,
            .idacComp = 32u,
            .bslnExt = 0u,
        },
        {
            .raw = 0u,
            .bsln = 0u,
            .diff = 0u,
            .status = 0u,
            .negBslnRstCnt = 0u,
            .idacComp = 32u,
            .bslnExt = 0u,
        },
        {
            .raw = 0u,
            .bsln = 0u,
            .diff = 0u,
            .status = 0u,
            .negBslnRstCnt = 0u,
            .idacComp = 32u,
            .bslnExt = 0u,
        },
        {
            .raw = 0u,
            .bsln = 0u,
            .diff = 0u,
            .status = 0u,
            .negBslnRstCnt = 0u,
            .idacComp = 32u,
            .bslnExt = 0u,
        },
    },
    .position = {
        {
            .x = 0u,
            .y = 0u,
            .z = 0u,
            .id = 0u,
        },
    },
    .gestureConfig = {
        {
            .resolutionX = 100u,
            .resolutionY = 300u,
            .gestureEnableMask = 0x81,
            .flickTimeoutMax = 300u,
            .edgeTimeoutMax = 2000u,
            .clickTimeoutMax = 1000u,
            .clickTimeoutMin = 0u,
            .secondClickIntervalMax = 1000u,
            .secondClickIntervalMin = 0u,
            .zoomDistanceMin = 50u,
            .flickDistanceMin = 100u,
            .scrollDistanceMin = 20u,
            .rotateDistanceMin = 50u,
            .edgeDistanceMin = 200u,
            .secondClickDistanceMax = 100u,
            .clickDistanceMax = 100u,
            .zoomDebounce = 3u,
            .scrollDebounce = 3u,
            .rotateDebounce = 10u,
            .edgeAngleMax = 45u,
            .edgeEdgeSize = 200u,
        },
    },
};

#if (CY_CAPSENSE_BIST_SUPPORTED)
#if (CY_CAPSENSE_BIST_EN != 0)
#define CY_CAPSENSE_MULTI_FREQUENCY_COUNT                      (CY_CAPSENSE_MULTI_FREQUENCY_SCAN_EN == 0 ? 1 : 3)
uint16_t bslnInv[CY_CAPSENSE_SENSOR_COUNT * CY_CAPSENSE_MULTI_FREQUENCY_COUNT];
uint32_t eltdCap[CY_CAPSENSE_ELTD_COUNT];
uint16_t wdgtCrc[CY_CAPSENSE_WIDGET_COUNT];

cy_stc_capsense_bist_context_t cy_capsense_bist_context =
{
};
#endif
#endif

cy_stc_capsense_context_t cy_capsense_context =
{
    .ptrCommonConfig = &cy_capsense_commonConfig,
    .ptrCommonContext = &cy_capsense_tuner.commonContext,
    .ptrInternalContext = &cy_capsense_internalContext,
    .ptrWdConfig = &cy_capsense_widgetConfig[0u],
    .ptrWdContext = &cy_capsense_tuner.widgetContext[0u],
    .ptrPinConfig = &cy_capsense_pinConfig[0u],
    .ptrShieldPinConfig = NULL,
    .ptrActiveScanSns = &cy_capsense_activeScanSns,
    .ptrFptrConfig = (const void *) &cy_capsense_fptrConfig,
#if (CY_CAPSENSE_BIST_SUPPORTED)
    .ptrBistContext = NULL,
#endif
};

#endif /* CY_CAPSENSE_CORE == __CORTEX_M */

#endif /* !defined(CY_DISABLE_CAPSENSE) */

/* [] END OF FILE */

