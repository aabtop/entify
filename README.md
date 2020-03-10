# Entify

## Building

Entify uses the [respire](https://github.com/aabtop/respire) as the build
system, thus respire must be installed first.  To build for the host system,
simply type:

```
python src/build.py
```

## Setup

Note that all instructions for installing Entify on a device are maintained as
Ansible scripts in `src/platform_provisioning/ansible`.
In many cases, after you've installed an operating system on a device, you can
just use Ansible to build and install Entify on a device.

For example, for the NVIDIA Nano, run the command:

```
ansible-playbook -K src/platform_provisioning/ansible/provision_nano.yaml -e "entify_password=foobar"
```

This will setup the system for use by Entify, build and deploy Entify on it,
and make it so that Entify is launched on device startup.

### Hardware

For hardware, I'm using a USB hub to power the display as well as the Raspberry Pi:

https://www.amazon.com/gp/product/B00VH8ZW02/ref=oh_aui_detailpage_o00_s00?ie=UTF8&psc=1

And for the display I made sure to find one that can be powered by USB:

https://www.amazon.com/gp/product/B07KGQ5W1H/ref=oh_aui_detailpage_o00_s00?ie=UTF8&psc=1

And of course, a small HDMI cable will be needed to connect the Pi to the
display:

https://www.amazon.com/gp/product/B014ROO71W/ref=oh_aui_detailpage_o02_s00?ie=UTF8&psc=1

Using a Raspberry Pi Zero W:

https://www.amazon.com/CanaKit-Raspberry-Wireless-Complete-Starter/dp/B07CMVDHWB/ref=sr_1_1_sspa?ie=UTF8&qid=1544433889&sr=8-1-spons&keywords=raspberry+pi+zero+w&psc=1

### Software

#### NVIDIA Jetson Nano Setup

Download and install Jetson SD card image.

Download page: https://developer.nvidia.com/embedded/jetpack

##### Download and install the NVIDIA SDK Manager

This can be downloaded from the same page as the SD card above.

You can probably use the command line install options to install the host tools:
https://docs.nvidia.com/sdk-manager/sdkm-command-line-install/index.html

But if you want the graphical user interface, see [this forum post](https://devtalk.nvidia.com/default/topic/1048864/install-sdk-manager-via-wsl-windows-subsystem-for-linux-for-install-jetpack-4-2/?offset=6)
for more information on how to get things up and running on the Linux
Subsystem for Windows.

Follow the prompts and install the SDK.

##### Installing the cross-compiler toolchain

In case it did not get installed from the above, the cross-compiler toolchain
can be installed with

```
sudo apt install g++-aarch64-linux-gnu
```

##### Rsync the Nano's system files

Set the environment variable `NANO_ADDR` to the IP address of the Nano device.

Create a directory on the host machine and set the environment variable
`JETSON_SYSROOT` to it.

On the nano, install `rsync` with `sudo apt install rsync`.

Then on the host, call (this might take about 30 minutes):

```
rsync -avzhP --safe-links --delete-after nano@$NANO_ADDR:/{opt,lib,usr} --exclude="lib/firmware" --exclude="lib/modules" --include="usr/lib" --include="usr/include" --include="usr/local/include" --include="usr/local/lib" --exclude="usr/*" --exclude="opt/*" --exclude="usr/lib/libreoffice" --exclude="usr/lib/python*" --exclude="usr/lib/weston" --exclude="usr/lib/thunderbird" --exclude="usr/lib/ssl" $JETSON_SYSROOT
```

Build the application with platform `jetson`.

On the device, disable the lock screen by running:

```
gsettings set org.gnome.desktop.session idle-delay 0
gsettings set org.gnome.desktop.screensaver lock-enabled false
```


rsync the built binary to the device, and then launch it.  Make sure you set the
`DISPLAY` environment variable on the device correctly.

##### Environment setup

Make sure to set the environment variable BUS_NUMBER_FOR_BH1745NUC to a bus
number (e.g. "1") in order to have it read light sensor data from a BH1745NUC
chip.  You may also need to `chmod 777 /dev/i2c-1` in order to access it from
a non-root user.

#### Raspberry Pi setup

Install Raspbian (or Raspbian Lite) on the Raspberry Pi.

##### Enable SSH

Run `sudo raspi-config`, open `Interfacing Options`, select `SSH` and set it
to enabled.

#### Enable auto-login

Run `sudo raspi-config`, open `Boot Options`, then `Desktop / CLI` and then
select `Console Autologin`.

#### Provide a bigger budget of GPU memory

Run `sudo raspi-config`, open `Advanced Options`, then `Memory Split` and then
give the GPU 256MB of memory.

#### Wifi Connectivity Troubleshooting

PS: I had trouble with SSH on the wifi connection until I set my router's
channel to 1.

#### Make sure it's up-to-date

Run:

```
sudo apt-get update
sudo apt-get upgrade
sudo rpi-update
```

#### Host build machine setup

You will need a directory to store the Raspberry Pi toolchain and the Raspberry
Pi sysroot libraries.  You will also need to know the IP address of the
Raspberry Pi to connect to.  Consider setting up environment variables for these
parameters now, and possibly add them to .bashrc, e.g.

```
export RASPI_SYSROOT=$HOME/src/raspi/sysroot
export RASPI_TOOLS=$HOME/src/raspi/tools
export RASPI_ADDR=192.168.0.109
```

##### Clone the Raspberry Pi tools

Clone the Raspberry Pi tools downloaded from their git repository:

https://github.com/raspberrypi/tools

e.g. with the command

```
git clone https://github.com/raspberrypi/tools $RASPI_TOOLS
```

##### Download the system libraries from the Raspberry Pi device

Create a directory to store the sysroot folder, say $RASPI_SYSROOT.

```
mkdir $RASPI_SYSROOT
```

Assuming your Raspberry Pi's address is set as $RASPI_ADDR,

```
rsync -avzhP --safe-links \
      --delete-after pi@$RASPI_ADDR:/{opt,lib,usr} \
      --exclude="lib/firmware" --exclude="lib/modules" \
      --include="usr/lib" --include="usr/include" \
      --include="usr/local/include" --include="usr/local/lib" \
      --exclude="usr/*" --include="opt/vc" --exclude="opt/*" \
      $RASPI_SYSROOT
```

#### Build entify

With the device and host setup with the right toolchains and sysroot, you can
now build entify for the Raspberry Pi.

From the entify src/ directory, enter the command:

```
python build.py -p raspi -c release
```

This will result in a entify package directory, `out/release/package`, that can
be copied to the device and executed.

You can copy the entify output to the Raspberry Pi with the command:

```
rsync -avzh out/release/package/* pi@$RASPI_ADDR:~/entify
```