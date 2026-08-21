# SUN
`sun` computes the sunrise and sunset times for points on Earth's surface, or for cities from a database stored as a Human-readable text file.
The user may specify a city in a number of ways: cities may be given as arguments on the command line, given on standard input, or a default city read from the `SUN_HOME_CITY` environment variable.

Output is given in the currently-set user timezone (i.e. `TZ` or `/etc/localtime`), unless the `-u` flag is given, in which case it is output in UTC.

In addition to sunrise and sunset, the times of civil, nautical, and astronomical twilight can also be computed, which can be useful for planning or religious purposes.

Accuracy is limited only by the un-forseeable atmospheric conditions, down to an error of about plus or minus 2 minutes.
Due to different exact values used in the computation, this program may output values slightly different from those produced by other sources (i.e. national weather services); however, these values should in general agree within a couple minutes.

## EXAMPLE USAGE
```sh
$ sun chicago rome tokyo
Chicago: 07:06:15 AM	08:41:26 PM
Rome: 12:25:11 AM	02:01:16 PM
Tokyo: 04:04:59 PM	05:23:31 AM
$
$ sun -
new york
New York: 06:12:57 AM	07:44:38 PM
berlin
Berlin: 12:00:12 AM	02:18:53 PM
canberra
Canberra: 04:37:04 PM	03:36:26 AM
$
$ echo $SUN_HOME_CITY
washington
$
$ sun
Washington: 06:28:01 AM	07:54:18 PM
```

## INSTALLATION
Installation may be performed by way of the `makefile` included in the distribution.
With appropriate permissions, install may be as simple as

```sh
$ make
$ make install
```

which will install the program, the cities database, and documentation to the `/usr/local/` tree.
In order to change installation location, the PREFIX and DATADIR variables may be set on the command line before running `make`.
Note that the DATADIR variable must be set to the same value when running both `make` and `make install`.

## DOCUMENTATION
Documentation is provided in the form of this README, and in a `man` macro-formatted `roff` man page.
Following proper installation, documentation may be read with `man sun`. 

## LICENSE
sun Copyright (C) 2026 Jack Renton Uteg.
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.

Written by Jack R. Uteg.

Cities database Copyright (C) 2026 simplemaps.com, at
<https://simplemaps.com/data/world-cities>.
License CC-BY-4.0: Creative Commons Attribution 4.0 Internation
<https://creativecommons.org/licenses/by/4.0/>
Modified from the original in a manner not endorsed by simplemaps.com.
