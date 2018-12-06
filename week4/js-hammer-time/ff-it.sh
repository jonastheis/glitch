
WEB_PORT=5000
DEBUG_PORT=13370

adb reboot
sleep 20

adb reverse tcp:$WEB_PORT tcp:$WEB_PORT
adb forward tcp:$DEBUG_PORT localabstract:org.mozilla.firefox/firefox-debugger-socket
# adb exec-out su -c /data/local/tmp/frida-server12 &
adb exec-out su -c "/data/local/tmp/frida-server12 &"

adb shell input keyevent KEYCODE_POWER #turn on screen
adb shell input touchscreen swipe 930 880 930 380 #swipe up to unlock
sleep 1

adb shell am start -n org.mozilla.firefox/org.mozilla.gecko.BrowserApp
sleep 1.5
adb shell input tap 895 189 # multi-tab page
sleep 1

adb shell input tap 496 331 # close the tab
sleep 1.5

adb shell input tap 453 183 # input text
sleep 0.5

adb shell input text "localhost:"$WEB_PORT 
adb shell input keyevent 66 #enter


# to kill firefox
# adb shell am force-stop org.mozilla.firefox
