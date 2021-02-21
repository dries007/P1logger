# P1logger

Belgium & the Netherlands (and presumably other countries) have "smart" or digital utility meters.
They are equipped with a "user port", also known as the "P1" port. This port allows users to read out info about their meter. The information is logged to an influxDB database.

Be aware you might have to activate this port before you can use it.

I'm Flemish, so this program primarily targets meters from Fluvius, but it should work for all meters following roughly the same standard.
If you live in another region and would like to test it out and/or adapt the required changes, please let me know.

I've saved a number of related documents (technical specs) in the [docs](./docs) folder.

## Dashboards

I use Grafana to monitor the data. I made an export of my dashboard, you can import this to save yourself some time setting everything up. The dashboard is setup with a datasource called "p1log".

The file is called [grafana.json](./grafana.json).

## License

The code, dashboard, dockerfile and other original works are released under the MIT license.

## Usage

Recommended: Run the provided docker image.

```
docker run -d --name p1logger --device=/dev/ttyUSB0 -v /etc/localtime:/etc/localtime:ro --network host --restart unless-stopped ghcr.io/dries007/p1logger:latest
```

Nodes:
+ `--device /dev/ttyUSB0` grants the container access to the USB-serial device. You cannot use a volume for this.
+ The `/etc/localtime` read only volume is required make sure the timezone inside the container is the same as your host machine, which is presumed to be the timezone of your meter. If not, use the TZ env var to set it.
+ You must provide a valid InfluxDB dsn. The one from the example command assumes you're running it on the host system.
  If you'd like to use a fully dockerized setup, create a docker compose file.

Alternatively use standard python methods to run the `P1logger` folder as a module.

Accepted arguments are found in [P1logger's main file](./P1logger/__main__.py).

**Note** This module generates 1 row per second. I recommend you create a retention policy and downsample setup if you are using something with limited storage capacity, like a Raspberry Pi with SD card.

## Known Hardware

### Sagemcom S211

Used by Fluvius (Belgium). 

Supports Belgian version of the DSMR 5.0.2 standard, known as eMUCs v1.4.
 
Buying a dedicated converter cable is recommended, costs 10~20€ online.
DIY is possible, the signal is a basic TX-only 115200 baud 8N1 UART/RS232 with inverted signal levels (open collector).
See the DSMR5 and eMUCs documents for more in depth info.

Due to GDPR related things, you must activate the P1 port via the Fluvius website before it can be used.

This meter also has S1 port (real time sample data), but that's not supported by this program.
