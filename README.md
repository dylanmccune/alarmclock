# Wakey Wakey (an alarm clock) 
I made this project because I currently use my Alexa as an alarm clock in the mornings, but the problem with that is I can just tell it to stop without getting out of bed and fall right asleep. This aims to fix that problem by forcing me to get up and turn on the lights to make it stop.

## Hardware
The physical hardware is based off a Seeed Xiao ESP32C3 module and a regular photoresistor. Mine has it soldered to pin A0 but you can change that in the config.

## Firmware
The alarm clock works by sending out a Voicemonkey webhook in order to start the alarm playing. The "alarm" sound is just a looping song on Spotify, so you can set it to either a traditional beep/ringy alarm or any song you want! Voicemonkey is a service that lets you trigger Alexa routines remotely through a webhook. It has a free plan, which I'm using. After the initial alarm start, it talks to the Spotify API to handle the stopping and restarting of the alarm. The reason I can't do that at the start is because the Spotify API needs devices to be "awake" and have an active Spotify Connect connection, and it stops the connection after 20-30 mins of being idle. Voicemonkey can get around this by directly talking to Amazon.

## Setup
You need these:
- Voicemonkey account (voicemonkey.io)
- An Amazon Echo that's connected to your Spotify account
- Spotify premium (required to use the API)
- The hardware (detailed above)

Open the Voicemonkey dashboard (app.voicemonkey.io), and click "manage routines". Then, create a new routine, set the name to "alarm clock start", and click create trigger. After that, head to Webhooks on the sidebar, click the green new webhook button, and set the action type to routine trigger and the device to the trigger you just made. After that, it'll give you a webhook url. Save this for later.

After that, open up the Alexa app and create a new routine with the voicemonkey trigger and set the action to play your chosen song on Spotify. I set it to use a playlist so I can change the song directly in the Spotify app, and add multiple songs to pick between.
!(Screenshot of the Alexa app)[./media/6DA885D5-5814-48B5-BA76-D32182BE152B.jpeg]

Finally, open the Spotify developer dashboard (https://developer.spotify.com/dashboard) and create a new app. Name it whatever you want, and set the redirect URI to https://spesp32.finianlandes.workers.dev/api/spotify/callback.

Now that you've done all the setup, you can get the code onto the board. Clone this repo (`git clone https://github.com/dylanmccune/alarmclock.git`) and open it in VSCode. Install the PlatformIO extension if it isn't already. Open `src/secrets.h.example`, copy and rename it to `secrets.h`, and put in your Spotify client ID/secret and the Voicemonkey endpoint from earlier.
Change the time in `src/config.h`, and then upload the code to the board!
Once the code is running, open your computer/phone and you should see a new WiFi network to connect to. Connect to it, then it'll open up a popup asking for your main WiFi SSID/password. Put it in, then reupload the code and look in the serial console for a spotify auth url. Open that on your computer and log in to Spotify. Once that works, you should be good to go!

Made for Hack Club Stardance