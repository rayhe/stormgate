# Stormgate

A text-based MUD (Multi-User Dungeon) descended from the Diku/Merc/Envy lineage. Written in C, running on Oracle Cloud free tier.

**Play now:** [rayhe.net/stormgate](https://new.rayhe.net/stormgate.html) | `telnet 129.146.126.146 4000`

<img src="http://rayhe.net/cat" width="600px"/>

## Architecture

```
Browser (rayhe.net/stormgate.html)
  → WebSocket → Node.js proxy (ws-proxy.js)
  → TCP → Stormgate MUD server (C, port 4000)

Oracle Cloud VM (ARM A1 Flex, 4 OCPU / 24GB RAM, always free)
  ├── stormgate     — C game server (systemd)
  ├── ws-proxy.js   — WebSocket-to-TCP bridge (systemd)
  └── Caddy         — HTTPS + reverse proxy (Let's Encrypt)
```

## Web Client Features

- Terminal emulation via xterm.js with full ANSI color support
- Tab autocomplete for MUD commands
- Command history (persisted across sessions)
- Quick macro buttons for movement and common commands
- Numpad navigation (8/2/6/4/9/3 = N/S/E/W/U/D)
- Scrollback (PageUp/PageDown, 10k lines)
- Mobile-responsive with touch-friendly buttons

## Building from Source

```bash
cd src
make -j$(nproc) -f makefile.prod
```

Requires `gcc`, `g++`, `make`, `libcrypt-dev`. The production makefile uses `-fcommon` for GCC 10+ compatibility.

## Deploying

```bash
git clone https://github.com/rayhe/stormgate.git /opt/stormgate
sudo /opt/stormgate/deploy/setup.sh
sudo systemctl start stormgate
```

See `deploy/` for systemd service files and Caddyfile.

## Game Details

- 21 classes (Mage, Warrior, Thief, Paladin, Necromancer, Vampire, ...)
- 27 races, 27 languages, 485 skills/spells
- 116 areas with OLC (Online Creation) for builders
- Clans, religions, quests, economy, crafting, PvP arena
- Max level 116 (100 = hero, 105+ = immortal staff)

## Lineage

Diku MUD (1990) → Merc 2.1 (1993) → Envy (1994) → Mythran 3.1.1 → Stormgate
