# WeatherStation
esp32 based weather station, mqtt

## Some notes on power consumption.

during connect and mqtt send the esp takes around 100mA during 5 seconds.
In deep sleep the AZ delivery module takes 1.5 mA (maybe due to the always-on power led). 

If I wake up every 10 minutes and send data, it will consume 6\*5s\*100mA = 3000mAs within 1 hour.
Addionally the 1.5mA\*(3600s-6\*5s) = 5355 mAs. Together I get 8355mAs consumed, or 2.3mAh. in other words, within 24h I consume 55mAh. 

I will be using a small 3W solar panel. This gives around 20mA*8h = 160mAh under weakest conditions (200mA in full sunshine). This means that I still have 160mAh - 55mAh = 105mAh left each day to make sure that the battery is really charged. I tmeans I should have enough power even in winter to operate the esp32 24/7. 

Note my battery has 800mAh, so enough buffer to cover the "dark" times of the day.
