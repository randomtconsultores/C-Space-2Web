# Replaceable audio assets — first small delivery

The existing C event bridge remains in place. This delivery adds only a Python
standard-library build tool and tests for temporary WAV assets. It does not yet
load Howler.js, enable SOUND, change gameplay or make the current page audible.
The following delivery must connect the JS player/menu and include asset generation
in the Web build; do not describe audio migration as complete before that works.

From the repository root:

```sh
python3 web/test_sound_assets.py
python3 web/make_sounds.py --output web/build/sounds
```

This generates 16 mono, 16-bit PCM WAVs plus manifest.json. IDs match web/audio.h;
manifest src values are relative to the manifest directory. Effects are simple,
deterministic approximations inspired by existing frequencies/durations, NOT a
faithful SoX export. No shell tools, new audio events, music or dependencies are
needed to generate them. There is no synthesis in the browser.

The future JS/Howler player should map C IDs to these files, not interpret SoX.
Own recordings can later replace the generated files or the event-to-file mapping.
These outputs are disposable: regeneration overwrites matching output filenames.
Do not keep custom originals in web/build/sounds; keep them in a source asset
folder and arrange copying/mapping when introducing them.

Local generator tests passed: C ID coverage, 16 valid WAVs, sample format/duration,
nonzero bounded samples, zero endpoints and reproducibility. No new Wasm build or
browser listening test has been performed for this delivery. Existing native code,
MAX_NIVEIS=4 and deferred persistence remain unchanged. The overall plan/report
must be updated with integration and actual Codespace/browser results in the next
connected increment. No existing files are modified by this commit.
