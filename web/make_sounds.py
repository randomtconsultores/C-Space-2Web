"""Build-time placeholder WAVs for existing C events; not a SoX emulator."""
import argparse
import json
import math
from pathlib import Path
import random
import struct
import wave

RATE = 22050
# ID order matches web/audio.h. These are replaceable approximations, not new events.
SOUNDS = [
    ("PICKUP", "sine", .20, 600, 1800, .35),
    ("LASER_DAMAGE", "noise", .35, 120, 120, .40),
    ("BOSS_HIT", "sine", .03, 1500, 1500, .20),
    ("CORE_HIT", "sine", .03, 1800, 1800, .25),
    ("BOSS_EXPLOSION", "noise", .80, 300, 40, .45),
    ("ROCK_BREAK", "sine", .08, 180, 60, .35),
    ("ENEMY_EXPLOSION", "sine", .15, 350, 50, .40),
    ("BOSS_LOCK", "sine", .15, 300, 900, .35),
    ("BOSS_PHASE", "sine", .50, 100, 600, .40),
    ("LASER_AIM", "sine", .04, 2000, 2000, .25),
    ("MEGA_LASER", "square", .60, 150, 150, .35),
    ("BOSS_CHARGE", "sine", .20, 400, 1200, .35),
    ("PLAYER_DAMAGE", "sine", .12, 220, 110, .40),
    ("REWARD_SPAWN", "square", .05, 800, 800, .15),
    ("BOSS_ENTRY", "square", .71, 440, 280, .30),
    ("BODY_DAMAGE", "sine", .16, 200, 150, .40),
]


def pcm(event, sound):
    _, kind, seconds, start, end, volume = sound
    count = int(RATE * seconds)
    rng = random.Random(event)
    noise = 0.0
    data = bytearray()
    for i in range(count):
        t = i / RATE
        phase = 2 * math.pi * (start * t + (end - start) * t * t / (2 * seconds))
        value = math.sin(phase)
        if kind == "square":
            value = 1.0 if value >= 0 else -1.0
        elif kind == "noise":
            noise = .85 * noise + .15 * rng.uniform(-1, 1)
            value = .4 * value + .6 * noise
        envelope = min(1.0, i / (RATE * .003), (count - 1 - i) / (RATE * .01))
        sample = round(32767 * volume * envelope * value)
        data.extend(struct.pack("<h", max(-32767, min(32767, sample))))
    return bytes(data)


def build(output):
    output = Path(output)
    output.mkdir(parents=True, exist_ok=True)
    manifest = {}
    for event, sound in enumerate(SOUNDS, 1):
        name = str(event) + ".wav"
        with wave.open(str(output / name), "wb") as wav:
            wav.setnchannels(1)
            wav.setsampwidth(2)
            wav.setframerate(RATE)
            wav.writeframes(pcm(event, sound))
        manifest[str(event)] = {"name": sound[0], "src": name}
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path("web/build/sounds"))
    build(parser.parse_args().output)
