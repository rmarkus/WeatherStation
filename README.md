# Weather Station
esp32 based weather station, mqtt

## Some notes on power consumption.

during connect and mqtt send the esp takes around 100mA during 5 seconds.
In deep sleep the AZ delivery module takes 1.5 mA (maybe due to the always-on power led). 

If I wake up every 10 minutes and send data, it will consume 6\*5s\*100mA = 3000mAs within 1 hour.
Addionally the 1.5mA\*(3600s-6\*5s) = 5355 mAs. Together I get 8355mAs consumed, or 2.3mAh. in other words, within 24h I consume 55mAh. 

I will be using a small 3W solar panel. This gives around 20mA*8h = 160mAh under weakest conditions (200mA in full sunshine). This means that I still have 160mAh - 55mAh = 105mAh left each day to make sure that the battery is really charged. I tmeans I should have enough power even in winter to operate the esp32 24/7. 

Note my battery has 800mAh, so enough buffer to cover the "dark" times of the day.

### Real World

here are somoe measurements. The good news is, that approximately all consumed energy is renewed during daylight time. We had 3 very cloudy days. The peak at the end of the graph is a sunny morning with full sunlight on the solar panel. 

![canvas](https://user-images.githubusercontent.com/1353254/232988197-748cf854-1556-4988-b1d2-bd6a48a50a68.png)

What surprises me nevertheless: First day, I had 1 minute interval to connect to WLAN and post data, later on 10 minutes. From the graph, this does not seem to make any difference in power consumption. 

I have meausred again, and it reveals:
* the Waveshare charger consumes ~2mA all the time
* the AZ Delivery ESP32 board consumes 2mA in deepsleep.
* the actual sending might be faster than 5 seconds.
