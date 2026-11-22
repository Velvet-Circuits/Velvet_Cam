# Velvet Cam

Velvet Cam is a Linux encoder for on-camera use. It takes one SDI input from a Blackmagic DeckLink card and publishes it on the network as an NDI source, or as an SRT stream.

One channel. The web dashboard configures the input, the output, the local preview, the intercom, and tally.

## How it works

The camera feeds SDI into the DeckLink card. Velvet Cam captures that signal and publishes it in one of two ways:

- **NDI.** A source on the local network. The default name is `Velvet-Cam` followed by the input number. NDI receivers, including Studio Monitor, see it like any other source. While the dashboard is running, the source also advertises the dashboard address: in Studio Monitor the gear icon opens that page.
- **SRT.** The same input leaves as MPEG-TS, with H.264 or H.265 video and AAC-LC audio. Velvet Cam can listen on a port, or call a remote address. AES encryption uses a passphrase of 10 to 79 characters. Video is encoded on an Intel GPU through VA-API, and only while a receiver is connected.

On the display attached to the computer, the SDI input is shown full screen. An NDI return can be overlaid full screen, or as a picture-in-picture at half or quarter size in one of the four corners.

The intercom plays the audio of an NDI source on the computer output. The microphone is sent back onto the network as an NDI talkback. Talk can be held from the dashboard or from a hardware button.

An Arduino or Seeeduino XIAO on USB lights the program and preview tally LEDs and reads the PTT button. An on-air contact closes while program tally is on.

The first start without a license keeps the SDI-to-NDI and SDI-to-SRT outputs open for five days. A license is one signed line for that computer and for a period. Paste it under **Settings → License**. The expiry date is part of the license. After that date the outputs stay open for seven more days, then they close. The dashboard stays available. An expired license does not start the five days again.

Interlaced SDI formats, such as 1080i50, are sent as they arrive. Deinterlacing is not available yet. Use a progressive format for SRT.

## Requirements

| Requirement | Notes |
|-------------|--------|
| System | Ubuntu 22.04 or 24.04, 64-bit |
| Card | Blackmagic DeckLink |
| Driver | Blackmagic Desktop Video, installed separately |
| Network | `apt` access during installation, then the network used for NDI or SRT |
| SRT | Intel GPU with VA-API, and the user in the `render` group |
| NDI\|HX return | FFmpeg 7 libraries. High Bandwidth NDI return does not need them |

The package already includes the NDI and SDL2 runtimes. The target machine does not need a compiler, the DeckLink SDK, or the NDI SDK.

## Install

Download `velvet-cam-1.2.0.tar.gz` from the [releases](https://github.com/Velvet-Circuits/Velvet_Cam/releases/latest).

```bash
tar xf velvet-cam-1.2.0.tar.gz
cd velvet-cam-1.2.0
sudo ./install.sh
```

Running `install.sh` accepts the terms in `LICENSE`. The script:

1. installs the required system libraries (audio, network, tray icon);
2. copies the program to `/opt/velvet-cam`;
3. adds **Velvet Cam** to the applications menu;
4. installs the `velvet-cam` service, without starting it on its own;
5. warns if Blackmagic Desktop Video is not installed.

When the installer asks for a GitHub token, press Enter and leave it empty. Releases in this repository are public, and the dashboard downloads updates without a token.

Start the encoder, and enable it at boot if it should come up with the machine:

```bash
sudo systemctl enable --now velvet-cam
```

Or open **Velvet Cam** from the applications menu. Autostart can also be turned on from the dashboard, under **Settings → Application**.

The dashboard is at [http://localhost:18080/](http://localhost:18080/). From another computer on the network, use the same port and this machine's IP address. Change the port under **Settings → UI**. A different port restarts Velvet Cam and opens the page at the new address.

Service logs:

```bash
journalctl -u velvet-cam -f
```

Remove the program with:

```bash
sudo velvet-cam-uninstall
```

Saved preferences and sessions stay on the computer.

## Use

1. Connect the camera to the DeckLink card and confirm in Blackmagic Desktop Video that the signal is locked.
2. Open the dashboard.
3. In **Configure**, choose the SDI input, the source name, and NDI or SRT output, then start the encode.
4. In **Preview**, turn on the local monitor and, if needed, choose the NDI return, its size, and its corner.
5. In **Intercom**, choose the source to listen to and the talkback microphone.
6. In **Hardware**, enable the Arduino or XIAO board when one is connected.
7. Save the session. On the next start Velvet Cam comes back with the same configuration.

**Dashboard** summarizes the computer, the encode, and any alerts. **Channels** is the live channel, with start, pause, and edit.

### Tally and PTT

Default wiring, changeable under **Hardware**:

| Signal | Pin | Notes |
|--------|-----|--------|
| PTT button | D3 to GND | Internal pull-up; pressed means talk |
| Program LED | D2 | Lit on program tally |
| Preview LED | D8 | Lit on preview tally |
| On-air contact | D4 to GND | Pulled to ground while program is on |

On a Seeeduino XIAO the levels are 3.3 V, and the external pins to use are D0–D10. Holding the hardware PTT turns talkback on. Releasing it leaves a dashboard latch unchanged.

In SRT output, NDI tally does not apply, so the LEDs and the on-air contact stay off. PTT and the intercom keep working.

## Updates

Under **Settings → Updates**, Velvet Cam checks the releases in this repository. The archive is named `velvet-cam-<version>.tar.gz`. An update replaces the program in `/opt/velvet-cam` and restarts the service.
