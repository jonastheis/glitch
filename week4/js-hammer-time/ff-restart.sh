adb shell am force-stop org.mozilla.firefox

adb shell am start -n org.mozilla.firefox/org.mozilla.gecko.BrowserApp
sleep 1.5
adb shell input tap 895 189 # multi-tab page