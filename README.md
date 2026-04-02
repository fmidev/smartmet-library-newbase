# smartmet-library-newbase

Part of [SmartMet Server](https://github.com/fmidev/smartmet-library-spine). See the [SmartMet Server documentation](https://github.com/fmidev/smartmet-library-spine) for an overview of the ecosystem.

## Overview

The newbase library defines the native FMI weather data format, **QueryData** (`.sqd` files). It provides the core data structures and algorithms for working with gridded meteorological data in SmartMet Server and FMI tools.

## Features

- **QueryData format** — reading, writing, and manipulating `.sqd` files
- **Projections** — support for a wide range of map projections (azimuthal, Lambert, stereographic, latlon, etc.) via `NFmiAreaFactory`
- **Parameter definitions** — standardized FMI meteorological parameter IDs and metadata
- **Grid operations** — area masking, interpolation, and data extraction
- **Level handling** — pressure levels, height levels, hybrid levels

## Usage

Newbase is a foundational dependency of SmartMet Server and FMI tools. It is used by:
- [smartmet-engine-querydata](https://github.com/fmidev/smartmet-engine-querydata)
- [smartmet-qdtools](https://github.com/fmidev/smartmet-qdtools)
- [smartmet-qdcontour](https://github.com/fmidev/smartmet-qdcontour)

## License

MIT — see [LICENSE](LICENSE)

## Contributing

Bug reports and pull requests are welcome on [GitHub](../../issues).
