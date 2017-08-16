var group__usb__device__controller__driver =
[
    [ "_usb_device_callback_message_struct", "struct__usb__device__callback__message__struct.html", [
      [ "buffer", "struct__usb__device__callback__message__struct.html#adff2b0d4cadc686699c367705b7127f6", null ],
      [ "code", "struct__usb__device__callback__message__struct.html#ada4df9e2414e192bfd8c4d27fe1596ee", null ],
      [ "isSetup", "struct__usb__device__callback__message__struct.html#a241d466ac120fe60cad47ca2c84e5a29", null ],
      [ "length", "struct__usb__device__callback__message__struct.html#aae817b0dc7d1f8394024910412caba31", null ]
    ] ],
    [ "_usb_device_controller_interface_struct", "struct__usb__device__controller__interface__struct.html", [
      [ "deviceCancel", "struct__usb__device__controller__interface__struct.html#aeba4e88996e097bd9b88b6219c60c5c2", null ],
      [ "deviceControl", "struct__usb__device__controller__interface__struct.html#ad4f40ea9e2cfa7df3a40f601f202f36a", null ],
      [ "deviceDeinit", "struct__usb__device__controller__interface__struct.html#a8910543e7559d2e3046f718fb5dffa14", null ],
      [ "deviceInit", "struct__usb__device__controller__interface__struct.html#a2a8addd32dd6a3cb66f01f6912585ae1", null ],
      [ "deviceRecv", "struct__usb__device__controller__interface__struct.html#ac58934254b0e4122309de852bc1399c4", null ],
      [ "deviceSend", "struct__usb__device__controller__interface__struct.html#a316b3b4a883b7e9f4007e30f28a463ff", null ]
    ] ],
    [ "_usb_device_struct", "struct__usb__device__struct.html", [
      [ "controllerHandle", "struct__usb__device__struct.html#a0590c24c845667193936f0a446d08cb9", null ],
      [ "controllerId", "struct__usb__device__struct.html#ad402e79419d37fc72e7f71fb10ec3289", null ],
      [ "controllerInterface", "struct__usb__device__struct.html#aee4a13057112d36193a654fef978f4ae", null ],
      [ "deviceAddress", "struct__usb__device__struct.html#abae4041d3d8b46a4d06954cd13061424", null ],
      [ "deviceCallback", "struct__usb__device__struct.html#a9b9546705d3f2bbccf1a05605cdbf869", null ],
      [ "endpointCallback", "struct__usb__device__struct.html#a29eda36612811852d13e534b86ac927d", null ],
      [ "isResetting", "struct__usb__device__struct.html#a1df9a5fa5845e9b0adca6733e87a82d6", null ],
      [ "state", "struct__usb__device__struct.html#a9fa0b5304d25ba7cec0517ae05cad566", null ]
    ] ],
    [ "usb_device_controller_handle", "group__usb__device__controller__driver.html#gaec03ce2e4732aa876cd2a91cf8e93b5b", null ],
    [ "usb_device_callback_message_struct_t", "group__usb__device__controller__driver.html#ga85f9edb535af10a4a706061b9f06bc25", null ],
    [ "usb_device_control_type_t", "group__usb__device__controller__driver.html#gae16dc18943ae9da34a2b122859568b5e", null ],
    [ "usb_device_controller_cancel_t", "group__usb__device__controller__driver.html#ga476c2b07d6a5e467e104568aad59ef44", null ],
    [ "usb_device_controller_control_t", "group__usb__device__controller__driver.html#gae484b958b5a84af6ce5921f8538ff1c4", null ],
    [ "usb_device_controller_deinit_t", "group__usb__device__controller__driver.html#ga283139f5df38421250c30a094f34088d", null ],
    [ "usb_device_controller_init_t", "group__usb__device__controller__driver.html#ga7f8af48e2f4f886f521f8128b6a05e9b", null ],
    [ "usb_device_controller_interface_struct_t", "group__usb__device__controller__driver.html#ga634411452886b58586be873d8d631a3b", null ],
    [ "usb_device_controller_recv_t", "group__usb__device__controller__driver.html#gac14df7129a0dc5e7c0b7148b05992dc8", null ],
    [ "usb_device_controller_send_t", "group__usb__device__controller__driver.html#ga5879c987e628c5c5a27b5fcc685e90c8", null ],
    [ "usb_device_notification_t", "group__usb__device__controller__driver.html#gabb134b8f1a6b63ae3979523ba53480bc", null ],
    [ "usb_device_struct_t", "group__usb__device__controller__driver.html#ga4be463ccd1fe520b377febafb0a06c2f", null ],
    [ "_usb_device_control_type", "group__usb__device__controller__driver.html#gaa7a58da69289c9e774119a7998835c54", [
      [ "kUSB_DeviceControlRun", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a0d4334c836c8ddba19dd1b3c343bf718", null ],
      [ "kUSB_DeviceControlStop", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a3e65ecf4751583140d63e6ad7129bb9a", null ],
      [ "kUSB_DeviceControlEndpointInit", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a5f2f7690579a2582b74013d569f7a2b5", null ],
      [ "kUSB_DeviceControlEndpointDeinit", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a758593417eb515bd9a64b5c9d9faf6c3", null ],
      [ "kUSB_DeviceControlEndpointStall", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a667e55c04a0399ec26fd94a9aa6918e4", null ],
      [ "kUSB_DeviceControlEndpointUnstall", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54ae3d343db3b169349791d2c1b14b8140b", null ],
      [ "kUSB_DeviceControlGetDeviceStatus", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54aa04b915379ec7616d8e7318618652a1a", null ],
      [ "kUSB_DeviceControlGetEndpointStatus", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a904c707abbfeac703932a25b731f668e", null ],
      [ "kUSB_DeviceControlSetDeviceAddress", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a2e2b710fe6f299d1cd6655672b5d5c52", null ],
      [ "kUSB_DeviceControlGetSynchFrame", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54ab290661a0aa5f30d61a22f9117a58899", null ],
      [ "kUSB_DeviceControlResume", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a8bd921505bfadde8059c154c2025ae4e", null ],
      [ "kUSB_DeviceControlSleepResume", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a7bed5173624dd0ebbb35da467111bd67", null ],
      [ "kUSB_DeviceControlSuspend", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a8e66ee545151bb423cab8128033c5770", null ],
      [ "kUSB_DeviceControlSleep", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a43941c46b3a3352c16a61ae2679c9c0b", null ],
      [ "kUSB_DeviceControlSetDefaultStatus", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54aca7240a60464f7f19347ef3d518fd97f", null ],
      [ "kUSB_DeviceControlGetSpeed", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54aa5de8d34f68ecf4e2948033b27f67567", null ],
      [ "kUSB_DeviceControlGetOtgStatus", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a47e409659c5b7c11a8aa26ad4ec65279", null ],
      [ "kUSB_DeviceControlSetOtgStatus", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a9b5a35a9f0054382e96410fffb7f2e8b", null ],
      [ "kUSB_DeviceControlSetTestMode", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54ad3c6f680a4c3f112607d3a582b8b2cd8", null ],
      [ "kUSB_DeviceControlGetRemoteWakeUp", "group__usb__device__controller__driver.html#ggaa7a58da69289c9e774119a7998835c54a268bbf4bd031a441273508f75d24ddc4", null ]
    ] ],
    [ "_usb_device_notification", "group__usb__device__controller__driver.html#gafdba1a6b57efd437d6a941bfbb9239db", [
      [ "kUSB_DeviceNotifyBusReset", "group__usb__device__controller__driver.html#ggafdba1a6b57efd437d6a941bfbb9239dba6b3844cfeb2f1acd91ec86e553cb4938", null ],
      [ "kUSB_DeviceNotifySuspend", "group__usb__device__controller__driver.html#ggafdba1a6b57efd437d6a941bfbb9239dbaa9a1526acc96165a26fe187a771bd7ca", null ],
      [ "kUSB_DeviceNotifyResume", "group__usb__device__controller__driver.html#ggafdba1a6b57efd437d6a941bfbb9239dbaf82ebeb85b3be0b518d0608052acfb46", null ],
      [ "kUSB_DeviceNotifyLPMSleep", "group__usb__device__controller__driver.html#ggafdba1a6b57efd437d6a941bfbb9239dba28a2f5e3abf7a469f1f39f154b8e72b9", null ],
      [ "kUSB_DeviceNotifyLPMResume", "group__usb__device__controller__driver.html#ggafdba1a6b57efd437d6a941bfbb9239dba35a2f56b459ad548456cc002b468a632", null ],
      [ "kUSB_DeviceNotifyError", "group__usb__device__controller__driver.html#ggafdba1a6b57efd437d6a941bfbb9239dba703fdebcf80a09a04f9149cc9171576f", null ],
      [ "kUSB_DeviceNotifyDetach", "group__usb__device__controller__driver.html#ggafdba1a6b57efd437d6a941bfbb9239dba26b9d31e47d99f112ebc74b17e1879cd", null ],
      [ "kUSB_DeviceNotifyAttach", "group__usb__device__controller__driver.html#ggafdba1a6b57efd437d6a941bfbb9239dba9b4873734c36adcc45b205b5f55bad31", null ]
    ] ]
];