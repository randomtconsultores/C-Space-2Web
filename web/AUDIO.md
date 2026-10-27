# Web audio

The Web build generates the 16 replaceable mono WAV assets from the existing
event table in `web/make_sounds.py`. `make -f web/Makefile` writes the files and
manifest to `web/build/sounds`; the browser maps C event IDs directly to those
WAVs through Howler.js. The browser does not parse SoX commands or synthesize
audio. Howler.js is loaded from its CDN script in `web/index.html`.

The JavaScript consumer drains the bounded C FIFO and limits playback to eight
simultaneous sounds. Turning sound off, blurring the page, or hiding the tab
stops active playback and clears pending events.

SOUND starts on. The generated original chiptune loops only while the menu is
active. Howler preloads the 16 effects and menu track so playback never waits on
an unloaded sound. To test real playback, open the forwarded port 8000 in a browser with
network access to the Howler.js CDN and tap SOUND OFF (shown as SOUND ON after
activation), or TEST SOUND (13) to play an existing damage WAV directly. The Q
touch button also triggers the existing damage event in a game. The Audio status
line reports loaded WAVs, C events, play calls, errors, volume and AudioContext
state. Browser autoplay policy requires a user gesture.

The WAVs are deterministic placeholders based on the existing event durations
and frequency descriptions, not faithful SoX exports or new game sounds. Native
audio behavior, `MAX_NIVEIS=4`, and persistence are unchanged.
