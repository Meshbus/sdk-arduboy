# Audio adapter

The default runtime implements ArduboyTones one-, two- and three-note calls,
bounded frequency/duration sequences, `noTone()` and `playing()`. Each request
owns a host playback token. `playing()` queries the host worker, so completion,
explicit stop and preemption by a system notification are visible. It reports
active playback, not an acoustic measurement. Zero duration holds the final
note until stopped. Frequency zero is a rest.

The runtime copies up to 32 notes into one 128-byte buffer. Caller arrays may
change after submission. Before reusing the buffer or exiting, the runtime
stops its token and synchronizes with host note reads. No callback enters the
extension. Stopping an obsolete token does not stop the newer system sound.
Playback uses the existing host workqueue, with no added resident thread.

`tones(score, value_count)` checks the declared span and requires TONES_END.
The compatible pointer-only overload requires the caller to supply a valid
terminated score of at most 32 pairs; it cannot validate an arbitrary pointer.
Malformed, over-capacity and repeated tone-pair sequences are rejected in full.
`TONES_REPEAT` and `volumeMode` remain unsupported and expose -ENOTSUP.
Registered legacy opcode spans have a separate bounded decoder, including E0
repeat; see [legacy scores](legacy.md) for registration and mono-channel limits.
TONE_HIGH_VOLUME is accepted as a frequency flag but the hardware uses its fixed
50 percent duty cycle; this adapter does not provide amplitude control.

`meshbus_arduboy_audio_status()` exposes the latest request error: -ENODEV for
missing hardware, -EACCES for disabled/muted audio, -EINVAL for malformed input,
-E2BIG for capacity, -ENOTSUP for unsupported mode, and zero for accepted playback.
`SketchConfig.silent_audio` explicitly suppresses playback without changing the
saved preference. Unsupported requests do not silently play a prefix.

The runtime loads the game audio preference from EEPROM byte 2 before setup.
`audio.off()` immediately stops owned playback. `saveOnOff()` updates byte 2 and
uses the regular transactional flush/exit path. Byte 0 is not overwritten and
game progress starts at 16. Erased byte 2 means enabled. Global device buzzer
settings still apply; a game cannot override the user's disabled device buzzer.
