A simple standalone dmx controller on ESP32 basis

About:
My DMX Controller broke and I have a few ESP32s laying around so I decided to create a very simple DMX Controller running on the ESP32. While there are many projects using the ESP32 as an ArtnetNode, I wanted a controller that runs standalone so I can use my lights without running a whole network and computer with software.

Hardware:
ESP Terminal 3.5-inch Display

Libraries:
-LVGL
-esp_dmx by someweisguy

More information might follow when this is working...

roadmap:
*get input working to output static dmx values
*first ui overhall to prepare for animations
*animation engine and animated dmx values
*second ui overhall and scenes

possible future features:
fixture definition and usage (maybe using an open fixture format)
artnet node mode
webui
compability with other Esp32s with or without screens
player for recorded dmx data
