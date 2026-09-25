# Audio fixture

Build `audio` through the [example index](../README.md). The screen shows audio
state and controls. A requests 440/660/880 Hz for 500 ms each, Up stops playback,
and B toggles and saves the game audio preference. Long Back exits.

The dedicated save ID is `sdk_ab_audio_20260913`. Relaunch to check the persisted
preference. Device-wide mute still applies. Logs report accepted requests and
playback state; audible output needs physical observation. See [audio](../../docs/audio.md).
