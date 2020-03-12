<?xml version="1.0" encoding="ASCII"?>
<ResourceModel:App xmi:version="2.0" xmlns:xmi="http://www.omg.org/XMI" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ResourceModel="http://www.infineon.com/Davex/Resource.ecore" name="PWM" URI="http://resources/4.1.10/app/PWM/2" description="Generates a PWM using one timer slice of CCU4 or CCU8" mode="NOTSHARABLE" version="4.1.10" minDaveVersion="4.3.2" instanceLabel="PWM_b" appLabel="" containingProxySignal="true">
  <properties provideInit="true"/>
  <virtualSignals name="global_signal" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_ccu8_cc8_slice_global" hwSignal="global_signal" hwResource="//@hwResources.0"/>
  <virtualSignals name="event_compare_match" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_ccu8_cc8_slice_compare_match_interrupt" hwSignal="cmd1s_cmu1s" hwResource="//@hwResources.0"/>
  <virtualSignals name="event_period_match" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_ccu8_cc8_slice_period_match_interrupt" hwSignal="pmus_omds" hwResource="//@hwResources.0"/>
  <virtualSignals name="timer_status" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_ccu8_cc8_slice_st" hwSignal="st" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="pwm_output" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_ccu8_cc8_slice_out" hwSignal="out0" hwResource="//@hwResources.0" visible="true"/>
  <virtualSignals name="global_signal" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_ccu4_cc4_slice_global" hwSignal="global_signal" hwResource="//@hwResources.1" required="false"/>
  <virtualSignals name="event_compare_match" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_ccu4_cc4_slice_compare_match_interrupt" hwSignal="cmds_cmus" hwResource="//@hwResources.1" required="false"/>
  <virtualSignals name="event_period_match" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_ccu4_cc4_slice_period_match_interrupt" hwSignal="pmus_omds" hwResource="//@hwResources.1" required="false"/>
  <virtualSignals name="timer_status" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_ccu4_cc4_slice_st" hwSignal="st" hwResource="//@hwResources.1" required="false" visible="true"/>
  <virtualSignals name="pwm_output" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_ccu4_cc4_slice_out" hwSignal="out" hwResource="//@hwResources.1" required="false" visible="true"/>
  <virtualSignals name="pwm_pad" URI="http://resources/4.1.10/app/PWM/2/vs_pwm_out_pin" hwSignal="pad" hwResource="//@hwResources.2"/>
  <virtualSignals name="PWM Output Pin_signal" URI="http://resources/4.1.10/app/PWM/2/__pin_vs_pwm_out_pin" hwSignal="pin" hwResource="//@hwResources.3"/>
  <requiredApps URI="http://resources/4.1.10/app/PWM/2/pwm_app_global_ccu8_cc8" requiredAppName="GLOBAL_CCU8" requiringMode="SHARABLE">
    <downwardMapList xsi:type="ResourceModel:App" href="../../GLOBAL_CCU8/v4_1_12/GLOBAL_CCU8_0.app#/"/>
  </requiredApps>
  <requiredApps URI="http://resources/4.1.10/app/PWM/2/pwm_app_global_ccu4_cc4" requiredAppName="GLOBAL_CCU4" required="false" requiringMode="SHARABLE"/>
  <hwResources name="CC8 Slice" URI="http://resources/4.1.10/app/PWM/2/hwres_ccu8_cc8_slice" resourceGroupUri="resources/0.1.31/peripheral/ccu8/0/cc8/1" mResGrpUri="peripheral/ccu8/*/cc8/*">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/CCU80/CCU80_0.dd#//@provided.43"/>
  </hwResources>
  <hwResources name="CC4 Slice" URI="http://resources/4.1.10/app/PWM/2/hwres_ccu4_cc4_slice" resourceGroupUri="" required="false" mResGrpUri="peripheral/ccu4/*/cc4/*"/>
  <hwResources name="PWM Output Pin" URI="http://resources/4.1.10/app/PWM/2/hwres_port_pad_pwm_out" resourceGroupUri="port/p/*/pad/*" mResGrpUri="port/p/*/pad/*">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/Port0/Port0_0.dd#//@provided.15"/>
  </hwResources>
  <hwResources name="PWM Output Pin" URI="http://resources/4.1.10/app/PWM/2/__pin_hwres_port_pad_pwm_out" resourceGroupUri="devicepackage/0/96" constraintType="GLOBAL_RESOURCE" mResGrpUri="devicepackage/0/*">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/DEVICEPACKAGE/DEVICEPACKAGE_0.dd#//@provided.4"/>
  </hwResources>
  <connections URI="http://resources/4.1.10/app/PWM/2/http://resources/4.1.10/app/PWM/2/vs_pwm_ccu8_cc8_global/http://resources/4.1.10/app/PWM/2/vs_pwm_ccu8_cc8_slice_global" systemDefined="true" sourceSignal="ccu8_global" targetSignal="global_signal" targetVirtualSignal="//@virtualSignals.0" proxySrcVirtualSignalUri="http://resources/4.1.12/app/GLOBAL_CCU8/0/vs_global_ccu8_globalsignal" containingProxySignal="true">
    <downwardMapList xsi:type="ResourceModel:VirtualSignal" href="../../GLOBAL_CCU8/v4_1_12/GLOBAL_CCU8_0.app#//@virtualSignals.0"/>
    <srcVirtualSignal href="../../GLOBAL_CCU8/v4_1_12/GLOBAL_CCU8_0.app#//@virtualSignals.0"/>
  </connections>
  <connections URI="http://resources/4.1.10/app/PWM/2/http://resources/4.1.10/app/PWM/2/vs_pwm_ccu4_cc4_global/http://resources/4.1.10/app/PWM/2/vs_pwm_ccu4_cc4_slice_global" systemDefined="true" sourceSignal="ccu4_global" targetSignal="global_signal" required="false" targetVirtualSignal="//@virtualSignals.5" proxySrcVirtualSignalUri="http://resources/4.1.12/app/GLOBAL_CCU4/0/vs_global_ccu4_globalsignal" containingProxySignal="true"/>
  <connections URI="http://resources/4.1.10/app/PWM/2/http://resources/4.1.10/app/PWM/2/vs_pwm_ccu4_cc4_slice_out/http://resources/4.1.10/app/PWM/2/vs_pwm_out_pin" systemDefined="true" sourceSignal="pwm_output" targetSignal="pwm_pad" required="false" srcVirtualSignal="//@virtualSignals.9" targetVirtualSignal="//@virtualSignals.10"/>
  <connections URI="http://resources/4.1.10/app/PWM/2/http://resources/4.1.10/app/PWM/2/vs_pwm_ccu8_cc8_slice_out/http://resources/4.1.10/app/PWM/2/vs_pwm_out_pin" systemDefined="true" sourceSignal="pwm_output" targetSignal="pwm_pad" srcVirtualSignal="//@virtualSignals.4" targetVirtualSignal="//@virtualSignals.10"/>
  <connections URI="http://resources/4.1.10/app/PWM/2/http://resources/4.1.10/app/PWM/2/vs_pwm_out_pin/http://resources/4.1.10/app/PWM/2/__pin_vs_pwm_out_pin" systemDefined="true" sourceSignal="pwm_pad" targetSignal="PWM Output Pin_signal" srcVirtualSignal="//@virtualSignals.10" targetVirtualSignal="//@virtualSignals.11"/>
  <connections URI="http://resources/4.1.10/app/PWM/2/http://resources/4.1.10/app/PWM/2/__pin_vs_pwm_out_pin/http://resources/4.1.10/app/PWM/2/vs_pwm_out_pin" systemDefined="true" sourceSignal="PWM Output Pin_signal" targetSignal="pwm_pad" srcVirtualSignal="//@virtualSignals.11" targetVirtualSignal="//@virtualSignals.10"/>
</ResourceModel:App>
